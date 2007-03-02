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
 #ifndef MACHINEVIEW_H
 #define MACHINEVIEW_H

 #include <QWidget>
 #include <QtGui/QGraphicsView>
 #include <QtGui/QGraphicsScene>
 #include "newmachinedlg.h"
 #include "machinegui.h"

 class MachineGui;

 class MachineView : public QGraphicsView
 {
     Q_OBJECT

 public:
     MachineView();

     void connectMachines(MachineGui *srcMacGui, MachineGui *dstMacGui );

 protected:
    void keyPressEvent(QKeyEvent *event);
//    void mouseDoubleClickEvent(QMouseEvent *event);

    void scaleView(qreal scaleFactor);

 public slots:
    void newConnection(MachineGui *srcMacGui, QGraphicsSceneMouseEvent *event);
    void closeNewConnection(MachineGui *srcMacGui, QGraphicsSceneMouseEvent *event);

 private:
     NewMachineDlg *newMachineDlg;
     QGraphicsScene *scene_;
     QGraphicsLineItem *tempLine_;
 };

 #endif
