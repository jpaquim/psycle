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

#include <QGraphicsItem>
#include <QAction>
#include <QColor>

namespace qpsycle {

class MachineGui;
class MachineView;
class WireDlg;

enum RewireType {
	rewire_none,
	rewire_dest,
	rewire_src
};

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

	bool rewireDest( MachineGui *newDstGui );
	bool rewireSource( MachineGui *newSrcGui );

	void adjust();

Q_SIGNALS:
	void deleteConnectionRequest( WireGui *wireGui );

public Q_SLOTS:
	void deleteConnectionRequest();
	void onVolumeChanged(float newval);

protected:
	QRectF boundingRect() const;
	QPainterPath shape () const;
	void paint( QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget );
	void contextMenuEvent( QGraphicsSceneContextMenuEvent *event );
	void mousePressEvent( QGraphicsSceneMouseEvent *event );
	void mouseMoveEvent( QGraphicsSceneMouseEvent *event );
	void mouseReleaseEvent( QGraphicsSceneMouseEvent *event );
	void mouseDoubleClickEvent( QGraphicsSceneMouseEvent *event );

private:
	MachineView *machineView;

	MachineGui *source, *dest;

	QPointF sourcePoint;
	QPointF destPoint;
	qreal arrowSize;

	QAction *delConnAct_;

	QColor arrowColor;

	WireDlg *wiredlg;

	RewireType state_;
};

} // namespace qpsycle

#endif

