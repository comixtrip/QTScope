/***************************************************************************
 *   Copyright (C) 2004 by Matthias H. Hennig                              *
 *   hennig@cn.stir.ac.uk                                                  *
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

#include "newviewdialog.h"

#include <qpushbutton.h>
#include <q3listbox.h>
#include <qlayout.h>
#include <q3buttongroup.h>
#include <qspinbox.h>
//Added by qt3to4:
#include <Q3HBoxLayout>
#include <Q3VBoxLayout>
#include <Q3ValueList>
#include <Q3Frame>


newViewDialog::newViewDialog(int numChannels, Q3ValueList <pluginData> availablePlugins, QTScope *c, QWidget *parent, const char *name, bool modal, Qt::WFlags f )
    : QDialog(parent, name, modal, f)
{
  caller = c;
  pl = availablePlugins;
  maxSelect = 1;
  maxChannels = numChannels;

  channelSelectors.setAutoDelete( TRUE );

  setCaption( "QTScope -- Open new Plot" );
  resize( 320, 240 );

  mainLayout = new Q3VBoxLayout( this, 2, 2, "main");

  f1 = new Q3Frame( this );
  f1->setFrameShape( Q3Frame::StyledPanel );
  f1->setFrameShadow( Q3Frame::Sunken );
  mainLayout->addWidget(f1);

  chooserBox = new Q3HBoxLayout( f1, 2, 2, "choose" );

  // the list of plugins
  pluginsList = new Q3ListBox( f1 );
  pluginsList->setFocusPolicy( Qt::StrongFocus );
  pluginsList->setFrameStyle( Q3Frame::Panel | Q3Frame::Raised );

  Q3ValueList<pluginData>::iterator it;
  for ( it = pl.begin(); it != pl.end(); ++it )
    pluginsList->insertItem( (*it).name );

  connect( pluginsList, SIGNAL( highlighted ( const QString &) ), this, SLOT( slotPluginSelected(const QString &)));

  chooserBox->addWidget(pluginsList);

  // the channels
  channelsList = new Q3ButtonGroup( "Select Input Channel(s)", f1 );
  channelsList->setColumnLayout(0, Qt::Vertical );
  channelsList->layout()->setSpacing( 6 );
  channelsList->layout()->setMargin( 11 );
  channelsListL = new Q3VBoxLayout( channelsList->layout() );
  channelsListL->setAlignment( Qt::AlignTop );

  chooserBox->addWidget(channelsList);

  // OK+Cancel
  buttonBox = new Q3HBoxLayout( mainLayout );

  okPushButton = new QPushButton( this, "ok" );
  okPushButton->setText( "OK" );
  okPushButton->setDefault( TRUE );
  okPushButton->setMaximumSize(okPushButton->sizeHint());
  //okPushButton->setSizePolicy(QSizePolicy::Fixed);
  buttonBox->addWidget( okPushButton );

  cancelPushButton = new QPushButton( this, "cancel" );
  cancelPushButton->setText( "Cancel" );
  //cancelPushButton->setAccel( Key_Escape );
  cancelPushButton->setMaximumSize(cancelPushButton->sizeHint());
  //cancelPushButton->setSizePolicy(QSizePolicy::Fixed);
  buttonBox->addWidget( cancelPushButton );

  connect( okPushButton, SIGNAL( clicked() ), this, SLOT( accept() ) );
  connect( cancelPushButton, SIGNAL( clicked() ), this, SLOT( reject() ) );

  // start with first plugin
  pluginsList->setCurrentItem(0);


}


newViewDialog::~newViewDialog()
{}

/*!
    \fn newViewDialog::accept()
 */
void newViewDialog::accept()
{
  // prepare a list of requested channels
  int *channels = new int[maxSelect];
  channelSelectors.first();
  for(unsigned int i=0; i<maxSelect; i++) {
    channels[i] = channelSelectors.current()->value();
    channelSelectors.next();
  }
  // start a new plugin
  caller->runPlugin(pluginsList->currentText(), channels);

  QDialog::accept();
}

/*!
    \fn newViewDialog::slotPluginSelected(const QString & name)
 */
void newViewDialog::slotPluginSelected(const QString &name)
{
  Q3ValueList<pluginData>::iterator it;
  for ( it = pl.begin(); it != pl.end(); ++it )
    {
      if ( (*it).name == name )
        {
          maxSelect = (*it).numChannels;
          if(channelSelectors.count() < maxSelect)
            for(unsigned int i = channelSelectors.count(); i < maxSelect; i++) {
              channelSelectors.append( new QSpinBox( 0, maxChannels-1, 1, channelsList ) );
              channelsListL->addWidget(channelSelectors.current());
              channelSelectors.current()->show();
            }
          else if(channelSelectors.count() > maxSelect) {
            channelSelectors.last();
            for(unsigned int i = maxSelect; i< channelSelectors.count(); i++)
              channelSelectors.remove();
          }
        }
    }
}

