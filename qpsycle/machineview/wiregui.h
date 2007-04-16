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

#ifndef WIREGUI_H
#define WIREGUI_H

#include "psycore/signalslib.h"

#include <QGraphicsItem>
#include <QAction>

class MachineGui;
class MachineView;

 class WireGui : public QObject, public QGraphicsItem
 {
    Q_OBJECT
 public:
     WireGui(MachineGui *sourceMacGui, MachineGui *destMacGui, MachineView *macView);
     ~WireGui();

     MachineGui *sourceMacGui() const;
     void setSourceMacGui(MachineGui *macGui);

     MachineGui *destMacGui() const;
     void setDestMacGui(MachineGui *macGui);

     void adjust();

signals:
   void deleteConnectionRequest( WireGui *wireGui );
   void startRewiringDest( WireGui *wireGui, QGraphicsSceneMouseEvent *event );

public slots:
   void deleteConnectionRequest();
   void onRewireDestActionTriggered();

protected:
    QRectF boundingRect() const;
    void paint( QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget );
    void contextMenuEvent( QGraphicsSceneContextMenuEvent *event );
    void mousePressEvent( QGraphicsSceneMouseEvent *event );
    void mouseMoveEvent( QGraphicsSceneMouseEvent *event );
    void mouseReleaseEvent( QGraphicsSceneMouseEvent *event );

 private:
     MachineView *machineView;

     MachineGui *source, *dest;

     QPointF sourcePoint;
     QPointF destPoint;
     qreal arrowSize;

     QAction *delConnAct_;
     QAction *rewireDstAct_;

     int state_;
 };

 #endif

