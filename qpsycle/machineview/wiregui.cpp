/***************************************************************************
*   Copyright (C) 2007 by  Neil Mather   *
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

#include <QPainter>
#include <QGraphicsSceneContextMenuEvent>
#include <QMenu>
#include <QAction>

 #include "wiregui.h"
 #include "machinegui.h"

 #include <math.h>

 static const double Pi = 3.14159265358979323846264338327950288419717;
 static double TwoPi = 2.0 * Pi;

 WireGui::WireGui(MachineGui *sourceMacGui, MachineGui *destMacGui)
     : arrowSize(20)
 {
     setAcceptedMouseButtons(0);
     source = sourceMacGui;
     dest = destMacGui;
     source->addWireGui(this);
     dest->addWireGui(this);
     adjust();

     delConnAct_ = new QAction("Delete Connection", this);
     connect(delConnAct_, SIGNAL(triggered()), this, SLOT(deleteConnectionRequest()));
 }

 WireGui::~WireGui()
 {
 }

 MachineGui *WireGui::sourceMacGui() const
 {
     return source;
 }

 void WireGui::setSourceMacGui(MachineGui *macGui)
 {
     source = macGui;
     adjust();
 }

 MachineGui *WireGui::destMacGui() const
 {
     return dest;
 }

 void WireGui::setDestMacGui(MachineGui *macGui)
 {
     dest = macGui;
     adjust();
 }

  void WireGui::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
  {
     QMenu menu;
      menu.addAction( delConnAct_ );
      menu.addSeparator();
      menu.addAction("Rewire Connection Source");
      menu.addAction("Rewire Connection Destination");
      QAction *a = menu.exec(event->screenPos());
  }

 void WireGui::adjust()
 {
     if (!source || !dest)
         return;

     QLineF line(mapFromItem(source, source->boundingRect().width()/2, source->boundingRect().height()/2), 
                 mapFromItem(dest, dest->boundingRect().width()/2, dest->boundingRect().height()/2)); 
     qreal length = line.length();
     QPointF wireGuiOffset((line.dx() * 10) / length, (line.dy() * 10) / length);

     removeFromIndex();
     sourcePoint = line.p1() + wireGuiOffset;
     destPoint = line.p2() - wireGuiOffset;
     addToIndex();
 }

 QRectF WireGui::boundingRect() const
 {
     if (!source || !dest)
         return QRectF();

     qreal penWidth = 1;
     qreal extra = (penWidth + arrowSize) / 2.0;

     return QRectF(sourcePoint, QSizeF(destPoint.x() - sourcePoint.x(),
                                       destPoint.y() - sourcePoint.y()))
         .normalized()
         .adjusted(-extra, -extra, extra, extra);
 }

 void WireGui::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
 {
     if (!source || !dest)
         return;

     // Draw the line.
     QLineF line(sourcePoint, destPoint);
     painter->setPen(QPen(Qt::white, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
     painter->drawLine(line);

     // Draw the arrow.
     // FIXME: arrow isn't quite in centre of line (observe two machines close to each other)
     double angle = ::acos(line.dx() / line.length());
     if (line.dy() >= 0)
         angle = TwoPi - angle;

     QPointF midPoint = sourcePoint + QPointF((destPoint.x()-sourcePoint.x())/2, 
                                  (destPoint.y()-sourcePoint.y())/2);
     QPointF arrowP1 = midPoint + QPointF(sin(angle - Pi / 3) * arrowSize,
                                               cos(angle - Pi / 3) * arrowSize);
     QPointF arrowP2 = midPoint + QPointF(sin(angle - Pi + Pi / 3) * arrowSize,
                                               cos(angle - Pi + Pi / 3) * arrowSize);

     painter->setBrush(Qt::darkGray);
     painter->drawPolygon(QPolygonF() << midPoint << arrowP1 << arrowP2);
 }

 void WireGui::deleteConnectionRequest()
 {
    emit deleteConnectionRequest(this);
 }

