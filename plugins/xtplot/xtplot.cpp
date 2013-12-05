/***************************************************************************
 *   Copyright (C) 2004 by Matthias H. Hennig                              *
 *   hennig@cn.stir.ac.uk                                                  *
 *   Copyright (C) 2005 by Bernd Porr                                      *
 *   Bernd Porr, BerndPorr@f2s.com                                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "xtplot.h"

#include <qcheckbox.h>
#include <qlabel.h>
#include <qstring.h>
#include <q3groupbox.h>
#include <q3buttongroup.h>
#include <qpushbutton.h>
#include <q3filedialog.h>
#include <qmessagebox.h>


#include <qwt_math.h>
#include <qwt_counter.h>

#include <stdlib.h>

#include <iostream>

pluginInfo myPluginInfo = {"x-t Plot", "Plot", 1};

scopePlotPlugin *createPlugin (QTScope* caller, QWidget* parent, const char* name, int id, int wflags,int numberOfSamples) {
	return new xtPlot(caller, parent, name, id, wflags,numberOfSamples);
}

using std::cout;
using std::cerr;

xtPlot::xtPlot(QTScope* caller, QWidget* parent, const char* name, int id, int wflags,int numberOfSamples)
	: scopePlotPlugin(caller, parent, name, id, wflags,numberOfSamples)
{
  cout << "xtPlot::xtPlot: xtPlot Plugin generated\n";
  callingWidget = caller;
  idThis = id;

  setCaption( QString().sprintf("Channel %s",name) );

  cerr << "Number of samples:" << numberOfSamples << "\n";

  plotLength = numberOfSamples;
  nSamples = numberOfSamples;

  x=new double[numberOfSamples];
  y=new double[numberOfSamples];

  // initialize the data arrays
  clearData();

  // allow Docking
  setDockEnabled ( Qt::DockTop, TRUE );
  setDockEnabled ( Qt::DockLeft, TRUE );

  // conmstruct a toolbar
  Q3ToolBar * plotTools = new Q3ToolBar( this, "plot operations" );
  plotTools->setLabel( tr("Plot Operations") );

  autoscaleCheck = new QCheckBox("Autoscale", plotTools);
  autoscaleCheck->setChecked(TRUE);
  connect(autoscaleCheck, SIGNAL(clicked()), this, SLOT(slotAutoscaleToggled()));

  plotTools->addSeparator();

  //plotTools->addSeparator();
  new QLabel(tr("Ymax:"), plotTools);
  ymaxCounter = new QwtCounter( plotTools);
  ymaxCounter->setRange(-1, 20.0, 0.01);
  ymaxCounter->setNumButtons(3);
  ymaxCounter->setIncSteps(QwtCounter::Button1, 1);
  ymaxCounter->setIncSteps(QwtCounter::Button2, 10);
  ymaxCounter->setIncSteps(QwtCounter::Button3, 100);
  ymaxCounter->setValue(1);
  connect(ymaxCounter, SIGNAL(valueChanged(double)), this, SLOT(slotYmaxChanged(double)));
  ymaxCounter->setDisabled(TRUE);

  plotTools->addSeparator();
  new QLabel(tr("Ymin:"), plotTools);
  yminCounter = new QwtCounter( plotTools);
  yminCounter->setRange(-20.0, 1, 0.01);
  yminCounter->setNumButtons(3);
  yminCounter->setIncSteps(QwtCounter::Button1, 1);
  yminCounter->setIncSteps(QwtCounter::Button2, 10);
  yminCounter->setIncSteps(QwtCounter::Button3, 100);
  yminCounter->setValue(-1);
  connect(yminCounter, SIGNAL(valueChanged(double)), this, SLOT(slotYminChanged(double)));
  yminCounter->setDisabled(TRUE);

  QFont tbFont("Courier",14);
  tbFont.setBold(TRUE);

  Q3ButtonGroup* tbGrp=new Q3ButtonGroup( 3,
					Qt::Horizontal,
					plotTools );

  QLabel* lx = new QLabel(tr("X:"), tbGrp);
  lx->setFont(tbFont);

  // create the button
  tbIncPushButton = new QPushButton( "+", tbGrp, "tbIncPushButton" );
  tbIncPushButton->setFont(tbFont);
  tbIncPushButton->setToggleButton( FALSE );
  tbIncPushButton->setOn( FALSE );
  tbIncPushButton->setSizeIncrement(0,0);
  tbIncPushButton->setSizePolicy (QSizePolicy(QSizePolicy::Fixed,
					      QSizePolicy::Fixed));

  plotTools->connect(tbIncPushButton, SIGNAL( clicked() ),
		     this, SLOT( incTbEvent() ) );
  
  // create the button
  tbDecPushButton = new QPushButton( "-", tbGrp, "tbDecPushButton" );
  tbDecPushButton->setFont(tbFont);
  tbDecPushButton->setToggleButton( FALSE );
  tbDecPushButton->setOn( FALSE );
  tbDecPushButton->setSizeIncrement(0,0);
  tbDecPushButton->setSizePolicy (QSizePolicy(QSizePolicy::Fixed,
					      QSizePolicy::Fixed));
  

  plotTools->connect(tbDecPushButton, SIGNAL( clicked() ),
		     this, SLOT( decTbEvent() ) );


  freezePushButton = new QPushButton( "Freeze", plotTools );
  freezePushButton->setEnabled( TRUE );
  freezePushButton->setToggleButton( TRUE );
  freezePushButton->setOn( FALSE );

  plotTools->addSeparator();

  // filename the button
  filePushButton = new QPushButton( "Save Data", plotTools );
  filePushButton->setToggleButton( FALSE );
  filePushButton->setEnabled( FALSE );
  filePushButton->setOn( FALSE );

  plotTools->connect(filePushButton, SIGNAL( clicked() ),
		     this, SLOT( enterFileName() ) );
  
  moveDockWindow( plotTools, Qt::DockLeft );

  // contruct a QwtPlot Widget
  plotWidget = new QwtPlot(this);

  // QwtPlot specific defaults:
  // colour
  plotWidget->setCanvasBackground(Qt::white);
  // outline
  //plotWidget->enableOutline(FALSE);
  // no legend
  //plotWidget->enableLegend(FALSE);
  // no grid
  //plotWidget->enableGridX(FALSE);
  //plotWidget->enableGridY(FALSE);
  // set some defaults for the axes
  plotWidget->setAxisTitle(QwtPlot::xBottom, "Time/ms");
  plotWidget->setAxisTitle(QwtPlot::yLeft, "Amplitude/V");
  // default xrange
  plotWidget->setAxisScale( QwtPlot::xBottom, 0, 1000);
  // yrange autoscale
  plotWidget->setAxisAutoScale(QwtPlot::yLeft);

  // insert a curve
  //curve = plotWidget->insertCurve("");
  // assign data
  //plotWidget->setCurveRawData(curve, x, y, plotLength);

  setCentralWidget(plotWidget);
}


xtPlot::~xtPlot()
{
	delete x;
	delete y;
}

/*!
    \fn xtPlot::insertValue(double v)
 */
