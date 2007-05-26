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
	: m_macView(macView),
	  m_mac(mac)
{
	left_ = left;
	top_ = top;

	setHandlesChildEvents( true );

	nameItem = new QGraphicsTextItem("", this );
	nameItem->setFont( QFont( "verdana", 7 ) );
	nameItem->setDefaultTextColor(Qt::white);
	nameItem->setTextWidth( 90 );
	nameItem->setPos( 5, 20 );
	nameItem->setAcceptedMouseButtons(0);

	QString string = QString::fromStdString( m_mac->GetEditName() );
	setName( QString(string) );

	setZValue( 1 );
	setRect(QRectF(0, 0, 100, 60));
	setPos(left, top);
	setPen(QPen(Qt::white,1));
	setBrush( Qt::blue );
	setFlags( ItemIsMovable | ItemIsSelectable | ItemIsFocusable );

	m_macTwkDlg = new MachineTweakDlg( this, m_macView );

	showMacTwkDlgAct_ = new QAction( "Tweak Parameters", this );
	deleteMachineAct_ = new QAction( "Delete", this );
	cloneMachineAct_ = new QAction( "Clone", this );
	renameMachineAct_ = new QAction( "Rename", this );
	QString muteText;   
	m_mac->_mute ? muteText = "Unmute" : muteText = "Mute";
	toggleMuteAct_ = new QAction( muteText, this );
	QString soloText;   
	m_mac->song()->machineSoloed == m_mac->id() ? soloText = "Unsolo" : soloText = "Solo";
	toggleSoloAct_ = new QAction( soloText, this );

	connect( showMacTwkDlgAct_, SIGNAL( triggered() ), this, SLOT( showMacTwkDlg() ) );
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


MachineGui::~MachineGui()
{
	delete m_macTwkDlg;
	// Note -- delete this here as it is parented to MachineView,
	// not MachineGui (because MachineGui isn't a QWidget... :/ )
}

void MachineGui::paint( QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget )
{
	// Do the default painting business for a QGRectItem.
	if ( this == m_macView->chosenMachine() ) {
		painter->setPen( QPen( Qt::red ) );
	} else {
		painter->setPen( QPen( Qt::white ) );
	}
	painter->setBrush( brush() );
	painter->drawRect( boundingRect() );
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
	QString text = QInputDialog::getText( m_macView, "Rename machine",
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
	if ( ( event->button() == Qt::LeftButton ) && ( m_macView->isCreatingWire() ) ) {
		emit closeNewConnection(this, event);
	} 
	else { // business as usual
		QGraphicsItem::mouseReleaseEvent(event);
	}
}

void MachineGui::mouseDoubleClickEvent( QGraphicsSceneMouseEvent * event )
{ 
	showMacTwkDlgAct_->trigger();
}

void MachineGui::showMacTwkDlg()
{
	m_macTwkDlg->show();
}

QPointF MachineGui::centrePointInSceneCoords() {
	return mapToScene( QPointF( boundingRect().width()/2, boundingRect().height()/2 ) );
}

psy::core::Machine* MachineGui::mac()
{
	return m_mac;
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

