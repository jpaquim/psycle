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

 #include "machineview.h"

 MachineGui::MachineGui(int left, int top, MachineView *macView)
     : machineView(macView)
 {
     setRect(QRectF(left, top, 100, 60));
     setPen(QPen(Qt::gray,5));
     setBrush(QBrush(Qt::darkGray));
     left_ = left;
     top_ = top;
     setFlag(ItemIsMovable);
     setZValue(1);
 }

/*  QRectF MachineGui::boundingRect() const
 {
     qreal adjust = 2;
     return QRectF(left_ - adjust, top_ - adjust,
                   100 + adjust, 60 + adjust);
 }

 QPainterPath MachineGui::shape() const
 {
      QPainterPath path;
     path.addRect(left_, top_, 100, 60);
     return path;
 }

 void MachineGui::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
 {
     painter->setPen(QPen(Qt::red, 0));
     painter->drawRect(left_, top_, 100, 60);
 }*/