void xtPlot::insertValues(int num,int append) {
    //if (freezePushButton->state()==QButton::On) {
    if (1) {
		filePushButton->setEnabled( TRUE );
	} else {
		filePushButton->setEnabled( FALSE );
		if (append) {
			// scroll
            //qwtShiftArray(y, nSamples, num);
			for(int i=0; i<num; i++) {
				y[i] = ds[num-i-1];
			}
		} else {
			// write from zero loc
			for(int i=0; i< num; i++) {
				y[i] = ds[i];
			}
		}
	}
}


/*!
    \fn xtPlot::clearData()

    reset the data on the plot
 */
void xtPlot::clearData()
{
  for(int i=0; i<nSamples; i++) {
    x[i]=1000.0*(double)i/freq;
    y[i]=0.0;
  }
}


/*!
    \fn xtPlot::replot()
 */
void xtPlot::replot()
{
  plotWidget->replot();
}


void xtPlot::incTbEvent() {
	plotLength=plotLength*2;
	xRangeChanged();
}



void xtPlot::decTbEvent() {
	plotLength=plotLength/2;
	xRangeChanged();
}



/*!
    \fn xtPlot::slotXrangeChanged(double)
 */
void xtPlot::xRangeChanged()
{
	if (plotLength>=nSamples) {
		plotLength=nSamples;
	}
	if (plotLength<2) {
		plotLength=2;
	}
	samplingRateChanged();
}


