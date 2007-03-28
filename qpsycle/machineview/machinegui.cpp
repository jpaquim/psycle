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

#include "psycore/global.h"
#include "psycore/inputhandler.h"

#include "machinegui.h"
#include "machineview.h"

 #include <QGraphicsScene>
 #include <QGraphicsSceneMouseEvent>
 #include <QPainter>
 #include <QStyleOption>
 #include <QMessageBox>
 #include <QMenu>
 #include <QAction>
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

     macTwkDlg_ = new MachineTweakDlg( mac_, machineView );
     showMacTwkDlgAct_ = new QAction("Tweak Machine", this);
     connect(showMacTwkDlgAct_, SIGNAL(triggered()), this, SLOT(showMacTwkDlg()));

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
     wireGuiList << wireGui;
     wireGui->adjust();
 }

 QVariant MachineGui::itemChange(GraphicsItemChange change, const QVariant &value)
 {
     switch (change) {
     case ItemPositionChange:
         foreach (WireGui *wireGui, wireGuiList)
             wireGui->adjust();
         break;
     default:
         break;
     };

     return QGraphicsItem::itemChange(change, value);
 }
    
  void MachineGui::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
  {
     QMenu menu;
      menu.addAction("Rename");
      menu.addAction("Clone");
      menu.addAction("Delete");
      menu.addSeparator();
      menu.addAction(showMacTwkDlgAct_);
      QAction *a = menu.exec(event->screenPos());
  }

void MachineGui::keyPressEvent ( QKeyEvent * event )
{
    int command = psy::core::Global::pConfig()->inputHandler().getEnumCodeByKey( psy::core::Key( event->modifiers(), event->key() ) );
    int note;
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
        default: 
            note = NULL;
            event->ignore();
            return;
    }
    int velocity = 127;
    if (note) {
        machineView->PlayNote( machineView->octave() * 12 + note, velocity, false, mac() );   
    }
}

void MachineGui::mousePressEvent( QGraphicsSceneMouseEvent * event )
{
    QGraphicsItem::mousePressEvent( event ); // Get the default behaviour.
    emit chosen( this );    
}

void MachineGui::mouseDoubleClickEvent( QGraphicsSceneMouseEvent * event )
{ 
    showMacTwkDlgAct_->activate(QAction::Trigger);    
}

void MachineGui::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->buttons() & Qt::MidButton) {
        qDebug("emitting new wire con..");
        emit startNewConnection(this, event);
    } 
    else { // business as usual
        QGraphicsItem::mouseMoveEvent(event);
    }
}

void MachineGui::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::MidButton) {
        qDebug("ending new wire con..");
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
