/***************************************************************************
*   Copyright (C) 2007 by Psycledelics Community   *
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

#include <psycore/signalslib.h>
#include <psycore/machine.h>
#include <psycore/song.h>
#include <psycore/constants.h>

#include "../global.h"
#include "../configuration.h"
#include "inputhandler.h"
#include "machinegui.h"
#include "machineview.h"
#include "wiregui.h"
#include "machinetweakdlg.h"

#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QStyleOption>
#include <QMessageBox>
#include <QMenu>
#include <QAction>
#include <QInputDialog>

#include <iostream>
#include <iomanip>

MachineGui::MachineGui(int left, int top, psy::core::Machine *mac, MachineView *macView)
	: machineView(macView)
{
	mac_ = mac;
	left_ = left;
	top_ = top;

	setHandlesChildEvents( true );

	nameItem = new QGraphicsTextItem("", this );
	nameItem->setFont( QFont( "verdana", 7 ) );
	nameItem->setDefaultTextColor(Qt::white);
	nameItem->setTextWidth( 90 );
	nameItem->setPos( 5, 20 );
	nameItem->setAcceptedMouseButtons(0);

	QString string = QString::fromStdString( mac->GetEditName() );
	setName( QString(string) );

	setZValue( 1 );
	setRect(QRectF(0, 0, 100, 60));
	setPos(left, top);
	setPen(QPen(Qt::white,1));
	setBrush( Qt::blue );
	setFlags( ItemIsMovable | ItemIsSelectable | ItemIsFocusable );

	macTwkDlg_ = new MachineTweakDlg( this, machineView );

	showMacTwkDlgAct_ = new QAction( "Tweak Parameters", this );
	deleteMachineAct_ = new QAction( "Delete", this );
	cloneMachineAct_ = new QAction( "Clone", this );
	renameMachineAct_ = new QAction( "Rename", this );
	QString muteText;   
	mac_->_mute ? muteText = "Unmute" : muteText = "Mute";
	toggleMuteAct_ = new QAction( muteText, this );
	QString soloText;   
	mac_->song()->machineSoloed == mac_->id() ? soloText = "Unsolo" : soloText = "Solo";
	toggleSoloAct_ = new QAction( soloText, this );

	connect( showMacTwkDlgAct_, SIGNAL( triggered() ), this, SLOT( showMacTwkDlg() ) );
	connect( deleteMachineAct_, SIGNAL( triggered() ), this, SLOT( onDeleteMachineActionTriggered() ) );
	connect( renameMachineAct_, SIGNAL( triggered() ), this, SLOT( onRenameMachineActionTriggered() ) );
	connect( toggleMuteAct_, SIGNAL( triggered() ), this, SLOT( onToggleMuteActionTriggered() ) );
	connect( toggleSoloAct_, SIGNAL( triggered() ), this, SLOT( onToggleSoloActionTriggered() ) );
	connect( cloneMachineAct_, SIGNAL( triggered() ), this, SLOT( onCloneMachineActionTriggered() ) );

	connect( this, SIGNAL(startNewConnection(MachineGui*, QGraphicsSceneMouseEvent*)), 
		 machineView, SLOT(startNewConnection(MachineGui*, QGraphicsSceneMouseEvent*)) );
	connect( this, SIGNAL(closeNewConnection(MachineGui*, QGraphicsSceneMouseEvent*)), 
		 machineView, SLOT(closeNewConnection(MachineGui*, QGraphicsSceneMouseEvent*)) );
}


MachineGui::~MachineGui()
{
	delete macTwkDlg_;
	// Note -- delete this here as it is parented to MachineView,
	// not MachineGui (because MachineGui isn't a QWidget... :/ )
}

void MachineGui::paint( QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget )
{
	// Do the default painting business for a QGRectItem.
	QGraphicsRectItem::paint( painter, option, widget );
	painter->setPen( QPen( Qt::white ) );
}

void MachineGui::setName(const QString &name)
{
	std::ostringstream buffer;
	buffer.setf(std::ios::uppercase);
	buffer.str("");
	buffer << std::setfill('0') << std::hex << std::setw(2);
	buffer << mac()->id() << ": " << mac()->GetEditName();

	nameItem->setPlainText( QString::fromStdString( buffer.str() ) );
}

void MachineGui::addWireGui(WireGui *wireGui)
{
	wireGuiList_.push_back( wireGui );
	wireGui->adjust();
}

std::vector<WireGui *> MachineGui::wireGuiList()
{
    return wireGuiList_;
}

void MachineGui::onRenameMachineActionTriggered()
{
	bool ok;
	QString text = QInputDialog::getText( machineView, "Rename machine",
					      "Name: ", QLineEdit::Normal,
					      QString::fromStdString( mac()->GetEditName() ), &ok);
	if ( ok && !text.isEmpty() ) {
		mac()->SetEditName( text.toStdString() );
		setName( text );
	}
	emit renamed();
}

QVariant MachineGui::itemChange(GraphicsItemChange change, const QVariant &value)
{
	switch (change) {
	case ItemPositionChange:
	{
		std::vector<WireGui*>::iterator it = wireGuiList_.begin();
		for ( ; it != wireGuiList_.end(); it++ ) {
			WireGui *wireGui = *it;
			wireGui->adjust();
		}
	}
	break;
	default:
		break;
	};

	return QGraphicsItem::itemChange(change, value);
}
    
void MachineGui::mousePressEvent( QGraphicsSceneMouseEvent * event )
{
	QGraphicsItem::mousePressEvent( event ); // Get the default behaviour.
}

void MachineGui::mouseDoubleClickEvent( QGraphicsSceneMouseEvent * event )
{ 
	showMacTwkDlgAct_->trigger();
}

void MachineGui::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
	if ( ( event->buttons() == Qt::LeftButton ) && ( event->modifiers() == Qt::ShiftModifier ) ) {
		emit startNewConnection(this, event);
	} 
	else { // Default implementation takes care of moving the MacGui.
		QGraphicsItem::mouseMoveEvent(event);
	}
}

void MachineGui::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
	if ( ( event->button() == Qt::LeftButton ) && ( machineView->isCreatingWire() ) ) {
		emit closeNewConnection(this, event);
	} 
	else { // business as usual
		QGraphicsItem::mouseReleaseEvent(event);
	}
}

void MachineGui::showMacTwkDlg()
{
	macTwkDlg_->show();
}

QPointF MachineGui::centrePointInSceneCoords() {
	return mapToScene( QPointF( boundingRect().width()/2, boundingRect().height()/2 ) );
}

psy::core::Machine* MachineGui::mac()
{
	return mac_;
}

void MachineGui::onDeleteMachineActionTriggered()
{
	emit deleteRequest( this );
}

void MachineGui::onToggleMuteActionTriggered() 
{
	mac()->_mute = !mac()->_mute;
	if ( mac()->_mute ) 
	{
		mac()->_volumeCounter = 0.0f;
		mac()->_volumeDisplay = 0;
		if ( mac()->song()->machineSoloed == mac()->id() ) {
			mac()->song()->machineSoloed = -1;
		}
	}

	update( boundingRect() );
}

void MachineGui::onToggleSoloActionTriggered() 
{
	if (mac()->song()->machineSoloed == mac()->id() ) // Unsolo it.
	{
		mac()->song()->machineSoloed = -1;
		for ( int i=0;i<psy::core::MAX_MACHINES;i++ ) {
			if ( mac()->song()->machine(i) ) {
				if (( mac()->song()->machine(i)->mode() == psy::core::MACHMODE_GENERATOR )) {
					mac()->song()->machine(i)->_mute = false;
				}
			}
		}
	} else { // Solo it.
		for ( int i=0;i<psy::core::MAX_MACHINES;i++ ) {
			if ( mac()->song()->machine(i) )
			{
				if (( mac()->song()->machine(i)->mode() == psy::core::MACHMODE_GENERATOR ) && (i != mac()->id()))
				{
					mac()->song()->machine(i)->_mute = true;
					mac()->song()->machine(i)->_volumeCounter=0.0f;
					mac()->song()->machine(i)->_volumeDisplay=0;
				}
			}
		}
		mac()->_mute = false;
		mac()->song()->machineSoloed = mac()->id();
	}

	scene()->update(); // FIXME: possibly more efficient to update individual machines in the loop above.
}

void MachineGui::onCloneMachineActionTriggered()
{
	emit cloneRequest( this );
}



/**
 * GeneratorGui
 */
