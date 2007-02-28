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
#include <QGraphicsScene>
#include <QPainter>
#include <iostream>

 #include "machineview.h"
 #include "machinegui.h"
 #include "wiregui.h"

 MachineView::MachineView()
 {
     QGraphicsScene *scene = new QGraphicsScene(this);
     scene->setBackgroundBrush(Qt::black);
     scene->setSceneRect(0,0,width(),height());

     setScene(scene);
     setBackgroundBrush(Qt::black);

     MachineGui *machGui = new MachineGui(100, 20, this);
     QGraphicsTextItem *name = new QGraphicsTextItem("Foo", machGui);
     name->setDefaultTextColor(Qt::white);
     MachineGui *machGui2 = new MachineGui(400, 20, this);
     QGraphicsTextItem *name2 = new QGraphicsTextItem("Bar", machGui2);
     name2->setDefaultTextColor(Qt::white);
     MachineGui *machGui3 = new MachineGui(100, 120, this);
     QGraphicsTextItem *name3 = new QGraphicsTextItem("Baz", machGui3);
     name3->setDefaultTextColor(Qt::white);
     scene->addItem(machGui);
     scene->addItem(machGui2);
     scene->addItem(machGui3);
     WireGui *wireGui = new WireGui(machGui, machGui2);
     WireGui *wireGui1 = new WireGui(machGui, machGui3);
     scene->addItem(wireGui);
     scene->addItem(wireGui1);
 }

 void MachineView::keyPressEvent(QKeyEvent *event)
 {
     switch (event->key()) {
     case Qt::Key_Plus:
         scaleView(1.2);
         break;
     case Qt::Key_Minus:
         scaleView(1 / 1.2);
         break;
     default:
         QGraphicsView::keyPressEvent(event);
     }
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
 void MachineView::scaleView(qreal scaleFactor) 
 {
      qreal factor = matrix().scale(scaleFactor, scaleFactor).mapRect(QRectF(0, 0, 1, 1)).width();
     if (factor < 0.07 || factor > 100)
         return;

     scale(scaleFactor, scaleFactor);
 }
