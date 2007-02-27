/***************************************************************************
*   Copyright (C) 2006 by  Stefan   *
*   natti@linux   *
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
#include <QGraphicsScene>
#include <QPainter>
#include <iostream>

 #include "machineview.h"
 #include "machinegui.h"

// should be a canvas
 MachineView::MachineView()
 {
     QGraphicsScene *scene = new QGraphicsScene(this);
     scene->setItemIndexMethod(QGraphicsScene::NoIndex);
     scene->setBackgroundBrush(Qt::black);
     setBackgroundBrush(Qt::black);
     setScene(scene);
     setCacheMode(CacheBackground);
     setRenderHint(QPainter::Antialiasing);
     setTransformationAnchor(AnchorUnderMouse);
     setResizeAnchor(AnchorViewCenter);

     MachineGui *machGui = new MachineGui(0, 0, this);
     scene->addItem(machGui);
 }

/* void MachineView::mouseDoubleClickEvent(QMouseEvent *event)
 {
     int accepted = newMachineDlg->exec();
     if (accepted) {
        QMessageBox::information (this, "Hi", "Add a new machine...");
     } else {
        QMessageBox::information (this, "Hi", "Cancel");
     }
 }*/
