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

#include <psycle/core/machine.h>

#include "effectgui.h"

#include "../configuration.h"
#include "../global.h"
#include "../inputhandler.h"
#include "machinetweakdlg.h"
#include "machineview.h"

#include <QMenu>
#include <QPainter>
#include <QKeyEvent>
#include <QGraphicsSceneContextMenuEvent>

EffectGui::EffectGui(int left, int top, psy::core::Machine *mac, MachineView *macView)
	: MachineGui(left, top, mac, macView)
{
	qDebug("creating effeect gui");
	m_macTweakDlg = new MachineTweakDlg( this, macView );
	showMacTweakDlgAct_ = new QAction( "Tweak Parameters", this );
	connect( showMacTweakDlgAct_, SIGNAL( triggered() ), this, SLOT( showMacTweakDlg() ) );

	setBrush( QColor( 0, 180, 0 ) );

	toggleBypassAct_ = new QAction( "Bypass", this );
	connect( toggleBypassAct_, SIGNAL( triggered() ), this, SLOT( onToggleBypassActionTriggered() ) );
}

EffectGui::~EffectGui()
{
	delete m_macTweakDlg;
}

void EffectGui::paint( QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget )
{
	MachineGui::paint( painter, option, widget );
	painter->setPen( Qt::white );
	mac()->_mute ? painter->setBrush( Qt::red ) : painter->setBrush( QColor( 100, 0, 0 ) );
	painter->drawEllipse( (int)boundingRect().width() - 15, 5, 10, 10 );
	mac()->_bypass ? painter->setBrush( Qt::yellow ) : painter->setBrush( QColor( 100, 100, 0 ) );
	painter->drawEllipse( (int)boundingRect().width() - 30, 5, 10, 10 );
}

void EffectGui::keyPressEvent( QKeyEvent * event )
{
	int command = Global::configuration().inputHandler().getEnumCodeByKey( Key( event->modifiers(), event->key() ) );
	switch ( command ) { 
		case commands::mute_machine:
		toggleMuteAct_->trigger();
		return;
		case commands::bypass_machine:
		toggleBypassAct_->trigger();
		return;
		default:;
	}
	event->ignore();
}

void EffectGui::mousePressEvent( QGraphicsSceneMouseEvent *event )
{
	///\todo adapt for skins
	QRect muteRect( (int)boundingRect().width() - 15, 5, 10, 10 );
	QRect bypassRect( (int)boundingRect().width() - 30, 5, 10, 10 );

	if ( event->button() == Qt::LeftButton ) {
		if ( event->modifiers() == Qt::ControlModifier )
			emit chosen( this );
		else if ( muteRect.contains(event->pos().toPoint()) )
			toggleMuteAct_->trigger();
		else if ( bypassRect.contains(event->pos().toPoint()) )
			toggleBypassAct_->trigger();
	}
	MachineGui::mousePressEvent( event );
}

void EffectGui::mouseReleaseEvent( QGraphicsSceneMouseEvent *event )
{
	if ( event->button() == Qt::RightButton && !m_macView->isCreatingWire() ) {
		QString muteText;
		mac()->_mute ? muteText = "Unmute" : muteText = "Mute";
		toggleMuteAct_->setText( muteText );

		QString bypassText;
		mac()->_bypass ? bypassText = "Unbypass" : bypassText = "Bypass";
		toggleBypassAct_->setText( bypassText );

		QMenu menu;
		menu.addAction( renameMachineAct_ );
		menu.addAction("Clone");
		menu.addAction( deleteMachineAct_ );
		menu.addSeparator();
		menu.addAction( showMacTweakDlgAct_ );
		menu.addSeparator();
		menu.addAction( toggleMuteAct_ );
		menu.addAction( toggleBypassAct_ );
		QAction *a = menu.exec( event->screenPos() );
	} else
		MachineGui::mouseReleaseEvent( event );
}


void EffectGui::mouseDoubleClickEvent( QGraphicsSceneMouseEvent *event )
{
	///\todo adapt for skins
	QRect muteRect( (int)boundingRect().width() - 15, 5, 10, 10 );
	QRect bypassRect( (int)boundingRect().width() - 30, 5, 10, 10 );

	if ( event->button() == Qt::LeftButton ) {
		if ( !(
			muteRect.contains( event->pos().toPoint() ) ||
			bypassRect.contains( event->pos().toPoint() )
		) )
			showMacTweakDlgAct_->trigger();
		else
			mousePressEvent(event);
	}
}

void EffectGui::showMacTweakDlg()
{
	m_macTweakDlg->show();
}

void EffectGui::onToggleBypassActionTriggered() 
{
	mac()->_bypass = !mac()->_bypass;
	if (mac()->_bypass)
	{
		mac()->_volumeCounter=0.0f;
		mac()->_volumeDisplay=0;
	}
	update( boundingRect() );
}
