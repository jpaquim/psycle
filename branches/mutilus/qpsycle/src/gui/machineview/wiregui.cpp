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
#include <qpsyclePch.hpp>

#include <psycle/core/player.h>

#include "wiregui.h"
#include "machinegui.h"
#include "machineview.h"

#include <math.h>

#include <QPainter>
#include <QGraphicsSceneContextMenuEvent>
#include <QMenu>
#include <QAction>

	static const double Pi = 3.14159265358979323846264338327950288419717;
	static double TwoPi = 2.0 * Pi;

	WireGui::WireGui(MachineGui *sourceMacGui, MachineGui *destMacGui, MachineView *macView)
		: arrowSize(20), machineView(macView)
	{
		source = sourceMacGui;
		dest = destMacGui;
		source->addWireGui(this);
		if ( dest ) dest->addWireGui(this);
		adjust();

		delConnAct_ = new QAction( "Delete Connection", this );
		rewireDstAct_ = new QAction( "Rewire Connection Destination", this );

		connect(delConnAct_, SIGNAL(triggered()), this, SLOT(deleteConnectionRequest()));
		connect(this, SIGNAL(deleteConnectionRequest( WireGui * )), machineView, SLOT(deleteConnection( WireGui * ) ) );
		// FIXME: the above lines seem not the best way of doing things.
		// (i.e. should delete signal go direct to the machineView? )
		
		connect( rewireDstAct_, SIGNAL( triggered() ), this, SLOT( onRewireDestActionTriggered() ) );

		setSourceMacGui( sourceMacGui );
		setDestMacGui( destMacGui );
	}

	WireGui::~WireGui()
	{
		#if 0
		sourceMacGui()->wireGuiList().removeAll( this );
		destMacGui()->wireGuiList().removeAll( this );
		QList<WireGui*>::iterator it = sourceMacGui()->wireGuiList().begin();
		for ( ; it != sourceMacGui()->wireGuiList().end(); it++ ) {
			if ( *it == this ) {
				sourceMacGui()->wireGuiList().erase( it );
			}
		}
		it = destMacGui()->wireGuiList().begin();
		for ( ; it != destMacGui()->wireGuiList().end(); it++ ) {
			if ( *it == this ) {
				destMacGui()->wireGuiList().erase( it );
			}
		}
		#endif
	}

	MachineGui *WireGui::sourceMacGui() 
	{
		return source;
	}

	void WireGui::setSourceMacGui(MachineGui *macGui)
	{
		source = macGui;
		adjust();
	}

	MachineGui *WireGui::destMacGui() 
	{
		return dest;
	}

	void WireGui::setDestMacGui(MachineGui *macGui)
	{
		dest = macGui;
		adjust();
	}

void WireGui::mousePressEvent( QGraphicsSceneMouseEvent *event )
{
	if ( event->button() == Qt::LeftButton ) 
	{
		if ( event->modifiers() & Qt::ShiftModifier )
			state_ = 1;
		else if ( event->modifiers() & Qt::ControlModifier )
			state_ = 2;
	}
}

void WireGui::mouseMoveEvent( QGraphicsSceneMouseEvent *event )
{
	if ( event->buttons() == Qt::LeftButton )
	{
		if ( ( event->modifiers() & Qt::ShiftModifier ) && state_ == 1 ) {
			scene()->update( boundingRect() );
			destPoint = event->lastScenePos();    
			scene()->update( boundingRect() );
		}
		if ( ( event->modifiers() & Qt::ControlModifier ) && state_ == 2 ) {
			scene()->update( boundingRect() );
			sourcePoint = event->lastScenePos();    
			scene()->update( boundingRect() );
		}
	}
}

void WireGui::mouseReleaseEvent( QGraphicsSceneMouseEvent *event )
{
	if ( MachineGui* hitMacGui = machineView->machineGuiAtPoint( event->scenePos() ) ) 
	{
		if ( state_ == 1 ) { // rewire dest
			if ( hitMacGui->mac()->acceptsConnections() ) {
				rewireDest( hitMacGui );
			} else destPoint = mapFromItem( dest, dest->boundingRect().width()/2, dest->boundingRect().height()/2 ); 
		} else if ( state_ == 2 ) { // rewire src
			if ( hitMacGui->mac()->emitsConnections() ) {
				rewireSource( hitMacGui );
			} else sourcePoint = mapFromItem( source, source->boundingRect().width()/2, source->boundingRect().height()/2 ); 
		}
		state_ = 0;
	} else adjust();
	
	scene()->update( scene()->itemsBoundingRect() );
}

void WireGui::rewireDest( MachineGui *newDestGui )
{
	// Update GUI connection.
	MachineGui *oldDestGui = destMacGui();
	std::vector<WireGui*>::iterator it = oldDestGui->wireGuiList_.begin();
	for ( ; it != oldDestGui->wireGuiList_.end(); it++ ) {
		if ( *it == this ) {
			oldDestGui->wireGuiList_.erase( it );
			break;
		}
	}
	dest = newDestGui; 
	dest->addWireGui(this);
	// Update song connection.
	psy::core::Machine *srcMac = sourceMacGui()->mac();
	psy::core::Machine *newDstMac = newDestGui->mac();
	psy::core::Player::Instance()->lock();
	int oldDstWireIndex = srcMac->FindOutputWire( oldDestGui->mac()->id() );
	machineView->song()->ChangeWireDestMac( srcMac->id(), newDstMac->id(), oldDstWireIndex );
	psy::core::Player::Instance()->unlock();
}

void WireGui::rewireSource( MachineGui *newSrcGui )
{
	// Update GUI connection.
	MachineGui *oldSrcGui = sourceMacGui();
	std::vector<WireGui*>::iterator it = oldSrcGui->wireGuiList_.begin();
	for ( ; it != oldSrcGui->wireGuiList_.end(); it++ ) {
		if ( *it == this ) {
			oldSrcGui->wireGuiList_.erase( it );
			break;
		}
	}
	source = newSrcGui; 
	source->addWireGui(this);
	// Update song connection.
	psy::core::Machine *newSrcMac = newSrcGui->mac();
	psy::core::Machine *dstMac = destMacGui()->mac();
	psy::core::Player::Instance()->lock();
	int oldSrcWireIndex = dstMac->FindInputWire( oldSrcGui->mac()->id() );
	machineView->song()->ChangeWireSourceMac( newSrcMac->id(), dstMac->id(), oldSrcWireIndex );
	psy::core::Player::Instance()->unlock();
}

void WireGui::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
	event->accept();
	QMenu menu;
	menu.addAction( delConnAct_ );
	menu.addSeparator();
	QAction *a = menu.exec( event->screenPos() );
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

QPainterPath WireGui::shape () const
{
	QPainterPath path;
	// Making a rectangle with width = arrowSize, height = wire length.
	qreal halfPolygonWidth = arrowSize/2;
	qreal sideLength = sqrt((halfPolygonWidth*halfPolygonWidth)/2);
	QPointF foo( sideLength, sideLength );
	QPointF p0 = sourcePoint - foo;
	QPointF p1 = sourcePoint + foo;
	QPointF p2 = destPoint + foo;
	QPointF p3 = destPoint - foo;
	QPolygonF polygon;
	polygon << p0 << p1 << p2 << p3;
	path.addPolygon( polygon );
	path.closeSubpath();
	return path;
}

void WireGui::deleteConnectionRequest()
{
	emit deleteConnectionRequest(this);
}

void WireGui::onRewireDestActionTriggered()
{
	//emit startRewiringDest(this);
}
