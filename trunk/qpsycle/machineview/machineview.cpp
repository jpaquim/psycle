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
 #include "mastergui.h"
 #include "wiregui.h"

 #include "psycore/song.h"
 #include "psycore/sampler.h"
 #include "psycore/constants.h"
 #include "psycore/machine.h"

 MachineView::MachineView(psy::core::Song *song_)
 {
     scene_ = new QGraphicsScene(this);
     scene_->setBackgroundBrush(Qt::black);

     setDragMode(QGraphicsView::RubberBandDrag);
     setSceneRect(0,0,width(),height());
     setScene(scene_);
     setBackgroundBrush(Qt::black);

    int fb = song_->GetFreeBus();
    song_->CreateMachine(psy::core::MACH_SAMPLER, 100, 20, "SAMPLER", fb);  
    psy::core::Machine *sampler0 = song_->_pMachine[fb];

    fb = song_->GetFreeBus();
    song_->CreateMachine(psy::core::MACH_SAMPLER, 300, 20, "SAMPLER", fb);  
    psy::core::Machine *sampler1 = song_->_pMachine[fb];

     psy::core::Machine *master = song_->_pMachine[psy::core::MASTER_INDEX] ; 
     song_->InsertConnection( sampler0->_macIndex , master->_macIndex, 1.0f);
     song_->InsertConnection( sampler1->_macIndex , master->_macIndex, 1.0f);

     newMachineDlg = new NewMachineDlg();

     // A temporary line to display when user is making a new connection.
     tempLine_ = new QGraphicsLineItem(0, 0, 0, 0);
     tempLine_->setPen(QPen(Qt::gray,2,Qt::DashLine));
     tempLine_->setVisible(false);// We don't want it to be visible yet.
     scene_->addItem(tempLine_);

    // Create MachineGuis for the Machines in the Song.
    for(int c=0;c<psy::core::MAX_MACHINES;c++)
    {
        psy::core::Machine* mac = song_->_pMachine[c];
        if (mac) std::cout << "c:" << c << mac->mode() << std::endl;
        MachineGui *macGui;
        if (mac) { 
            switch ( mac->mode() ) {							
                case psy::core::MACHMODE_GENERATOR:
                    macGui = new MachineGui(mac->GetPosX(), mac->GetPosY(), mac, this );
                break;
                case psy::core::MACHMODE_FX:
                    macGui = new MachineGui(mac->GetPosX(), mac->GetPosY(), mac, this );
                break;
                case psy::core::MACHMODE_MASTER: 
                    macGui = new MasterGui(mac->GetPosX(), mac->GetPosY(), mac, this);
                break;
                default:
                    macGui = 0;
            }
            scene_->addItem(macGui);
            machineGuis.push_back(macGui);
        }
    }
    // Create WireGuis for connections in Song file.
    for(int c=0;c<psy::core::MAX_MACHINES;c++)
    {
        psy::core::Machine* tmac = song_->_pMachine[c];
        if (tmac) for ( int w=0; w < psy::core::MAX_CONNECTIONS; w++ )
        {
            if (tmac->_connection[w]) {
                    std::cout << "hi" << std::endl;
                MachineGui* srcMacGui = findByMachine(tmac);
                if ( srcMacGui!=0 ) {
                    psy::core::Machine *pout = song_->_pMachine[tmac->_outputMachines[w]];
                    MachineGui* dstMacGui = findByMachine(pout);
                    if ( dstMacGui != 0 ) {
                        WireGui *wireGui = new WireGui(srcMacGui, dstMacGui, this);
                        scene_->addItem( wireGui );
                    }
                }
            }
        }
    }

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
    WireGui *newWireGui = new WireGui( srcMacGui, dstMacGui, this );
    scene_->addItem( newWireGui );
 }

 void MachineView::deleteConnection( WireGui *wireGui )
 {
    qDebug("deleting connection");

    // Delete the connection in the song file.
    
    // Delete the connection in the GUI.
    scene_->removeItem( wireGui ); // FIXME: do we need to do more here?
 }

MachineGui *MachineView::findByMachine( psy::core::Machine *mac )
{
    for (std::vector<MachineGui*>::iterator it = machineGuis.begin() ; it < machineGuis.end(); it++) {
        MachineGui* machineGui = *it;
        if ( machineGui->mac() == mac ) return machineGui;
    }
    return 0;
}



