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

#include <psycle/core/signalslib.h>
#include <psycle/core/machine.h>
#include <psycle/core/song.h>

#include "generatorgui.h"
#include "../global.h"
#include "../configuration.h"
#include "machineview.h"
#include "machinetweakdlg.h"

#include <QPainter>
#include <QMenu>

GeneratorGui::GeneratorGui(int left, int top, psy::core::Machine *mac, MachineView *macView)
	: MachineGui(left, top, mac, macView)
{
	m_macTwkDlg = new MachineTweakDlg( this, macView );
	showMacTwkDlgAct_ = new QAction( "Tweak Parameters", this );
	connect( showMacTwkDlgAct_, SIGNAL( triggered() ), this, SLOT( showMacTwkDlg() ) );
}

GeneratorGui::~GeneratorGui()
{
	delete m_macTwkDlg;
}

void GeneratorGui::paint( QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget )
{
	MachineGui::paint( painter, option, widget );
	painter->setPen( Qt::white );
	mac()->_mute ? painter->setBrush( Qt::red ) : painter->setBrush( QColor( 100, 0, 0 ) );
	painter->drawEllipse( boundingRect().width() - 15, 5, 10, 10 );
	mac()->song()->machineSoloed == mac()->id() ? painter->setBrush( Qt::green ) : painter->setBrush( QColor( 0, 100, 0 ) );
	painter->drawEllipse( boundingRect().width() - 30, 5, 10, 10 );
}

void GeneratorGui::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
	QString muteText;
	m_mac->_mute ? muteText = "Unmute" : muteText = "Mute";
	toggleMuteAct_->setText( muteText );

	QString soloText;   
	m_mac->song()->machineSoloed == m_mac->id() ? soloText = "Unsolo" : soloText = "Solo";
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
	MachineGui::mousePressEvent( event );
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

void GeneratorGui::showMacTwkDlg()
{
	m_macTwkDlg->show();
}

void GeneratorGui::mouseDoubleClickEvent( QGraphicsSceneMouseEvent *event )
{
	if ( event->button() == Qt::LeftButton )
		showMacTwkDlgAct_->trigger();
}
