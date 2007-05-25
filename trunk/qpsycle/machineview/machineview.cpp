/***************************************************************************
*   Copyright (C) 2007 Psycledelics Community   *
*   psycle.sourceforge.net   *
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

#include <psycore/song.h>
#include <psycore/player.h>
#include <psycore/sampler.h>
#include <psycore/constants.h>
#include <psycore/machine.h>
#include <psycore/pluginfinder.h>
#include <psycore/patternevent.h>

#include "../global.h"
#include "../configuration.h"
#include "machinegui.h"
#include "machineview.h"
#include "mastergui.h"
#include "effectgui.h"
#include "wiregui.h"
#include "newmachinedlg.h"

#include <QtGui/QGraphicsScene>
#include <QPainter>
#include <iostream>
#include <QGraphicsLineItem>
#include <QDebug>

#include <algorithm>

MachineView::MachineView(psy::core::Song *song)
{
	song_ = song;
	scene_ = new MachineScene(this);
	scene_->setBackgroundBrush(Qt::black);
	setScene(scene_);
	setAlignment( Qt::AlignLeft | Qt::AlignTop );
	setDragMode( QGraphicsView::RubberBandDrag );
	setBackgroundBrush(Qt::black);

	createMachineGuis();
	createWireGuis();
	createTempLine();
	initKeyjazzSettings();
}

void MachineView::createMachineGuis()
{
	if ( song_ ) {
		for(int c=0;c<psy::core::MAX_MACHINES;c++)
		{
			psy::core::Machine* mac = song_->machine(c);
			if ( mac ) {
				createMachineGui( mac );
			}
		}
	}
}

void MachineView::createMachineGui( psy::core::Machine *mac )
{
	MachineGui *macGui;
	switch ( mac->mode() ) {							
        case psy::core::MACHMODE_GENERATOR:
		macGui = new GeneratorGui(mac->GetPosX(), mac->GetPosY(), mac, this );
		break;
        case psy::core::MACHMODE_FX:
		macGui = new EffectGui(mac->GetPosX(), mac->GetPosY(), mac, this );
		break;
        case psy::core::MACHMODE_MASTER: 
		macGui = new MasterGui(mac->GetPosX(), mac->GetPosY(), mac, this);
		break;
        default:
		macGui = 0;
	}

	if ( mac->mode() == psy::core::MACHMODE_GENERATOR ||
	     mac->mode() == psy::core::MACHMODE_FX ) {
		connect( macGui, SIGNAL( chosen( MachineGui* ) ), 
			 this, SLOT( onMachineChosen( MachineGui* ) ) );
		connect( macGui, SIGNAL( chosen( MachineGui* ) ), 
			 this, SLOT( onMachineChosen( MachineGui* ) ) );
		connect( macGui, SIGNAL( deleteRequest( MachineGui* ) ),
			 this, SLOT( onDeleteMachineRequest( MachineGui* ) ) );
		connect( macGui, SIGNAL( renamed() ),
			 this, SLOT( onMachineRenamed() ) );
		connect( macGui, SIGNAL( cloneRequest( MachineGui* ) ),
			 this, SLOT( cloneMachine( MachineGui* ) ) );
	}
	scene_->addItem(macGui);
	machineGuis.push_back(macGui);
}

void MachineView::createWireGuis() 
{
	if ( song_ ) {
		for(int c=0;c<psy::core::MAX_MACHINES;c++)
		{
			psy::core::Machine* tmac = song_->machine(c);
			if (tmac) for ( int w=0; w < psy::core::MAX_CONNECTIONS; w++ )
				  {
					  if (tmac->_connection[w]) {
						  MachineGui* srcMacGui = findByMachine(tmac);
						  if ( srcMacGui!=0 ) {
							  psy::core::Machine *pout = song_->machine(tmac->_outputMachines[w]);
							  MachineGui* dstMacGui = findByMachine(pout);
							  if ( dstMacGui != 0 ) {
								  WireGui *wireGui = createWireGui( srcMacGui, dstMacGui );
								  scene_->addItem( wireGui );
							  }
						  }
					  }
				  }
		}
	}
}

void MachineView::createTempLine()
{
	// A temporary line to display when user is making a new connection.
	tempLine_ = new QGraphicsLineItem(0, 0, 0, 0);
	tempLine_->setPen(QPen(Qt::gray,2,Qt::DotLine));
	tempLine_->setVisible(false);// We don't want it to be visible yet.
	scene_->addItem(tempLine_);
	creatingWire_ = false;
}


void MachineView::initKeyjazzSettings()
{
	outtrack = 0;
	notetrack[psy::core::MAX_TRACKS];
	for ( int i=0; i<psy::core::MAX_TRACKS; i++ ) notetrack[i]=120;
}


WireGui *MachineView::createWireGui( MachineGui *srcMacGui, MachineGui *dstMacGui )
{
    WireGui *wireGui = new WireGui(srcMacGui, dstMacGui, this);
    return wireGui;
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


 void MachineView::scaleView(qreal scaleFactor) 
 {
      qreal factor = matrix().scale(scaleFactor, scaleFactor).mapRect(QRectF(0, 0, 1, 1)).width();
     if (factor < 0.07 || factor > 100)
         return;

     scale(scaleFactor, scaleFactor);
 }

void MachineView::startNewConnection(MachineGui *srcMacGui, QGraphicsSceneMouseEvent *event)
{
    tempLine_->setLine( QLineF( srcMacGui->centrePointInSceneCoords(), event->scenePos() ) );
    tempLine_->setVisible(true);
    creatingWire_ = true;
}

void MachineView::closeNewConnection(MachineGui *srcMacGui, QGraphicsSceneMouseEvent *event)
{
	MachineGui *dstMacGui = machineGuiAtPoint( tempLine_->line().p2() );
	if ( dstMacGui ) {
		connectMachines( srcMacGui, dstMacGui );
	}
	tempLine_->setVisible(false);     // We want the tempLine to disappear, whatever happens.
	creatingWire_ = false;
}

MachineGui *MachineView::machineGuiAtPoint( QPointF point )
{
	std::vector<MachineGui*>::iterator iter;
	for( iter = machineGuis.begin(); iter != machineGuis.end(); iter++ ) {
		MachineGui *macGui = *iter;
		if ( macGui->contains( macGui->mapFromScene( point ) ) )
		{
			return macGui;
		}
	}
	return 0;
}

void MachineView::connectMachines( MachineGui *srcMacGui, MachineGui *dstMacGui )
{
	if ( dstMacGui->mac()->acceptsConnections() ) {
		if (song_->InsertConnection( srcMacGui->mac()->id(), dstMacGui->mac()->id(), 1.0f))
		{
			// Make a new wiregui connection.
			WireGui *newWireGui = createWireGui( srcMacGui, dstMacGui );
			scene_->addItem( newWireGui );
		}
	}
}

void MachineView::onDeleteMachineRequest( MachineGui *macGui )
{
	int id = macGui->mac()->id();

	// Remove machine and connections from the gui. 
	std::vector<WireGui*>::iterator wIt;
	while ( true ) {
		// Loop this way as deleteConnection removes
		// wireGuis from the wireGuiList.
		wIt = macGui->wireGuiList_.begin();
		if ( wIt != macGui->wireGuiList_.end() ) {
			deleteConnection( *wIt );
		} else break;
	}

	std::vector<MachineGui*>::iterator it;
	for ( it = machineGuis.begin(); it != machineGuis.end(); it++ ) {
		if ( macGui == *it ) {
			machineGuis.erase( it );
			break;
		}

	}
	scene()->removeItem( macGui );
	delete macGui;

	// Remove machine and connections from the Song. 
	song()->DestroyMachine( id );


	emit machineDeleted( id ); 
}

void MachineView::onMachineRenamed()
{
	emit machineRenamed();
}

void MachineView::deleteConnection( WireGui *wireGui )
{
	psy::core::Player::Instance()->lock();

	psy::core::Machine *srcMac = wireGui->sourceMacGui()->mac();
	psy::core::Machine *dstMac = wireGui->destMacGui()->mac();

	// Delete the connection in the GUI.
	std::vector<WireGui*>::iterator it;
	it = std::find( wireGui->sourceMacGui()->wireGuiList_.begin(),
			wireGui->sourceMacGui()->wireGuiList_.end(), wireGui );
	if ( it != wireGui->sourceMacGui()->wireGuiList_.end() ) {
		wireGui->sourceMacGui()->wireGuiList_.erase(it);	
 	}  
	it = std::find( wireGui->destMacGui()->wireGuiList_.begin(),
			wireGui->destMacGui()->wireGuiList_.end(), wireGui );
	if ( it != wireGui->destMacGui()->wireGuiList_.end() ) {
		wireGui->destMacGui()->wireGuiList_.erase(it);	
 	}  
	scene_->removeItem( wireGui );
	delete wireGui;

	// Delete the connection in the song file.
	srcMac->Disconnect( *dstMac );

	psy::core::Player::Instance()->unlock();
}

MachineGui *MachineView::findByMachine( psy::core::Machine *mac )
{
	for (std::vector<MachineGui*>::iterator it = machineGuis.begin() ; it < machineGuis.end(); it++) {
		MachineGui* machineGui = *it;
		if ( machineGui->mac() == mac ) return machineGui;
	}
	return 0;
}

MachineGui *MachineView::findMachineGuiByMachineIndex( int index )
{
	for (std::vector<MachineGui*>::iterator it = machineGuis.begin() ; it < machineGuis.end(); it++) {
		MachineGui* machineGui = *it;
		if ( machineGui->mac()->id() == index ) return machineGui;
	}
	return 0;
}

psy::core::Song *MachineView::song()
{
	return song_;
}

void MachineView::PlayNote( int note,int velocity,bool bTranspose, psy::core::Machine *pMachine )
{

	// stop any notes with the same value
	StopNote(note,bTranspose,pMachine);

	if(note<0) return;

	// octave offset
	if(note<120) {
		if(bTranspose)
			note+= octave()*12;
		if (note > 119)
			note = 119;
	}

	// build entry
	psy::core::PatternEvent entry;
	entry.setNote( note );
	entry.setInstrument( song()->auxcolSelected );
//    entry.setMachine( song()->seqBus );	// Not really needed.

	entry.setCommand( 0 );
	entry.setParameter( 0 );

	// play it
	if(pMachine==NULL)
	{
		int mgn = song()->seqBus;

		if (mgn < psy::core::MAX_MACHINES) {
			pMachine = song()->machine(mgn);
		}
	}

	if (pMachine) {
		// pick a track to play it on	
		//        if(bMultiKey)
		{
			int i;
			for (i = outtrack+1; i < song()->tracks(); i++)
			{
				if (notetrack[i] == 120) {
					break;
				}
			}
			if (i >= song()->tracks()) {
				for (i = 0; i <= outtrack; i++) {
					if (notetrack[i] == 120) {
						break;
					}
				}
			}
			outtrack = i;
		}// else  {
		//   outtrack=0;
		//}
		// this should check to see if a note is playing on that track
		if (notetrack[outtrack] < 120) {
			StopNote(notetrack[outtrack], bTranspose, pMachine);
		}

		// play
		notetrack[outtrack]=note;
		pMachine->Tick(outtrack, entry );
	}
}

void MachineView::StopNote( int note, bool bTranspose, psy::core::Machine * pMachine )
{
    if (!(note >=0 && note < 128)) return;

    // octave offset
    if(note<120) {
        if(bTranspose) note+= octave()*12;
        if (note > 119) note = 119;
    }

    if(pMachine==NULL) {
        int mgn = song()->seqBus;

        if (mgn < psy::core::MAX_MACHINES) {
            pMachine = song()->machine(mgn);
        }

        for(int i=0; i<song()->tracks(); i++) {
            if(notetrack[i]==note) {
                notetrack[i]=120;
                // build entry
                psy::core::PatternEvent entry;
                entry.setNote( 120+0 );
                entry.setInstrument( song()->auxcolSelected );
                entry.setMachine( song()->seqBus );
                entry.setCommand( 0 );
                entry.setParameter( 0 );

                // play it
                if (pMachine) {
                    pMachine->Tick( i, entry );
                }
            }
        }

    }
}


void MachineView::onMachineChosen( MachineGui *macGui )
{
    setChosenMachine( macGui );
    emit machineChosen( macGui );
    update();
}

int MachineView::octave() const
{
    return octave_;
}

void MachineView::setOctave( int newOctave )
{
    octave_ = newOctave;
}

MachineScene::MachineScene( MachineView *macView )
:
	QGraphicsScene( macView ),
	pluginFinder_(Global::configuration().pluginPath(), Global::configuration().ladspaPath())
{
    macView_ = macView;
    newMachineDlg = new NewMachineDlg();
}

void MachineScene::mouseDoubleClickEvent( QGraphicsSceneMouseEvent *event )
{ 
    QGraphicsScene::mouseDoubleClickEvent( event );
    if ( !event->isAccepted() ) // Check whether one of the items on the scene ate it.
    {
        int accepted = newMachineDlg->exec();
        if (accepted) { // Add a new machine to the song.
             psy::core::PluginFinderKey key = newMachineDlg->pluginKey(); 

            // Create machine, tell where to place the new machine--get from mouse.	  
            psy::core::Machine *mac = macView_->song()->createMachine( pluginFinder_, key, event->scenePos().x(), event->scenePos().y() );
            if ( mac ) {
                macView_->createMachineGui( mac );
                emit newMachineCreated( mac );
                update();
            }
        } 
    }
}

void MachineView::cloneMachine( MachineGui *macGui )
{
	qDebug("in clone machine");
	psy::core::Machine *pMachine = macGui->mac();
	psy::core::Machine::id_type src( pMachine->id() );
	psy::core::Machine::id_type dst(-1);

	if ((src < psy::core::MAX_BUSES) && (src >=0))
	{
		// we need to find an empty slot
		for (psy::core::Machine::id_type i(0); i < psy::core::MAX_BUSES; i++)
		{
			if (!song_->machine(i))
			{
				dst = i;
				break;
			}
		}
	}
	else if ((src < psy::core::MAX_BUSES*2) && (src >= psy::core::MAX_BUSES))
	{
		for (psy::core::Machine::id_type i(psy::core::MAX_BUSES); i < psy::core::MAX_BUSES*2; i++)
		{
			if (!song_->machine(i))
			{
				dst = i;
				break;
			}
		}
	}
	if (dst >= 0)
	{
      
		if (!song_->CloneMac(src,dst))
		{
			qDebug("Cloning failed");
		}
		else {
			qDebug("Cloning doesn't work yet."); // See Song::CloneMac().
		}
	} 

}