GeneratorGui::GeneratorGui(int left, int top, psy::core::Machine *mac, MachineView *macView)
	: MachineGui(left, top, mac, macView)
{
	connect( macTwkDlg_, SIGNAL( notePress( int, psy::core::Machine* ) ),
		 this, SLOT( onNotePress( int, psy::core::Machine* ) ) );
	connect( macTwkDlg_, SIGNAL( noteRelease( int ) ),
		 this, SLOT( onNoteRelease( int ) ) );
}

void GeneratorGui::paint( QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget )
{
	// FIXME: not a good idea to do anything intensive in the paint method...
	if ( this == machineView->chosenMachine() ) {
		painter->setPen( QPen( Qt::red ) );
	}

	MachineGui::paint( painter, option, widget );
	mac()->_mute ? painter->setBrush( Qt::red ) : painter->setBrush( QColor( 100, 0, 0 ) );
	painter->drawEllipse( boundingRect().width() - 15, 5, 10, 10 );
	mac()->song()->machineSoloed == mac()->id() ? painter->setBrush( Qt::green ) : painter->setBrush( QColor( 0, 100, 0 ) );
	painter->drawEllipse( boundingRect().width() - 30, 5, 10, 10 );
}

void GeneratorGui::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
	QString muteText;
	mac_->_mute ? muteText = "Unmute" : muteText = "Mute";
	toggleMuteAct_->setText( muteText );

	QString soloText;   
	mac_->song()->machineSoloed == mac_->id() ? soloText = "Unsolo" : soloText = "Solo";
	toggleSoloAct_->setText( soloText );

	QMenu menu;
	menu.addAction( renameMachineAct_ );
	menu.addAction( cloneMachineAct_ );
	menu.addAction( deleteMachineAct_ );
	menu.addSeparator();
	menu.addAction( showMacTwkDlgAct_ );
	menu.addSeparator();
	menu.addAction( toggleMuteAct_ );
	menu.addAction( toggleSoloAct_ );
	QAction *a = menu.exec( event->screenPos() );
}

