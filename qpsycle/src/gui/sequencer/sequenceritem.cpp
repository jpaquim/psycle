// -*- mode:c++; indent-tabs-mode:t -*-
/***************************************************************************
*   Copyright (C) 2007 by Psycledelics Community   *
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

#include <psycle/core/patternsequence.h>
#include <psycle/core/player.h>

#include "sequencerview.hpp"
#include "sequencerdraw.hpp"
#include "sequencerline.hpp"
#include "sequenceritem.hpp"

#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QMenu>
#include <QKeyEvent>
#include <QGraphicsItemGroup>

namespace qpsycle {

SequencerItem::SequencerItem( SequencerDraw *seqDraw ) 
{
	setFlags( ItemIsMovable | ItemIsSelectable | ItemIsFocusable );
	seqDraw_ = seqDraw;
}

SequencerItem::~SequencerItem() {
}

QRectF SequencerItem::boundingRect() const
{
	return QRectF( 0, 0, sequenceEntry_->pattern()->beats()*seqDraw_->beatPxLength(), parentItem()->boundingRect().height() );
}

void SequencerItem::paint( QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget )
{
	if ( isSelected() ) {
		painter->setPen( Qt::blue ); 
	} else {
		painter->setPen( Qt::white ); 
	}
	QColor col = QColorFromLongColor( sequenceEntry_->pattern()->category()->color() );
	painter->setBrush(col);
	painter->drawRect( boundingRect() ); // FIXME: need to take border width into account.
	painter->setPen( Qt::white ); 
	painter->drawText( boundingRect(), Qt::AlignCenter, QString::fromStdString( sequenceEntry_->pattern()->name() ) );
}

const QColor & SequencerItem::QColorFromLongColor( long longCol )
{
	unsigned int r, g, b;
	b = (longCol>>16) & 0xff;
	g = (longCol>>8 ) & 0xff;
	r = (longCol    ) & 0xff;

	return QColor( r, g, b );
}

void SequencerItem::setSequenceEntry( psy::core::SequenceEntry *sequenceEntry )
{
	sequenceEntry_ = sequenceEntry;
}

psy::core::SequenceEntry *SequencerItem::sequenceEntry() {
	return sequenceEntry_;
}

void SequencerItem::mouseMoveEvent( QGraphicsSceneMouseEvent *event )
{
	if ( event->modifiers() == Qt::NoModifier ) // Movement constrained to current line.
	{
		QPointF newPos( mapToParent(event->pos()) - matrix().map( event->buttonDownPos(Qt::LeftButton) ) );
		QPointF diff = newPos - pos();
		emit moved( this, diff ); 
		// <nmather> MoveEvent handling put in SequencerDraw -- seems more sensible
		// as we may be moving groups of selected items (which this item shouldn't
		// really know about).
	}
	if ( event->modifiers() == Qt::ShiftModifier ) // Movement allowed between lines.
	{
		QPointF newPos( mapToParent(event->pos()) - matrix().map( event->buttonDownPos(Qt::LeftButton) ) );
		QPointF diff = newPos - pos();
		emit moved( this, diff ); 

		SequencerLine *parentLine = qgraphicsitem_cast<SequencerLine*>( parentItem() );
		SequencerLine *lineUnderCursor = 0;
		QList<QGraphicsItem*> itemsUnderCursor = scene()->items( event->scenePos() );
		for ( int i = 0; i < itemsUnderCursor.size(); ++i ) {
			if ( qgraphicsitem_cast<SequencerLine *>( itemsUnderCursor.at(i) ) ) { // Make sure it's a SeqLine.
				lineUnderCursor = qgraphicsitem_cast<SequencerLine*>( itemsUnderCursor.at(i) );
			}
		}
		if ( parentLine != lineUnderCursor && lineUnderCursor != 0 ) {
			if ( diff.y() < 0 ) 
				emit changedLine( this, 0 );
			if ( diff.y() > 0 ) 
				emit changedLine( this, 1 );
		}
	}
}

void SequencerItem::constrainToParent() 
{
	int widthOfThisItem = boundingRect().width();
	int widthOfParent = parentItem()->boundingRect().width();
	//int maximumLeftPos = widthOfParent - widthOfThisItem;
	int currentLeftPos = pos().x();
	//int desiredLeftPos = std::min( currentLeftPos, maximumLeftPos );
	int newLeftPos = std::max( 0, currentLeftPos );

	setPos( newLeftPos, 0 );                 
	
	if ( true /*gridSnap()*/ ) {
		int beatPos = pos().x() / seqDraw_->beatPxLength();
		int snappedLeftPos = beatPos * seqDraw_->beatPxLength();
		setPos( snappedLeftPos, 0 );
	}
}

void SequencerItem::mousePressEvent( QGraphicsSceneMouseEvent *event )
{
	QGraphicsItem::mousePressEvent( event ); // Do normal business...
	printf( "parentLine %p\n", qgraphicsitem_cast<SequencerLine*>(parentItem()) );
	printf( "tickPos %f\n", sequenceEntry()->tickPosition() );

	emit clicked(this);
}

void SequencerItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
	QMenu menu;
	if ( psy::core::Player::Instance()->loopSequenceEntry() == sequenceEntry() ) {
		loopEntryAction_ = new QAction( "Unloop Entry", this );
	} else {
		loopEntryAction_ = new QAction( "Loop Entry", this );
	}
	deleteEntryAction_ = new QAction( "Delete Entry", this );
	connect( loopEntryAction_, SIGNAL( triggered() ), this, SLOT( onLoopEntryActionTriggered() ) );
	connect( deleteEntryAction_, SIGNAL( triggered() ), this, SLOT( onDeleteEntryActionTriggered() ) );
	menu.addAction( loopEntryAction_ );
	menu.addAction( deleteEntryAction_ );
	QAction *a = menu.exec(event->screenPos());
}

void SequencerItem::onLoopEntryActionTriggered()
{
	if ( psy::core::Player::Instance()->loopSequenceEntry() == sequenceEntry() ) {
		psy::core::Player::Instance()->setLoopSequenceEntry( 0 );
	} else {
		psy::core::Player::Instance()->setLoopSequenceEntry( sequenceEntry() );
	}
}

void SequencerItem::onDeleteEntryActionTriggered()
{
	emit deleteRequest( this );
}

void SequencerItem::keyPressEvent( QKeyEvent *event )
{
	switch ( event->key() )
	{
		case Qt::Key_Left:
		{
		QPointF diff( -seqDraw_->beatPxLength(), 0 );
		emit moved( this, diff );
		}
		break;
		case Qt::Key_Right :
		{
		QPointF diff( seqDraw_->beatPxLength(), 0 );
		emit moved( this, diff );
		}
		break;
		case Qt::Key_Up:
		emit changedLine( this, 0 ); 
		break;
		case Qt::Key_Down:
		emit changedLine( this, 1 );
		break;
		default: event->ignore();
	}
}

QVariant SequencerItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
	if (change == ItemPositionChange)
		return QPointF(value.toPointF().x(), pos().y()); // Constrains an item to present y pos.
	return QGraphicsItem::itemChange(change, value);
}

} // namespace qpsycle
