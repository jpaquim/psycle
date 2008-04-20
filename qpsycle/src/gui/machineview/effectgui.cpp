// -*- mode:c++; indent-tabs-mode:t -*-
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

#include "effectgui.hpp"

#include "../configuration.hpp"
#include "../global.hpp"
#include "../inputhandler.hpp"
#include "mixertweakdlg.hpp"
#include "machinetweakdlg.hpp"
#include "machineview.hpp"

#include <QMenu>
#include <QPainter>
#include <QKeyEvent>
#include <QGraphicsSceneContextMenuEvent>

namespace qpsycle {

EffectGui::EffectGui(int left, int top, psy::core::Machine *mac, MachineView *macView)
	: MachineGui(left, top, mac, macView)
	, isPanning(false)
{
	qDebug("creating effect gui");
	m_macTweakDlg = 0;

	showMacTweakDlgAct_ = new QAction( "Tweak Parameters", this );
	connect( showMacTweakDlgAct_, SIGNAL( triggered() ), this, SLOT( showMacTweakDlg() ) );

	setBrush( QColor( 0, 180, 0 ) );

	toggleBypassAct_ = new QAction( "Bypass", this );
	connect( toggleBypassAct_, SIGNAL( triggered() ), this, SLOT( onToggleBypassActionTriggered() ) );

	///\todo adapt for skins
	muteRect = new QRect( (int)boundingRect().width() - 15, 5, 10, 10 );
	bypassRect = new QRect( (int)boundingRect().width() - 30, 5, 10, 10 );
	panRect = new QRect(5, (int)boundingRect().height()-15, (int)boundingRect().width()-11, 10 );
	panRange = panRect->width()-panRect->height(); //assumes square thumb

}

EffectGui::~EffectGui()
{
	delete m_macTweakDlg;
	delete muteRect;
	delete bypassRect;
	delete panRect;

}

void EffectGui::paint( QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget )
{
	MachineGui::paint( painter, option, widget );
	painter->setPen( Qt::white );
	mac()->_mute ? painter->setBrush( Qt::red ) : painter->setBrush( QColor( 100, 0, 0 ) );
	painter->drawEllipse( *muteRect );
	mac()->_bypass ? painter->setBrush( Qt::yellow ) : painter->setBrush( QColor( 100, 100, 0 ) );
	painter->drawEllipse( *bypassRect );

	// panning
	painter->setBrush( QColor( 16, 16, 16 ) );
	painter->setPen( Qt::NoPen );
	painter->drawRect( *panRect );

	int thumbpos = panRect->x() + (int)(panRange * mac()->Pan() / 128);
	QRect panThumb( thumbpos, panRect->y(), panRect->height(), panRect->height() );
	if ( mac()->Pan() == 64 || mac()->Pan() == 0 || mac()->Pan() == 128 )
		painter->setBrush( QColor( 224, 224, 224) );
	else
		painter->setBrush( QColor( 192, 192, 192 ) );
	painter->drawRect( panThumb );
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
	QRect panArea(
		(int)boundingRect().x(),
		(int)boundingRect().y() + (int)boundingRect().height()*2/3,
		(int)boundingRect().width(),
		(int)boundingRect().height()/3
	);

	if ( event->button() == Qt::LeftButton ) {
		if ( event->modifiers() == Qt::ControlModifier )
			emit chosen( this );
		else if ( muteRect->contains(event->pos().toPoint()) )
			toggleMuteAct_->trigger();
		else if ( bypassRect->contains(event->pos().toPoint()) )
			toggleBypassAct_->trigger();
		else if ( panArea.contains(event->pos().toPoint()) ) {
			isPanning = true;
			startPanPos = event->pos().toPoint().x();
			startPan = mac()->Pan();
		}
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
		menu.exec( event->screenPos() );
	} else if ( isPanning )
		isPanning = false;
	else
		MachineGui::mouseReleaseEvent( event );
}


void EffectGui::mouseDoubleClickEvent( QGraphicsSceneMouseEvent *event )
{
	if ( event->button() == Qt::LeftButton ) {
		if ( !(
			muteRect->contains( event->pos().toPoint() ) ||
			bypassRect->contains( event->pos().toPoint() )
		) )
			showMacTweakDlgAct_->trigger();
		else
			mousePressEvent(event);
	}
}

void EffectGui::mouseMoveEvent( QGraphicsSceneMouseEvent *event )
{
	if ( isPanning ) {
		int moved = (int)event->pos().x() - startPanPos;
		int newpan = startPan + (int)(128 * moved / panRange);
		if ( newpan < 0 ) newpan = 0;
		else if (newpan > 128) newpan = 128;
		mac()->SetPan( newpan );
		update(*panRect);
	} else
		MachineGui::mouseMoveEvent( event );
}

void EffectGui::showMacTweakDlg()
{
	if ( !m_macTweakDlg )
	{
		if ( m_mac->getMachineKey() == psy::core::MachineKey::mixer())
			m_macTweakDlg = new MixerTweakDlg( this, m_macView);
		else
			m_macTweakDlg = new MachineTweakDlg( this, m_macView);
	}/* else if (mac()->setupChanged()){
		m_macTweakDlg->regenerateUI();
	}*/
	else //doing it the easy way for now.
	{
		delete m_macTweakDlg;

		if ( m_mac->getMachineKey() == psy::core::MachineKey::mixer())
			m_macTweakDlg = new MixerTweakDlg( this, m_macView );
		else
			m_macTweakDlg = new MachineTweakDlg( this, m_macView);
	}

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

} // namespace qpsycle
