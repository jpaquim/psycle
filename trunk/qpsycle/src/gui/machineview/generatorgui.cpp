// -*- mode:c++; indent-tabs-mode:t -*-
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

#include <psycle/core/signalslib.h>
#include <psycle/core/machine.h>
#include <psycle/core/song.h>

#include "generatorgui.hpp"
#include "../global.hpp"
#include "../configuration.hpp"
#include "machineview.hpp"
#include "machinetweakdlg.hpp"

#include <QPainter>
#include <QMenu>

namespace qpsycle {

GeneratorGui::GeneratorGui(int left, int top, psy::core::Machine *mac, MachineView *macView)
	: MachineGui(left, top, mac, macView)
	, isPanning(false)
{
	m_macTweakDlg = new MachineTweakDlg( this, macView );
	connect(m_macTweakDlg, SIGNAL( notePress( int, psy::core::Machine* )),
					macView, SLOT( onNotePress( int, psy::core::Machine* ) ) );
	connect(m_macTweakDlg, SIGNAL( noteRelease( int, psy::core::Machine* )),
					macView, SLOT( onNoteRelease( int, psy::core::Machine* ) ) );
	showMacTweakDlgAct_ = new QAction( "Tweak Parameters", this );
	connect( showMacTweakDlgAct_, SIGNAL( triggered() ), this, SLOT( showMacTweakDlg() ) );

	///\todo adapt for skins
	muteRect = new QRect( (int)boundingRect().width() - 15, 5, 10, 10 );
	soloRect = new QRect( (int)boundingRect().width() - 30, 5, 10, 10 );
	panRect = new QRect(5, (int)boundingRect().height()-15, (int)boundingRect().width()-11, 10 );
	panRange = panRect->width()-panRect->height(); //assumes square thumb
}

GeneratorGui::~GeneratorGui()
{
	delete m_macTweakDlg;
	delete muteRect;
	delete soloRect;
	delete panRect;
}

void GeneratorGui::paint( QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget )
{
	MachineGui::paint( painter, option, widget );
	painter->setPen( Qt::white );
	mac()->_mute ? painter->setBrush( Qt::red ) : painter->setBrush( QColor( 100, 0, 0 ) );
	painter->drawEllipse( *muteRect );
	dynamic_cast<psy::core::Song*>(mac()->song())->machineSoloed == mac()->id() ? painter->setBrush( Qt::green ) : painter->setBrush( QColor( 0, 100, 0 ) );
	painter->drawEllipse( *soloRect );

	//panning
	painter->setBrush( QColor( 16, 16, 16 ) );
	painter->setPen( Qt::NoPen );
	painter->drawRect( *panRect );

	int thumbpos = panRect->x() + (int)(panRange * mac()->_panning / 128);
	QRect panThumb( thumbpos, panRect->y(), panRect->height(), panRect->height() );
	if ( mac()->_panning == 64 || mac()->_panning == 0 || mac()->_panning == 128 )
		painter->setBrush( QColor( 224, 224, 224) );
	else
		painter->setBrush( QColor( 192, 192, 192 ) );
	painter->drawRect( panThumb );
}

void GeneratorGui::mouseReleaseEvent( QGraphicsSceneMouseEvent *event )
{
	if ( event->button() == Qt::RightButton && !m_macView->isCreatingWire() ) {
		QString muteText;
		m_mac->_mute ? muteText = "Unmute" : muteText = "Mute";
		toggleMuteAct_->setText( muteText );
	
		QString soloText;
		dynamic_cast<psy::core::Song*>(m_mac->song())->machineSoloed == m_mac->id() ? soloText = "Unsolo" : soloText = "Solo";
		toggleSoloAct_->setText( soloText );
	
		QMenu menu;
		menu.addAction( renameMachineAct_ );
		menu.addAction( cloneMachineAct_ );
		menu.addAction( deleteMachineAct_ );
		menu.addSeparator();
		menu.addAction( showMacTweakDlgAct_ );
		menu.addSeparator();
		menu.addAction( toggleMuteAct_ );
		menu.addAction( toggleSoloAct_ );
		menu.exec( event->screenPos() );
	} else if ( isPanning )
		isPanning = false;
	else
		MachineGui::mouseReleaseEvent( event );
}

void GeneratorGui::mousePressEvent( QGraphicsSceneMouseEvent *event )
{
	QRect panArea(
		(int)boundingRect().x(),
		(int)boundingRect().y() + (int)boundingRect().height()*2/3,
		(int)boundingRect().width(),
		(int)boundingRect().height()/3
	);

	if ( event->button() == Qt::LeftButton ) {
		if ( muteRect->contains(event->pos().toPoint()) )
			toggleMuteAct_->trigger();
		else if ( soloRect->contains(event->pos().toPoint()) )
			toggleSoloAct_->trigger();
		else if ( panArea.contains(event->pos().toPoint()) ) {
			isPanning = true;
			startPanPos = event->pos().toPoint().x();
			startPan = mac()->_panning;
		} else
			emit chosen( this );
	}
	MachineGui::mousePressEvent( event );
}

void GeneratorGui::mouseMoveEvent( QGraphicsSceneMouseEvent *event )
{
	if ( isPanning ) {
		int moved = (int)event->pos().x() - startPanPos;
		int newpan = startPan + (int)(128 * moved / panRange);
		if ( newpan < 0 ) newpan = 0;
		else if (newpan > 128) newpan = 128;
		mac()->SetPan( newpan );
		update(*panRect);
	} else
		MachineGui::mouseMoveEvent(event);
}

void GeneratorGui::keyPressEvent( QKeyEvent *event )
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
}

void GeneratorGui::keyReleaseEvent( QKeyEvent *event )
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
}

void GeneratorGui::showMacTweakDlg()
{
	m_macTweakDlg->show();
}

void GeneratorGui::mouseDoubleClickEvent( QGraphicsSceneMouseEvent *event )
{
	if ( event->button() == Qt::LeftButton ) {
		if ( !(
			muteRect->contains( event->pos().toPoint() ) ||
			soloRect->contains( event->pos().toPoint() )
		) )
			showMacTweakDlgAct_->trigger();
		else
			mousePressEvent(event);
	}
}

} // namespace qpsycle
