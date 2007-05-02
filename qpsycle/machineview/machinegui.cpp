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

#include "psycore/signalslib.h"
#include "psycore/global.h"
#include "psycore/inputhandler.h"
#include "psycore/machine.h"
#include "psycore/song.h"
#include "psycore/constants.h"
#include "psycore/configuration.h"

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

 MachineGui::MachineGui(int left, int top, psy::core::Machine *mac, MachineView *macView)
     : machineView(macView)
 {
     mac_ = mac;
     left_ = left;
     top_ = top;
     nameItem = new QGraphicsTextItem("", this);
     nameItem->setDefaultTextColor(Qt::white);

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
     renameMachineAct_ = new QAction( "Rename", this );
     QString muteText;   
     mac_->_mute ? muteText = "Unmute" : muteText = "Mute";
     toggleMuteAct_ = new QAction( muteText, this );
     QString soloText;   
     mac_->song()->machineSoloed == mac_->_macIndex ? soloText = "Unsolo" : soloText = "Solo";
     toggleSoloAct_ = new QAction( soloText, this );

     connect( showMacTwkDlgAct_, SIGNAL( triggered() ), this, SLOT( showMacTwkDlg() ) );
     connect( deleteMachineAct_, SIGNAL( triggered() ), this, SLOT( onDeleteMachineActionTriggered() ) );
     connect( renameMachineAct_, SIGNAL( triggered() ), this, SLOT( onRenameMachineActionTriggered() ) );
     connect( toggleMuteAct_, SIGNAL( triggered() ), this, SLOT( onToggleMuteActionTriggered() ) );
     connect( toggleSoloAct_, SIGNAL( triggered() ), this, SLOT( onToggleSoloActionTriggered() ) );

     connect( this, SIGNAL(startNewConnection(MachineGui*, QGraphicsSceneMouseEvent*)), 
              machineView, SLOT(startNewConnection(MachineGui*, QGraphicsSceneMouseEvent*)) );
     connect( this, SIGNAL(closeNewConnection(MachineGui*, QGraphicsSceneMouseEvent*)), 
              machineView, SLOT(closeNewConnection(MachineGui*, QGraphicsSceneMouseEvent*)) );
 }

 void MachineGui::paint( QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget )
 {
    // FIXME: not a good idea to do anything intensive in the paint method...
    if ( this == machineView->theChosenOne() ) {
        painter->setPen( QPen( Qt::red ) );
    }
    // Do the default painting business for a QGRectItem.
    QGraphicsRectItem::paint( painter, option, widget );
    painter->setPen( QPen( Qt::white ) );
 }

 void MachineGui::setName(const QString &name)
 {
     nameItem->setPlainText(name);
 }

void MachineGui::addWireGui(WireGui *wireGui)
{
    wireGuiList_ << wireGui;
    wireGui->adjust();
}

QList<WireGui *> MachineGui::wireGuiList()
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
        setName( text );
        mac()->SetEditName( text.toStdString() );
    }
    emit renamed();
}

QVariant MachineGui::itemChange(GraphicsItemChange change, const QVariant &value)
{
    switch (change) {
    case ItemPositionChange:
        foreach (WireGui *wireGui, wireGuiList_)
            wireGui->adjust();
        break;
    default:
        break;
    };

    return QGraphicsItem::itemChange(change, value);
}
    
void MachineGui::mousePressEvent( QGraphicsSceneMouseEvent * event )
{
    QGraphicsItem::mousePressEvent( event ); // Get the default behaviour.
    emit chosen( this );    
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
        if ( mac()->song()->machineSoloed == mac()->_macIndex ) {
            mac()->song()->machineSoloed = -1;
        }
    }

    update( boundingRect() );
}

void MachineGui::onToggleSoloActionTriggered() 
{
    if (mac()->song()->machineSoloed == mac()->_macIndex ) // Unsolo it.
    {
        mac()->song()->machineSoloed = -1;
        for ( int i=0;i<psy::core::MAX_MACHINES;i++ ) {
            if ( mac()->song()->_pMachine[i] ) {
                if (( mac()->song()->_pMachine[i]->_mode == psy::core::MACHMODE_GENERATOR )) {
                    mac()->song()->_pMachine[i]->_mute = false;
                }
            }
        }
    } else { // Solo it.
        for ( int i=0;i<psy::core::MAX_MACHINES;i++ ) {
            if ( mac()->song()->_pMachine[i] )
            {
                if (( mac()->song()->_pMachine[i]->_mode == psy::core::MACHMODE_GENERATOR ) && (i != mac()->_macIndex))
                {
                    mac()->song()->_pMachine[i]->_mute = true;
                    mac()->song()->_pMachine[i]->_volumeCounter=0.0f;
                    mac()->song()->_pMachine[i]->_volumeDisplay=0;
                }
            }
        }
        mac()->_mute = false;
        mac()->song()->machineSoloed = mac()->_macIndex;
    }

    scene()->update(); // FIXME: possibly more efficient to update individual machines in the loop above.
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
    MachineGui::paint( painter, option, widget );
    mac()->_mute ? painter->setBrush( Qt::red ) : painter->setBrush( QColor( 100, 0, 0 ) );
    painter->drawEllipse( boundingRect().width() - 15, 5, 10, 10 );
    mac()->song()->machineSoloed == mac()->_macIndex ?painter->setBrush( Qt::green ) : painter->setBrush( QColor( 0, 100, 0 ) );
    painter->drawEllipse( boundingRect().width() - 30, 5, 10, 10 );
}

