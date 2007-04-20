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

#include "psycore/machine.h"
#include "psycore/inputhandler.h"
#include "psycore/configuration.h"
#include "effectgui.h"

#include <QMenu>
#include <QPainter>
#include <QKeyEvent>
#include <QGraphicsSceneContextMenuEvent>

EffectGui::EffectGui(int left, int top, psy::core::Machine *mac, MachineView *macView)
     : MachineGui(left, top, mac, macView)
{
    setBrush( QColor( 0, 180, 0 ) );

    toggleBypassAct_ = new QAction( "Bypass", this );
    connect( toggleBypassAct_, SIGNAL( triggered() ), this, SLOT( onToggleBypassActionTriggered() ) );
}

void EffectGui::paint( QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget )
{
    MachineGui::paint( painter, option, widget );
    mac()->_mute ? painter->setBrush( Qt::red ) : painter->setBrush( QColor( 100, 0, 0 ) );
    painter->drawEllipse( boundingRect().width() - 15, 5, 10, 10 );
    mac()->_bypass ? painter->setBrush( Qt::yellow ) : painter->setBrush( QColor( 100, 100, 0 ) );
    painter->drawEllipse( boundingRect().width() - 30, 5, 10, 10 );
}

void EffectGui::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
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
    menu.addAction( showMacTwkDlgAct_ );
    menu.addSeparator();
    menu.addAction( toggleMuteAct_ );
    menu.addAction( toggleBypassAct_ );
    QAction *a = menu.exec( event->screenPos() );
}

void EffectGui::keyPressEvent( QKeyEvent * event )
{
    int command = psy::core::Global::pConfig()->inputHandler().getEnumCodeByKey( psy::core::Key( event->modifiers(), event->key() ) );
    switch ( command ) { 
        case psy::core::cdefMuteMachine:
            toggleMuteAct_->trigger();
            break;
        case psy::core::cdefBypassMachine:
            toggleBypassAct_->trigger();
            break;
        default:;
    }
}

