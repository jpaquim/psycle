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

#ifndef WIREGUI_H
 #define WIREGUI_H

 #include <QGraphicsItem>

 class MachineGui;

 class WireGui : public QGraphicsItem
 {
 public:
     WireGui(MachineGui *sourceMacGui, MachineGui *destMacGui);
     ~WireGui();

     MachineGui *sourceMacGui() const;
     void setSourceMacGui(MachineGui *macGui);

     MachineGui *destMacGui() const;
     void setDestMacGui(MachineGui *macGui);

     void adjust();

 protected:
     QRectF boundingRect() const;
     void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

 private:
     MachineGui *source, *dest;

     QPointF sourcePoint;
     QPointF destPoint;
     qreal arrowSize;
 };

 #endif