void GeneratorGui::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    QString muteText;
    mac_->_mute ? muteText = "Unmute" : muteText = "Mute";
    toggleMuteAct_->setText( muteText );

    QString soloText;   
    mac_->song()->machineSoloed == mac_->_macIndex ? soloText = "Unsolo" : soloText = "Solo";
    toggleSoloAct_->setText( soloText );

    QMenu menu;
    menu.addAction( renameMachineAct_ );
    menu.addAction("Clone");
    menu.addAction( deleteMachineAct_ );
    menu.addSeparator();
    menu.addAction( showMacTwkDlgAct_ );
    menu.addSeparator();
    menu.addAction( toggleMuteAct_ );
    menu.addAction( toggleSoloAct_ );
    QAction *a = menu.exec( event->screenPos() );
}

void GeneratorGui::keyPressEvent( QKeyEvent * event )
{
    if ( !event->isAutoRepeat() ) 
    {
        int command = psy::core::Global::pConfig()->inputHandler().getEnumCodeByKey( psy::core::Key( event->modifiers(), event->key() ) );

        switch ( command ) { 
            case psy::core::cdefMuteMachine:
                toggleMuteAct_->trigger();
                return;
            case psy::core::cdefSoloMachine:
                toggleSoloAct_->trigger();
                return;
        }

        int note = NULL;
        note = noteFromCommand( command );
        if (note) {
            onNotePress( note, mac() );
        }
    }
}

// FIXME: this gets triggered even when you're still holding the key down.  
// Most likely a Qt bug...
void GeneratorGui::keyReleaseEvent( QKeyEvent * event )
{
    int command = psy::core::Global::pConfig()->inputHandler().getEnumCodeByKey( psy::core::Key( event->modifiers(), event->key() ) );
    switch ( command ) { 
        case psy::core::cdefMuteMachine:
            toggleMuteAct_->trigger();
            return;
        case psy::core::cdefSoloMachine:
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
        case psy::core::cdefKeyC_0:
            note = 1;
            break;
        case psy::core::cdefKeyCS0:
            note = 2;
            break;
        case psy::core::cdefKeyD_0:
            note = 3;
            break;
        case psy::core::cdefKeyDS0:
            note = 4;
            break;
        case psy::core::cdefKeyE_0:
            note = 5;
            break;
        case psy::core::cdefKeyF_0:
            note = 6;
            break;
        case psy::core::cdefKeyFS0:
            note = 7;
            break;
        case psy::core::cdefKeyG_0:
            note = 8;
            break;
        case psy::core::cdefKeyGS0:
            note = 9;
            break;
        case psy::core::cdefKeyA_0:
            note = 10;
            break;
        case psy::core::cdefKeyAS0:
            note = 11;
            break;
        case psy::core::cdefKeyB_0: 
            note = 12;
            break;
        case psy::core::cdefKeyC_1:
            note = 13;
            break;
        case psy::core::cdefKeyCS1:
            note = 14;
            break;
        case psy::core::cdefKeyD_1:
            note = 15;
            break;
        case psy::core::cdefKeyDS1:
            note = 16;
            break;
        case psy::core::cdefKeyE_1:
            note = 17;
            break;
        case psy::core::cdefKeyF_1:
            note = 18;
            break;
        case psy::core::cdefKeyFS1:
            note = 19;
            break;
        case psy::core::cdefKeyG_1:
            note = 20;
            break;
        case psy::core::cdefKeyGS1:
            note = 21;
            break;
        case psy::core::cdefKeyA_1:
            note = 22;
            break;
        case psy::core::cdefKeyAS1:
            note = 23;
            break;
        case psy::core::cdefKeyB_1: 
            note = 24;
            break;
        case psy::core::cdefKeyC_2:
            note = 25;
            break;
        case psy::core::cdefKeyCS2:
            note = 26;
            break;
        case psy::core::cdefKeyD_2:
            note = 27;
            break;
        case psy::core::cdefKeyDS2:
            note = 28;
            break;
        case psy::core::cdefKeyE_2:
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

