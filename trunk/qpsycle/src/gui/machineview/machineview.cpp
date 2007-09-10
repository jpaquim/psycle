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
#include <qpsyclePch.hpp>

#include <psycle/core/song.h>
#include <psycle/core/player.h>
#include <psycle/core/sampler.h>
#include <psycle/core/constants.h>
#include <psycle/core/machine.h>
#include <psycle/core/pluginfinder.h>
#include <psycle/core/patternevent.h>

#include "machineview.h"
#include "../global.h"
#include "../configuration.h"
#include "machinegui.h"
#include "mastergui.h"
#include "effectgui.h"
#include "generatorgui.h"
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
	setScene( new MachineScene( this ) );
	scene()->setBackgroundBrush(Qt::black);

	setAlignment( Qt::AlignLeft | Qt::AlignTop );
	setDragMode( QGraphicsView::RubberBandDrag );
	setBackgroundBrush(Qt::black);

	createMachineGuis();
	createWireGuis();
	createTempLine();
	initKeyjazzSettings();
}

/**
	* Loops through all the machines in the CoreSong and
	* creates a MachineGui for each one it finds.
	*/
void MachineView::createMachineGuis()
{
	if ( song() ) {
		for( int m=0; m < psy::core::MAX_MACHINES; m++ )
		{
			psy::core::Machine* mac = song()->machine(m);
			if ( mac ) {
				createMachineGui( mac );
			}
		}
	}
}

/**
	* Given a core Machine, this method makes a MachineGui
	* for it and adds it to the graphics scene. 
	*/
MachineGui * MachineView::createMachineGui( psy::core::Machine *mac )
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

	if ( macGui ) {
		if ( mac->mode() == psy::core::MACHMODE_GENERATOR ||
				mac->mode() == psy::core::MACHMODE_FX ) {
			connect( macGui, SIGNAL( chosen( MachineGui* ) ), 
					this, SLOT( onMachineChosen( MachineGui* ) ) );
			connect( macGui, SIGNAL( deleteRequest( MachineGui* ) ),
					this, SLOT( onDeleteMachineRequest( MachineGui* ) ) );
			connect( macGui, SIGNAL( renamed() ),
					this, SLOT( onMachineRenamed() ) );
			connect( macGui, SIGNAL( cloneRequest( MachineGui* ) ),
					this, SLOT( cloneMachine( MachineGui* ) ) );
		}

		scene()->addItem(macGui);
		macGui->setPos( mac->GetPosX(), mac->GetPosY() );
		machineGuis.push_back(macGui);
	}

	return macGui;
}

/**
	* Loops through all the Machines in the CoreSong and 
	* for each one loops through all its core connections,
	* adding a WireGui to the scene for each one.
	*/
void MachineView::createWireGuis() 
{
	if ( song() ) 
	{
		for( int m=0; m < psy::core::MAX_MACHINES; m++ )
		{
			psy::core::Machine* tmac = song()->machine(m);
			if (tmac) 
			{ 
				for ( int w=0; w < psy::core::MAX_CONNECTIONS; w++ )
				{
					if (tmac->_connection[w]) {
						MachineGui* srcMacGui = findMachineGuiByCoreMachine(tmac);
						if ( srcMacGui!=0 ) {
							psy::core::Machine *pout = song()->machine(tmac->_outputMachines[w]);
							MachineGui* dstMacGui = findMachineGuiByCoreMachine(pout);
							if ( dstMacGui != 0 ) {
								WireGui *wireGui = createWireGui( srcMacGui, dstMacGui );
								scene()->addItem( wireGui );
							}
						}
					}
				}
			}
		}
	}
}

/**
	* Makes a WireGui.
	* Watch out what happens in the WireGui constructor... can't remember what
	* logic goes on in it.
	*/
WireGui *MachineView::createWireGui( MachineGui *srcMacGui, MachineGui *dstMacGui )
{
	WireGui *wireGui = new WireGui(srcMacGui, dstMacGui, this);
	return wireGui;
}

/**
	* Create and add to the scene a temporary line which we show when
	* user is in the process of connecting machines.
	*/
void MachineView::createTempLine()
{
	tempLine_ = new QGraphicsLineItem(0, 0, 0, 0);
	tempLine_->setPen(QPen(Qt::gray,2,Qt::DotLine));
	tempLine_->setVisible(false);// We don't want it to be visible yet.
	scene()->addItem(tempLine_);
	creatingWire_ = false;
}


/**
	* Some thing for keyjazz.  Can't remember at present what outtrack
	* and notetrack actually do.
	*/
void MachineView::initKeyjazzSettings()
{
	outtrack = 0;
	for ( int i=0; i<psy::core::MAX_TRACKS; i++ ) notetrack[i]=120;
}


/**
	* Triggered from a signal in a MachineGui, fired when user starts a 
	* connection out of it.
	*/
void MachineView::startNewConnection(MachineGui *srcMacGui, QGraphicsSceneMouseEvent *event)
{
	tempLine_->setLine( QLineF( srcMacGui->centrePointInSceneCoords(), event->scenePos() ) );
	tempLine_->setVisible(true);
	creatingWire_ = true;
}

