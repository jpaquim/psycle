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

#ifndef MASTERGUI_H
#define MASTERGUI_H

#include "machinegui.h"

class MasterGui : public MachineGui
{
public:
	MasterGui(int left, int top, psy::core::Machine *mac, MachineView *macView);

protected:
	// Override these, as MasterGui doesn't do much (for now.)
	void mouseDoubleClickEvent ( QGraphicsSceneMouseEvent * event ) {}
	void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) {}
	void mouseMoveEvent(QGraphicsSceneMouseEvent *event) { QGraphicsItem::mouseMoveEvent( event ); }
	void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) { QGraphicsItem::mouseReleaseEvent( event ); }

};

#endif
