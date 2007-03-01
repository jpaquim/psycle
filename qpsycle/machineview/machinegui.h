/***************************************************************************
*   Copyright (C) 2006 by  Neil Mather   *
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
#ifndef MACHINEGUI_H
#define MACHINEGUI_H

 #include <QGraphicsItem>
 #include "machineview.h"
 #include "wiregui.h"

 class MachineGui : public QGraphicsRectItem
 {

 public:
     MachineGui(int left, int top, MachineView *macView);
     void addWireGui(WireGui *wireGui);
     void setName(const QString &name);

 protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant &value);
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent * event);

 private:
     MachineView *machineView;
     QGraphicsTextItem *nameItem;
     int left_;
     int top_;
     QList<WireGui *> wireGuiList;
 };

 #endif
