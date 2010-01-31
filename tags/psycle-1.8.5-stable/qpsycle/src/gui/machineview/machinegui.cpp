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
#include <qpsyclePch.hpp>

#include <psycle/core/signalslib.h>
#include <psycle/core/machine.h>
#include <psycle/core/song.h>
#include <psycle/core/constants.h>

#include "../global.h"
#include "../configuration.h"
#include "../inputhandler.h"
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


/**
	* The graphical representation of a CoreSong machine in the GUI.
	*/
MachineGui::MachineGui(int left, int top, psy::core::Machine *mac, MachineView *macView)
	: m_macView(macView),
		m_mac(mac),
		left_(left),
		top_(top_)
{
	setHandlesChildEvents( true );
	setFlags( ItemIsMovable | ItemIsSelectable | ItemIsFocusable );
	setZValue( 1 );

	initGraphics();
	initActions();
	initSignals();
}


MachineGui::~MachineGui()
{}

void MachineGui::initGraphics()
{
	nameItem = new QGraphicsTextItem("", this );
	nameItem->setFont( QFont( "verdana", 7 ) );
	nameItem->setDefaultTextColor(Qt::white);
	nameItem->setTextWidth( 90 );
	nameItem->setPos( 5, 20 );
	nameItem->setAcceptedMouseButtons(0);

	QString string = QString::fromStdString( m_mac->GetEditName() );
	setName( QString(string) );

	setRect(QRectF(0, 0, 100, 60));
	setPen(QPen(Qt::white,1));
	setBrush( Qt::blue );
}

void MachineGui::initActions()
{
	deleteMachineAct_ = new QAction( "Delete", this );
	cloneMachineAct_ = new QAction( "Clone", this );
	renameMachineAct_ = new QAction( "Rename", this );
	QString muteText;   
	m_mac->_mute ? muteText = "Unmute" : muteText = "Mute";
	toggleMuteAct_ = new QAction( muteText, this );
	QString soloText;   
	m_mac->song()->machineSoloed == m_mac->id() ? soloText = "Unsolo" : soloText = "Solo";
	toggleSoloAct_ = new QAction( soloText, this );
}

void MachineGui::initSignals()
{
	connect( deleteMachineAct_, SIGNAL( triggered() ), this, SLOT( onDeleteMachineActionTriggered() ) );
	connect( renameMachineAct_, SIGNAL( triggered() ), this, SLOT( onRenameMachineActionTriggered() ) );
	connect( toggleMuteAct_, SIGNAL( triggered() ), this, SLOT( onToggleMuteActionTriggered() ) );
	connect( toggleSoloAct_, SIGNAL( triggered() ), this, SLOT( onToggleSoloActionTriggered() ) );
	connect( cloneMachineAct_, SIGNAL( triggered() ), this, SLOT( onCloneMachineActionTriggered() ) );

	connect( this, SIGNAL(startNewConnection(MachineGui*, QGraphicsSceneMouseEvent*)), 
			m_macView, SLOT(startNewConnection(MachineGui*, QGraphicsSceneMouseEvent*)) );
	connect( this, SIGNAL(closeNewConnection(MachineGui*, QGraphicsSceneMouseEvent*)), 
			m_macView, SLOT(closeNewConnection(MachineGui*, QGraphicsSceneMouseEvent*)) );
}

/**
	* Note some paint operations are handled in subclasses.
	*/
void MachineGui::paint( QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget )
{
	if ( this == m_macView->chosenMachine() ) {
		painter->setPen( QPen( Qt::red ) );
	} else {
		painter->setPen( QPen( Qt::white ) );
	}
	painter->setBrush( brush() );
	painter->drawRect( boundingRect() );

	if ( isSelected() ) {
		painter->setBrush( QColor( 255, 255, 0, 100 ) );
		painter->drawRect( boundingRect() );
	}
}

/**
	* Updates the name in the song and updates the GUI.
	*/
void MachineGui::setName(const QString &name)
{
	std::ostringstream buffer;
	buffer.setf(std::ios::uppercase);
	buffer.str("");
	buffer << std::setfill('0') << std::hex << std::setw(2);
	buffer << mac()->id() << ": " << mac()->GetEditName();

	nameItem->setPlainText( QString::fromStdString( buffer.str() ) );
}

/**
	* This gets called from WireGuis when they are created -- they
	* add themselves to their parent's list.
	*/
void MachineGui::addWireGui(WireGui *wireGui)
{
	wireGuiList_.push_back( wireGui );
	wireGui->adjust();
}


QPointF MachineGui::centrePointInSceneCoords() {
	return mapToScene( QPointF( boundingRect().width()/2, boundingRect().height()/2 ) );
}


/**
	* Rename machine and send out a signal.
	*/
void MachineGui::onRenameMachineActionTriggered()
{
	bool ok;
	QString text = QInputDialog::getText( m_macView, "Rename machine",
							"Name: ", QLineEdit::Normal,
							QString::fromStdString( mac()->GetEditName() ), &ok);
	if ( ok && !text.isEmpty() ) {
		mac()->SetEditName( text.toStdString() );
		setName( text );
	}
	emit renamed();
}

void MachineGui::onDeleteMachineActionTriggered()
{
	emit deleteRequest( this );
}

/**
	* Mute/unmute in the CoreSong and update the GUI.
	*/
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

/**
	* Solo/unsolo in the CoreSong and update the GUI.
	*/
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
	* Called when the item changes in some way (we're interested in when it moves.)
	* See QGraphicsItem::itemChange in the Qt API.
	*/
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


/**
	* Gets subclassed by EffectGui, GeneratorGui etc.
	*/
void MachineGui::mousePressEvent( QGraphicsSceneMouseEvent * event )
{
	QGraphicsRectItem::mousePressEvent( event ); // Get the default behaviour.
}

void MachineGui::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
	if ( ( event->buttons() == Qt::LeftButton ) && ( event->modifiers() == Qt::ShiftModifier ) ) {
		emit startNewConnection(this, event);
	} 
	else { // Default Qt implementation can take care of moving the MacGui.
		QGraphicsItem::mouseMoveEvent(event);
	}
}


void MachineGui::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
	if ( ( event->button() == Qt::LeftButton ) && ( m_macView->isCreatingWire() ) ) {
		emit closeNewConnection(this, event);
	} 
	else { // business as usual
		QGraphicsItem::mouseReleaseEvent(event);
	}
}



// Getters.
std::vector<WireGui *> MachineGui::wireGuiList()
{
	return wireGuiList_;
}

psy::core::Machine* MachineGui::mac()
{
	return m_mac;
}