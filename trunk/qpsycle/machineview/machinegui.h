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
 #include <QAction>
 #include <QObject>
 #include <QPointF>

 #include "machineview.h"
 #include "wiregui.h"
 #include "machinetweakdlg.h"

 #include "psycore/machine.h"

 class MachineView;
 class WireGui;

 class MachineGui : public QObject, public QGraphicsRectItem
 {
    Q_OBJECT

 public:
     MachineGui(int left, int top, MachineView *macView);
     void addWireGui(WireGui *wireGui);
     void setName(const QString &name);
     QPointF centrePointInSceneCoords();
    enum { Type = UserType + 1 };

    int type() const { return Type; }

 protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant &value);
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent * event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

public slots:
    void showMacTwkDlg();

signals:
    void startNewConnection(MachineGui *macGui, QGraphicsSceneMouseEvent *event);
    void closeNewConnection(MachineGui *macGui, QGraphicsSceneMouseEvent *event);

 private:
     MachineView *machineView;
     QGraphicsTextItem *nameItem;
     int left_;
     int top_;
     QList<WireGui *> wireGuiList;
     MachineTweakDlg *macTwkDlg_;

     QAction *showMacTwkDlgAct_;

 };

 #endif
