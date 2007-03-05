/***************************************************************************
*   Copyright (C) 2007 by Neil Mather   *
*   nmather@sourceforge   *
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
#include "machinegui.h"

 #include <QGraphicsScene>
 #include <QGraphicsSceneMouseEvent>
 #include <QPainter>
 #include <QStyleOption>
 #include <QMessageBox>
 #include <QMouseEvent>
 #include <QMenu>
 #include <QAction>
 #include <iostream>

 #include "machineview.h"

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

     setRect(QRectF(0, 0, 100, 60));
     setPos(left, top);
     setPen(QPen(Qt::white,1));
     setBrush( Qt::blue );
     setFlag(ItemIsMovable);
     setFlag(ItemIsSelectable);
     setFlag(ItemIsFocusable);
     setZValue(1);

     macTwkDlg_ = new MachineTweakDlg(machineView);
     showMacTwkDlgAct_ = new QAction("Tweak Machine", this);
     connect(showMacTwkDlgAct_, SIGNAL(triggered()), this, SLOT(showMacTwkDlg()));

     connect( this, SIGNAL(startNewConnection(MachineGui*, QGraphicsSceneMouseEvent*)), 
              machineView, SLOT(startNewConnection(MachineGui*, QGraphicsSceneMouseEvent*)) );
     connect( this, SIGNAL(closeNewConnection(MachineGui*, QGraphicsSceneMouseEvent*)), 
              machineView, SLOT(closeNewConnection(MachineGui*, QGraphicsSceneMouseEvent*)) );
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
    int key = event->key();
    int note;
    switch (key) { // FIXME: shouldn't be hardcoded.
        case Qt::Key_Z: // C_0
            note = 1;
            break;
        case Qt::Key_S: // CS_0
            note = 2;
            break;
        case Qt::Key_X: // D_0
            note = 3;
            break;
        case Qt::Key_D: // DS_0
            note = 4;
            break;
        case Qt::Key_C: // E_0
            note = 5;
            break;
        case Qt::Key_V: // F_0
            note = 6;
            break;
        case Qt::Key_G: // FS_0
            note = 7;
            break;
        case Qt::Key_B: // G_0
            note = 8;
            break;
        case Qt::Key_H: // GS_0
            note = 9;
            break;
        case Qt::Key_N: // A_0
            note = 10;
            break;
        case Qt::Key_J: // AS_0
            note = 11;
            break;
        case Qt::Key_M: // B_0
            note = 12;
            break;
        default: note = NULL;
    }
    int octave = 4;
    int velocity = 127;
    if (note) {
        machineView->PlayNote( octave * 12 + note, velocity, false, mac() );   
    }
}

void MachineGui::mouseDoubleClickEvent ( QGraphicsSceneMouseEvent * event )
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
    std::cout << event->buttons() << std::endl;
    std::cout << event->button() << std::endl;
    std::cout << event->modifiers() << std::endl;
    if (event->button() == Qt::MidButton) {
        qDebug("ending new wire con..");
        emit closeNewConnection(this, event);
    } 
    else { // business as usual
        QGraphicsItem::mouseReleaseEvent(event);
    }
}

//void MachineGui::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)

void MachineGui::showMacTwkDlg()
{
    macTwkDlg_->exec();
}

QPointF MachineGui::centrePointInSceneCoords() {
    return mapToScene( QPointF( boundingRect().width()/2, boundingRect().height()/2 ) );
}

psy::core::Machine* MachineGui::mac()
{
    return mac_;
}
