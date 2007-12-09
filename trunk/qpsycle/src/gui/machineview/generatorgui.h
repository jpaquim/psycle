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

#ifndef GENERATORGUI_H
#define GENERATORGUI_H

#include "machinegui.h"

namespace psy { namespace core {
		class Machine;
	}}


//class MachineView;
//class MachineTweakDlg;

class QGraphicsSceneContextMenuEvent;
class QKeyEvent;
class QGraphicsSceneMouseEvent;
class QPainter;
class QWidget;

class GeneratorGui : public MachineGui {
Q_OBJECT
public:
	GeneratorGui( int left, int top, psy::core::Machine *mac, MachineView *macView );
	~GeneratorGui();
	void paint( QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0 );

public slots:
	void showMacTweakDlg();

protected:
	void keyPressEvent( QKeyEvent * event );
	void keyReleaseEvent( QKeyEvent * event );
	void mousePressEvent( QGraphicsSceneMouseEvent *event );
	void mouseReleaseEvent( QGraphicsSceneMouseEvent *event );
	void mouseDoubleClickEvent( QGraphicsSceneMouseEvent *event );


	MachineTweakDlg *m_macTweakDlg;
};

#endif
