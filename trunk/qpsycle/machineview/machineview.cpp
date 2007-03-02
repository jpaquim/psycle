/***************************************************************************
*   Copyright (C) 2007 by  Neil Mather   *
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
#include <QGraphicsLineItem>

 #include "machineview.h"
 #include "machinegui.h"
 #include "wiregui.h"

 MachineView::MachineView()
 {
     scene_ = new QGraphicsScene(this);
     scene_->setBackgroundBrush(Qt::black);

     setDragMode(QGraphicsView::RubberBandDrag);
     setSceneRect(0,0,width(),height());
     setScene(scene_);
     setBackgroundBrush(Qt::black);

     MachineGui *machGui0 = new MachineGui(100, 20, this);
     MachineGui *machGui1 = new MachineGui(400, 20, this);
     MachineGui *machGui2 = new MachineGui(100, 120, this);

     machGui0->setName("Foo");
     machGui1->setName("Bar");
     machGui2->setName("Baz");

     scene_->addItem(machGui0);
     scene_->addItem(machGui1);
     scene_->addItem(machGui2);
     WireGui *wireGui0 = new WireGui(machGui0, machGui1);
     WireGui *wireGui1 = new WireGui(machGui0, machGui2);
     scene_->addItem(wireGui0);
     scene_->addItem(wireGui1);

     newMachineDlg = new NewMachineDlg();

     // A temporary line to display when user is making a new connection.
     tempLine_ = new QGraphicsLineItem(0, 0, 0, 0);
     tempLine_->setPen(QPen(Qt::gray,2,Qt::DashLine));
     tempLine_->setVisible(false);// We don't want it to be visible yet.
     scene_->addItem(tempLine_);
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
//        add a new machine.
     } else {
//        don't bother.
     }
 }*/

 void MachineView::scaleView(qreal scaleFactor) 
 {
      qreal factor = matrix().scale(scaleFactor, scaleFactor).mapRect(QRectF(0, 0, 1, 1)).width();
     if (factor < 0.07 || factor > 100)
         return;

     scale(scaleFactor, scaleFactor);
 }

 void MachineView::startNewConnection(MachineGui *srcMacGui, QGraphicsSceneMouseEvent *event)
 {
     qDebug("machineview: new con");
     tempLine_->setLine( QLineF( srcMacGui->centrePointInSceneCoords(), event->scenePos() ) );
     tempLine_->setVisible(true);
 }

 void MachineView::closeNewConnection(MachineGui *srcMacGui, QGraphicsSceneMouseEvent *event)
 {
     qDebug("machineview: close con");
     // See if we hit another machine gui.
     if ( scene_->itemAt( tempLine_->mapToScene( tempLine_->line().p2() ) )  ) {
         QGraphicsItem *itm = scene_->itemAt( tempLine_->mapToScene( tempLine_->line().p2() ) );
         if (itm->type() == 65537) { // FIXME: un-hardcode this
            MachineGui *dstMacGui = qgraphicsitem_cast<MachineGui *>(itm);
            connectMachines(srcMacGui, dstMacGui); 
         }
     }
     tempLine_->setVisible(false);     // We want the tempLine to disappear, whatever happens.
 }

 void MachineView::connectMachines( MachineGui *srcMacGui, MachineGui *dstMacGui )
 {
    // Check there's not already a connection.
    
    // Make a connection in the song file..
    
    // Make a new wiregui connection.
    WireGui *newWireGui = new WireGui( srcMacGui, dstMacGui );
    connect( newWireGui, SIGNAL( deleteConnectionRequest( WireGui* ) ),
             this, SLOT( deleteConnection( WireGui* ) ) );
    scene_->addItem( newWireGui );
 }

 void MachineView::deleteConnection( WireGui *wireGui )
 {
    qDebug("deleting connection");

    // Delete the connection in the song file.
    
    // Delete the connection in the GUI.
    scene_->removeItem( wireGui ); // FIXME: do we need to do more here?
 }