/**
	* Triggered from a signal in a MachineGui, fired when a mouse release
	* occurs over and while creatingWire_ is set to true.
	*/
void MachineView::closeNewConnection(MachineGui *srcMacGui, QGraphicsSceneMouseEvent *event)
{
	Q_UNUSED( event );
	MachineGui *dstMacGui = machineGuiAtPoint( tempLine_->line().p2() );
	if ( dstMacGui ) {
		connectMachines( srcMacGui, dstMacGui );
	}
	tempLine_->setVisible(false);     // We want the tempLine to disappear, whatever happens.
	creatingWire_ = false;
}


/**
	* Connects machines in the CoreSong and creates and adds a WireGui
	* to the scene.
	*/
void MachineView::connectMachines( MachineGui *srcMacGui, MachineGui *dstMacGui )
{
	if ( dstMacGui->mac()->acceptsConnections() ) {
		if (song()->InsertConnection( srcMacGui->mac()->id(), dstMacGui->mac()->id(), 1.0f))
		{
			// Make a new wiregui connection.
			WireGui *newWireGui = createWireGui( srcMacGui, dstMacGui );
			scene()->addItem( newWireGui );
		}
	}
}


/**
	* Deletes the machine from both the GUI and from
	* the CoreSong.
	* Triggered by a signal from somewhere.
	*/
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

/**
	* This can be called either when the user deletes a wire
	* directly, or a machine gets deleted and its connections
	* have to go too.
	*/
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
	scene()->removeItem( wireGui );
	delete wireGui;

	// Delete the connection in the song file.
	srcMac->Disconnect( *dstMac );

	psy::core::Player::Instance()->unlock();
}

/**
	* This just gets passed on to the MainWindow.  Maybe it can skip
	* the MachineView altogether.
	*/
void MachineView::onMachineRenamed()
{
	emit machineRenamed();
}


/**
	* Doesn't work yet as it isn't ready in psycore.
	*/
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
			if (!song()->machine(i))
			{
				dst = i;
				break;
			}
		}
	}
	else if ((src < psy::core::MAX_BUSES*2) && (src >= psy::core::MAX_BUSES))
	{
		// MAX_BUSES*2 is where FX begin.
		for (psy::core::Machine::id_type i(psy::core::MAX_BUSES); i < psy::core::MAX_BUSES*2; i++)
		{
			if (!song()->machine(i))
			{
				dst = i;
				break;
			}
		}
	}
	if (dst >= 0)
	{
		
		if (!song()->CloneMac(src,dst))
		{
			qDebug("Cloning failed");
		}
		else {
			qDebug("Cloning doesn't work yet."); // See Song::CloneMac().
		}
	} 

}


/**
	* This is called when a new machine is added via
	* the NewMachineDialog.
	* 
	*/
void MachineView::addNewMachineGui( psy::core::Machine *mac )
{
	MachineGui *macGui = createMachineGui( mac );

	if ( mac->mode() == psy::core::MACHMODE_GENERATOR ) {
		setChosenMachine( macGui );
		song()->seqBus = song()->FindBusFromIndex( macGui->mac()->id() );
		emit newMachineCreated( mac );
	}
	scene()->update( scene()->itemsBoundingRect() );
}

/**
	* Being 'chosen' refers to the machine being chosen
	* to respond to keyjazz keypresses.
	*/
void MachineView::onMachineChosen( MachineGui *macGui )
{
	song()->seqBus = song()->FindBusFromIndex( macGui->mac()->id() );

	setChosenMachine( macGui );
	scene()->update( scene()->itemsBoundingRect() );

	emit machineChosen( macGui );
}



MachineGui *MachineView::findMachineGuiByCoreMachine( psy::core::Machine *mac )
{
	for (std::vector<MachineGui*>::iterator it = machineGuis.begin() ; it < machineGuis.end(); it++) {
		MachineGui* machineGui = *it;
		if ( machineGui->mac() == mac ) return machineGui;
	}
	return 0;
}
/// <nmather> probably don't need both of these methods.
MachineGui *MachineView::findMachineGuiByCoreMachineIndex( int index )
{
	for (std::vector<MachineGui*>::iterator it = machineGuis.begin() ; it < machineGuis.end(); it++) {
		MachineGui* machineGui = *it;
		if ( machineGui->mac()->id() == index ) return machineGui;
	}
	return 0;
}


/**
	* Given a point in the scene we return the MachineGui at
	* that point,  if there's one there.  
	*/
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


/**
	* For keyjazz.
	*/
void MachineView::playNote( int note,int velocity,bool bTranspose, psy::core::Machine *pMachine )
{

	// stop any notes with the same value
	stopNote(note,bTranspose,pMachine);

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
	//entry.setMachine( song()->seqBus ); // Not really needed.

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
		//if(bMultiKey)
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
			//outtrack=0;
		//}
		// this should check to see if a note is playing on that track
		if (notetrack[outtrack] < 120) {
			stopNote(notetrack[outtrack], bTranspose, pMachine);
		}

		// play
		notetrack[outtrack]=note;
		pMachine->Tick(outtrack, entry );
	}
}

