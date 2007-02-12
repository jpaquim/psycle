/***************************************************************************
*   Copyright (C) 2006 by  Stefan   *
*   natti@linux   *
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

#include <QtGui>
#include <QMouseEvent>
#include <iostream>

 #include "machinegui.h"

 MachineGui::MachineGui(QWidget *parent) 
    : QWidget(parent)
 {
     setPalette(QPalette(QColor(100, 100, 100)));
     setAutoFillBackground(true);
     setMaximumSize(100,50);
 }

 void MachineGui::mouseMoveEvent(QMouseEvent *event)
 {
    QPoint pos = event->pos();
    setGeometry(pos.x(), parentWidget()->x()+pos.y(), 100, 50);
    std::cout << parentWidget()->x() << " " << parentWidget()->y() << std::endl;
    std::cout << pos.x() << " " << pos.y() << std::endl;
    update();
 }