void xtPlot::samplingRateChanged() {
	plotWidget->setAxisScale( QwtPlot::xBottom, 0, 1000.0*(double)plotLength/freq );
	clearData();
	replot();
}



/*!
    \fn xtPlot::slotYminChanged
 */
void xtPlot::slotYminChanged(double v)
{
  //const QwtScaleDiv* a = plotWidget->axisScale(QwtPlot::yLeft);
  //plotWidget->setAxisScale( QwtPlot::yLeft, v, a->hBound());
  ymaxCounter->setRange(v, 20.0, 0.01);
}


/*!
    \fn xtPlot::slotYmaxChanged(double)
 */
void xtPlot::slotYmaxChanged(double v)
{
  //const QwtScaleDiv* a = plotWidget->axisScale(QwtPlot::yLeft);
  //plotWidget->setAxisScale( QwtPlot::yLeft, a->lBound(), v);
  yminCounter->setRange(-20.0, v, 0.01);
}


/*!
    \fn xtPlot::slotAutoscaleToggled()
 */
void xtPlot::slotAutoscaleToggled()
{
  if(autoscaleCheck->isChecked())   {
    // yrange autoscale on
    yminCounter->setDisabled(TRUE);
    ymaxCounter->setDisabled(TRUE);
    plotWidget->setAxisAutoScale(QwtPlot::yLeft);

  }
  else
  {
    // yrange autoscale off
    yminCounter->setDisabled(FALSE);
    ymaxCounter->setDisabled(FALSE);
    //const QwtScaleDiv* a = plotWidget->axisScale(QwtPlot::yLeft);
    //yminCounter->setValue(a->lBound());
    //ymaxCounter->setValue(a->hBound());
    //plotWidget->setAxisScale( QwtPlot::yLeft, a->lBound(), a->hBound());
  }
}



void  xtPlot::enterFileName() {
	Q3FileDialog* fd=new Q3FileDialog( this );
	int result;
	do {
		result=0;
		fd->setMode( Q3FileDialog::AnyFile );
		fd->setFilter("comma separated values (*.csv)");
		fd->addFilter("space separated values (*.txt)");
		fd->addFilter("space separated values (*.dat)");
		if ( fd->exec() == QDialog::Accepted ) {
			QString filename;
			filename=fd->selectedFile();
			QString extension;
			extension=fd->selectedFilter();
			extension=extension.mid(extension.find("."),4);
			if (filename.find(extension)==-1) {
				filename=filename+extension;
			}
			FILE* f=fopen(filename.latin1(),"wt");
			if (f) {
				char c;
				if (fd->selectedFilter().find("csv")>0) {
					c=',';
				} else {
					c=' ';
				}
				for(int i=0; i<plotLength; i++) {
					float t=(float)i/freq;
					int result=fprintf(f,"%f%c%f\n",t,c,y[i]);
					if (result<0) {
						break;
					}
				}
				if (!(result<0)) {
					result=fclose(f);
				}
			} else {
				result=-1;
			}
		} else {
			result=0;
		}
		if (result<0) {
			fd->setCaption("Error saving file. Please retry.");
		}
        } while (result<0);
	delete fd;
}