/**
	* For keyjazz again, on key release.
	*/
void MachineView::stopNote( int note, bool bTranspose, psy::core::Machine * pMachine )
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


/**
	* Figure out which note to play from the command
	* given.
	*
	* <nmather> I think commands might correspond to the
	* correct notes directly at present, but best not to
	* rely on that always being the case.
	*/
int MachineView::noteFromCommand( int command )
{
	int note = NULL;
	switch ( command ) {
		case commands::key_C_0:
		note = 1;
		break;
		case commands::key_CS0:
		note = 2;
		break;
		case commands::key_D_0:
		note = 3;
		break;
		case commands::key_DS0:
		note = 4;
		break;
		case commands::key_E_0:
		note = 5;
		break;
		case commands::key_F_0:
		note = 6;
		break;
		case commands::key_FS0:
		note = 7;
		break;
		case commands::key_G_0:
		note = 8;
		break;
		case commands::key_GS0:
		note = 9;
		break;
		case commands::key_A_0:
		note = 10;
		break;
		case commands::key_AS0:
		note = 11;
		break;
		case commands::key_B_0: 
		note = 12;
		break;
		case commands::key_C_1:
		note = 13;
		break;
		case commands::key_CS1:
		note = 14;
		break;
		case commands::key_D_1:
		note = 15;
		break;
		case commands::key_DS1:
		note = 16;
		break;
		case commands::key_E_1:
		note = 17;
		break;
		case commands::key_F_1:
		note = 18;
		break;
		case commands::key_FS1:
		note = 19;
		break;
		case commands::key_G_1:
		note = 20;
		break;
		case commands::key_GS1:
		note = 21;
		break;
		case commands::key_A_1:
		note = 22;
		break;
		case commands::key_AS1:
		note = 23;
		break;
		case commands::key_B_1: 
		note = 24;
		break;
		case commands::key_C_2:
		note = 25;
		break;
		case commands::key_CS2:
		note = 26;
		break;
		case commands::key_D_2:
		note = 27;
		break;
		case commands::key_DS2:
		note = 28;
		break;
		case commands::key_E_2:
		note = 29;
		break;
	}
	return note;
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


/**
	* Some cheap code for zooming in and out of the view --
	* probably buggy and more trouble than it's worth in the
	* long run.
	*/
void MachineView::scaleView(qreal scaleFactor) 
{
	qreal factor = matrix().scale(scaleFactor, scaleFactor).mapRect(QRectF(0, 0, 1, 1)).width();
	if (factor < 0.07 || factor > 100)
		return;

	scale(scaleFactor, scaleFactor);
}


// Getters and setters.
psy::core::Song *MachineView::song()
{
	return song_;
}

int MachineView::octave() const
{
	return octave_;
}

void MachineView::setOctave( int newOctave )
{
	octave_ = newOctave;
}

void MachineView::setChosenMachine( MachineGui *macGui )
{ 
	chosenMachine_ = macGui;
}







/**
	* MachineScene.
	*/
MachineScene::MachineScene( MachineView *macView )
	:
	QGraphicsScene( macView ),
	pluginFinder_(Global::configuration().pluginPath(), Global::configuration().ladspaPath())
{
	macView_ = macView;
	newMachineDlg = new NewMachineDlg();
}

/**
	* Shows the NewMachineDialog on double-click.
	* This method would perhaps be better in MacView (as we only
	* care about dbl-clicks in the viewport, not in the entirety of
	* the scene) but I couldn't get it to work there iirc.
	*/
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
				macView_->addNewMachineGui( mac );

				update();
			}
		} 
	}
}


/**
	* Deals with keyjazz keypresses, rather than the individual MachineGuis,
	* for reasons which I forget at present.
	*/
void MachineScene::keyPressEvent( QKeyEvent * event )
{
	if ( macView_->chosenMachine() ) 
	{
		if ( !event->isAutoRepeat() ) 
		{
			int command = Global::configuration().inputHandler().getEnumCodeByKey( Key( event->modifiers(), event->key() ) );
			int note = NULL;
			note = macView_->noteFromCommand( command );
			if (note) {
				onNotePress( note, macView_->chosenMachine()->mac() );
			}
		}
	}
	event->ignore();
}

// FIXME: this gets triggered even when you're still holding the key down.  
// Most likely a Qt bug...
void MachineScene::keyReleaseEvent( QKeyEvent * event )
{
	int command = Global::configuration().inputHandler().getEnumCodeByKey( Key( event->modifiers(), event->key() ) );

	int note = macView_->noteFromCommand( command );
	if (note) {
		onNoteRelease( note );
	}
	event->ignore();
}

void MachineScene::onNotePress( int note, psy::core::Machine* mac )
{
	macView_->playNote( macView_->octave() * 12 + note, 127, false, mac );   
}

void MachineScene::onNoteRelease( int note )
{
	macView_->stopNote( note );   
}

