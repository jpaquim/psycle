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

 MachineGui::MachineGui(int left, int top, MachineView *macView)
     : machineView(macView)
 {
     left_ = left;
     top_ = top;
     nameItem = new QGraphicsTextItem("", this);
     nameItem->setDefaultTextColor(Qt::white);

     setRect(QRectF(0, 0, 100, 60));
     setPos(left, top);
     setPen(QPen(Qt::white,1));
     setBrush(QBrush(Qt::blue));
     setFlag(ItemIsMovable);
     setFlag(ItemIsSelectable);
     setZValue(1);
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
      QAction *a = menu.exec(event->screenPos());
  }

  void MachineGui::mouseDoubleClickEvent ( QGraphicsSceneMouseEvent * event )
  { 
        QDialog dialog(machineView);
        dialog.exec();
  }


