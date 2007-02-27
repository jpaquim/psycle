/***************************************************************************
*   Copyright (C) 2006 by  Neil Mather   *
*   nmather@sourceforge   *
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

#include <QtGui>

 #include "newmachinedlg.h"

 NewMachineDlg::NewMachineDlg(QWidget *parent) 
    : QDialog(parent)
 {
     setWindowTitle(tr("Choose New Machine"));
     
     QGridLayout *layout = new QGridLayout();

     QTabWidget *machineTabs = new QTabWidget();

     QListWidget *genList = new QListWidget();
     QListWidget *efxList = new QListWidget();
     QListWidget *intList = new QListWidget();
     QListWidget *ladList = new QListWidget();
     buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
                                      | QDialogButtonBox::Cancel);
     connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
     connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

     machineTabs->addTab(genList, "Generators");
     machineTabs->addTab(efxList, "Effects");
     machineTabs->addTab(intList, "Internal");
     machineTabs->addTab(ladList, "Ladspa");

     layout->addWidget(machineTabs);
     layout->addWidget(buttonBox);
     setLayout(layout);
 }

