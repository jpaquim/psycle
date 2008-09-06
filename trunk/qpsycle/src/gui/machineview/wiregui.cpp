// -*- mode:c++; indent-tabs-mode:t -*-
/**************************************************************************
*   Copyright (C) 2007 by Psycledelics Community                          *
*   psycle.sourceforge.net                                                *
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
#include <psycle/core/player.h>

#include "wiregui.hpp"
#include "wiredlg.hpp"
#include "machinegui.hpp"
#include "machineview.hpp"

#include <cmath>

#include <QPainter>
#include <QGraphicsSceneContextMenuEvent>
#include <QMenu>
#include <QAction>

namespace qpsycle {

const double Pi = 3.14159265358979323846264338327950288419717;
const double TwoPi = 2.0 * Pi;

WireGui::WireGui(MachineGui *sourceMacGui, MachineGui *destMacGui, MachineView *macView)
	: machineView(macView)
	, arrowSize(20)
	, wiredlg(0)
	, state_(rewire_none)
{
	source = sourceMacGui;
	dest = destMacGui;
	source->addWireGui(this);
	if ( dest ) dest->addWireGui(this);
	adjust();

	float newvol;
	psy::core::Wire::id_type wire_id;
	wire_id = dest->mac()->FindInputWire( source->mac()->id() );
	dest->mac()->GetWireVolume( wire_id, newvol );
	onVolumeChanged(newvol);

	delConnAct_ = new QAction( "Delete Connection", this );

	connect(delConnAct_, SIGNAL(triggered()), this, SLOT(deleteConnectionRequest()));
	connect(this, SIGNAL(deleteConnectionRequest( WireGui * )), machineView, SLOT(deleteConnection( WireGui * ) ) );
	///\todo the above lines seem not the best way of doing things.
	// (i.e. should delete signal go direct to the machineView? )
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
	if(wiredlg)
		delete wiredlg;
}

MachineGui *WireGui::sourceMacGui() const
{
	return source;
}

void WireGui::setSourceMacGui(MachineGui *macGui)
{
	source = macGui;
	adjust();
}

MachineGui *WireGui::destMacGui() const
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
			state_ = rewire_dest;
		else if ( event->modifiers() & Qt::ControlModifier )
			state_ = rewire_src;
	}
}

void WireGui::mouseMoveEvent( QGraphicsSceneMouseEvent *event )
{
	if ( event->buttons() == Qt::LeftButton )
	{
		if ( state_ == rewire_dest ) {
			scene()->update( boundingRect() );
			destPoint = event->lastScenePos();    
			scene()->update( boundingRect() );
		}
		if ( state_ == rewire_src ) {
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
		if ( state_ == rewire_dest && !rewireDest( hitMacGui ) )
			destPoint = mapFromItem( dest, dest->boundingRect().width()/2, dest->boundingRect().height()/2 );
		else if ( state_ == rewire_src && !rewireSource( hitMacGui ) )
			sourcePoint = mapFromItem( source, source->boundingRect().width()/2, source->boundingRect().height()/2 ); 
	}
	state_ = rewire_none;
	adjust();

	scene()->update( scene()->itemsBoundingRect() );
}

void WireGui::mouseDoubleClickEvent( QGraphicsSceneMouseEvent *event )
{
	if ( event->buttons() != Qt::LeftButton )
		return;

	if ( !wiredlg )
	{
		wiredlg = new WireDlg(this, machineView, event->screenPos() );
	}
	wiredlg->show();
	wiredlg->raise();
	wiredlg->activateWindow();
}

bool WireGui::rewireDest( MachineGui *newDestGui )
{
	if ( !newDestGui->mac()->acceptsConnections() )
		return false;

	MachineGui *oldDestGui = destMacGui();

	bool exists(false);
	std::vector<WireGui*>::iterator it = newDestGui->wireGuiList_.begin();
	for ( ; it != newDestGui->wireGuiList_.end(); ++it ) {
		if (
			(*it)->sourceMacGui() == this->sourceMacGui() || //duplicate
			(*it)->destMacGui() == this->sourceMacGui() //feedback
		) {
			exists=true;
			break;
		}
	}
	if (exists) return false;

	// Update GUI connection.
	oldDestGui->wireGuiList_.erase (
		std::remove( oldDestGui->wireGuiList_.begin(), oldDestGui->wireGuiList_.end(), this ),
		oldDestGui->wireGuiList_.end()
	);

	dest = newDestGui; 
	dest->addWireGui(this);

	// Update song connection.
	psy::core::Machine *srcMac = sourceMacGui()->mac();
	psy::core::Machine *newDstMac = newDestGui->mac();
	psy::core::Player::singleton().lock();
	int oldDstWireIndex = srcMac->FindOutputWire( oldDestGui->mac()->id() );
	machineView->song()->ChangeWireDestMac( *srcMac, *newDstMac, 0,oldDstWireIndex,0 );
	psy::core::Player::singleton().unlock();

	if(wiredlg) {
		wiredlg->wireChanged();
	}
	return true;
}

bool WireGui::rewireSource( MachineGui *newSrcGui )
{
	if ( !newSrcGui->mac()->emitsConnections() )
		return false;

	MachineGui *oldSrcGui = sourceMacGui();

	bool exists(false);
	std::vector<WireGui*>::iterator it = newSrcGui->wireGuiList_.begin();
	for ( ; it != newSrcGui->wireGuiList_.end(); ++it ) {
		if (
			(*it)->destMacGui() == this->destMacGui() || //duplicate
			(*it)->sourceMacGui() == this->destMacGui() //feedback
		) {
			exists=true;
			break;
		}
	}
	if (exists) return false;

	// Update GUI connection.
	oldSrcGui->wireGuiList_.erase (
		std::remove( oldSrcGui->wireGuiList_.begin(), oldSrcGui->wireGuiList_.end(), this ),
		oldSrcGui->wireGuiList_.end()
	);
		
	source = newSrcGui; 
	source->addWireGui(this);

	// Update song connection.
	psy::core::Machine *newSrcMac = newSrcGui->mac();
	psy::core::Machine *dstMac = destMacGui()->mac();
	psy::core::Player::singleton().lock();
	int oldSrcWireIndex = dstMac->FindInputWire( oldSrcGui->mac()->id() );
	machineView->song()->ChangeWireSourceMac( *newSrcMac, *dstMac, 0, oldSrcWireIndex,0 );
	psy::core::Player::singleton().unlock();

	if(wiredlg) wiredlg->wireChanged();

	return true;
}

void WireGui::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
	event->accept();
	QMenu menu;
	menu.addAction( delConnAct_ );
	menu.addSeparator();
	menu.exec( event->screenPos() );
}

void WireGui::adjust()
{
	if (!source || !dest)
		return;

	QLineF line(mapFromItem(source, source->boundingRect().width()/2, source->boundingRect().height()/2), 
				mapFromItem(dest, dest->boundingRect().width()/2, dest->boundingRect().height()/2)); 
	qreal length = ( line.length() != 0? line.length() : .001 );
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

	painter->setRenderHint(QPainter::Antialiasing);

	// Draw the line.
	QLineF line(sourcePoint, destPoint);
	painter->setPen(QPen(Qt::white, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
	painter->drawLine(line);

	// Draw the arrow.
	double angle = ( line.length() != 0 ? ::acos(line.dx() / line.length()) : 0 );
	if (line.dy() >= 0)
		angle = TwoPi - angle;

	QPointF midPoint = sourcePoint + QPointF((destPoint.x()-sourcePoint.x())/2 + cos(angle) * arrowSize/2, 
								(destPoint.y()-sourcePoint.y())/2 - sin(angle) * arrowSize/2);
	QPointF arrowP1 = midPoint + QPointF(sin(angle - Pi / 3) * arrowSize,
											cos(angle - Pi / 3) * arrowSize);
	QPointF arrowP2 = midPoint + QPointF(sin(angle - Pi + Pi / 3) * arrowSize,
											cos(angle - Pi + Pi / 3) * arrowSize);

	painter->setBrush(arrowColor);
	painter->drawPolygon(QPolygonF() << midPoint << arrowP1 << arrowP2);
}

void WireGui::onVolumeChanged(float newval)
{
	//16256.25 is 255^2/4.  this assumes the maximum amplification we'll see is 4x.
	int brightness = (int)sqrt(newval*16256.25);
	if( brightness > 255) brightness = 255;
	else if( brightness < 0) brightness = 0;

	arrowColor.setRgb(brightness, brightness, brightness);
	update();
}

QPainterPath WireGui::shape () const
{
	QPainterPath path;
	// Making a rectangle with width = arrowSize, height = wire length.
	qreal halfPolygonWidth = arrowSize/2;
	QLineF line(sourcePoint, destPoint);
	double angle = ( line.dx() != 0 ? ::atan( line.dy() / line.dx() ) : Pi/2 );

	QPointF foo(
		halfPolygonWidth * std::cos(angle + Pi/2),
		halfPolygonWidth * std::sin(angle + Pi/2)
	);
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

} // namespace qpsycle
