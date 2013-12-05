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
#ifndef PROPERTIESDIALOG_H
#define PROPERTIESDIALOG_H

#include <q3tabdialog.h>

class QTScope;
class Q3ListBox;
/**
A dialog to allow the user to change the default behaviour of the program.

@author Matthias H. Hennig
*/
class propertiesDialog : public Q3TabDialog
  {
    Q_OBJECT

  private:
    QTScope *caller;
    Q3ListBox *ppaths;

  public:
    propertiesDialog(QTScope *c, const char *name = 0);

    ~propertiesDialog();

private slots:
    void addClicked();
    void removeClicked();
  };

#endif