void GeneratorGui::mousePressEvent( QGraphicsSceneMouseEvent *event )
{
	if ( event->button() == Qt::LeftButton ) {
		emit chosen( this );    
	}
	QGraphicsItem::mousePressEvent( event );
}

void GeneratorGui::keyPressEvent( QKeyEvent * event )
{
	if ( !event->isAutoRepeat() ) 
	{
		int command = Global::configuration().inputHandler().getEnumCodeByKey( Key( event->modifiers(), event->key() ) );

		switch ( command ) { 
		case commands::mute_machine:
			toggleMuteAct_->trigger();
			return;
		case commands::solo_machine:
			toggleSoloAct_->trigger();
			return;
		}

		int note = NULL;
		note = noteFromCommand( command );
		if (note) {
			onNotePress( note, mac() );
		}
	}
	event->ignore();
}

// FIXME: this gets triggered even when you're still holding the key down.  
// Most likely a Qt bug...
void GeneratorGui::keyReleaseEvent( QKeyEvent * event )
{
	int command = Global::configuration().inputHandler().getEnumCodeByKey( Key( event->modifiers(), event->key() ) );
	switch ( command ) { 
        case commands::mute_machine:
		toggleMuteAct_->trigger();
		return;
        case commands::solo_machine:
		toggleSoloAct_->trigger();
		return;
        default:;
	}

	int note = noteFromCommand( command );
	if (note) {
		onNoteRelease( note );
	}
	event->ignore();
}

void GeneratorGui::onNotePress( int note, psy::core::Machine* mac )
{
	machineView->PlayNote( machineView->octave() * 12 + note, 127, false, mac );   
}

void GeneratorGui::onNoteRelease( int note )
{
	machineView->StopNote( note );   
}

// FIXME: should be somewhere else, perhaps global.
int MachineGui::noteFromCommand( int command )
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



// FIXME: should be in EffectGui, but Qt fails to recognise it there.
void MachineGui::onToggleBypassActionTriggered() 
{
    mac()->_bypass = !mac()->_bypass;
    if (mac()->_bypass)
    {
        mac()->_volumeCounter=0.0f;	
        mac()->_volumeDisplay=0;
    }
    update( boundingRect() );
}

