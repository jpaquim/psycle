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

#include "sequencerview.h"
#include "sequencerdraw.h"
#include "sequencerline.h"
#include "sequenceritem.h"

#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <boost/bind.hpp>
#include <QGraphicsItemGroup>

#include <iostream>

SequencerLine::SequencerLine( SequencerDraw *sDraw)
{
	sDraw_ = sDraw;
}

SequencerLine::~SequencerLine() {
	printf("~SequencerLine %p\n",this);
}

QRectF SequencerLine::boundingRect() const 
{
	int width = std::max( sDraw_->width(), (int)sDraw_->scene()->width() );
	return QRectF( 0, 0, width, sDraw_->lineHeight() );
}

void SequencerLine::paint( QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget )
{
	if ( sDraw_->selectedLine() == this ) {
		painter->setBrush( QBrush( QColor( 200, 200, 50, 100 ) ) );
	} else {
		painter->setBrush( QBrush( Qt::transparent ) );
	}
	painter->setPen( QPen( Qt::white, 1 ) );

	painter->drawRect( boundingRect() );
}

void SequencerLine::setSequenceLine( psy::core::SequenceLine * line )
{
	seqLine_ = line;
	// Iterate the sequence entries and add them.
	psy::core::SequenceLine::iterator iter = seqLine_->begin();
	for(; iter!= seqLine_->end(); ++iter) {
		psy::core::SequenceEntry* entry = iter->second;
		addEntry(entry);
	}
}

psy::core::SequenceLine *SequencerLine::sequenceLine() 
{
	return seqLine_;
}

void SequencerLine::addItem( psy::core::SinglePattern* pattern )
{
	printf("SequencerLine::addItem called\n");
	double endTick = sequenceLine()->tickLength();
	
	SequencerItem *item = new SequencerItem();
	psy::core::SequenceEntry* entry =
		sequenceLine()->createEntry(pattern, endTick);
	addEntry(entry);
}

void SequencerLine::addEntry( psy::core::SequenceEntry* entry )
{
	SequencerItem* item = new SequencerItem();
	item->setSequenceEntry( entry );
	item->setParentItem( this );
	items_.push_back( item );
	connect( item, SIGNAL( deleteRequest( SequencerItem* ) ), 
			sDraw_, SLOT( onSequencerItemDeleteRequest( SequencerItem* ) ) );
	connect( item, SIGNAL( clicked( SequencerItem*) ),
			this, SLOT( onItemClicked( SequencerItem*) ) );
	connect( item, SIGNAL( moved( SequencerItem*, QPointF ) ),
			sDraw_, SLOT( onItemMoved( SequencerItem*, QPointF ) ) );
	connect( item, SIGNAL( changedLine( SequencerItem*, int ) ),
			sDraw_, SLOT( onItemChangedLine( SequencerItem*, int ) ) );
	item->setPos( entry->tickPosition() * sDraw_->beatPxLength(), 0 );

	entry->wasDeleted.connect(boost::bind(&SequencerLine::removeEntry,this,_1));
	assert(scene());
//	scene()->addItem( item );
	scene()->update();
}

void SequencerLine::insertItem( SequencerItem *item )
{
	item->setParentItem( this );
	items_.push_back( item );
	connect( item, SIGNAL( clicked( SequencerItem*) ),
			this, SLOT( onItemClicked( SequencerItem*) ) );
	scene()->update();
}

// FIXME: design-wise, this may be better as SequencerItem::moveToNewLine.
void SequencerLine::moveItemToNewLine( SequencerItem *item, SequencerLine *newLine ) 
{
	printf( "prevline %p\n", this );
	printf( "newline %p\n", newLine );
	for( items_iterator i=items_.begin(); i!=items_.end(); ++i ) {
		assert(*i);
		if( (*i) == item ) {
			item->sequenceEntry()->setSequenceLine( newLine->sequenceLine() );
			newLine->insertItem( item );
			items_.erase(i);
			scene()->update();
			return;
		}
	}
}

void SequencerLine::removeEntry(psy::core::SequenceEntry* entry) {
	printf("SequencerLine(this=%p)::removeEntry(%p)",this,entry);
	for(items_iterator i=items_.begin();i!=items_.end();++i) {
		printf("item: %p\n",*i); // gives address 0x0000001 which is clearly wrong!
		assert(*i);
		if((*i)->sequenceEntry() == entry) {
			scene()->removeItem(*i);
			delete *i;
			// need to make line redraw itself?
			items_.erase(i);
			scene()->update();
			return;
		}
	}
}

void SequencerLine::onItemClicked(SequencerItem* item) {
	emit clicked ( this );
}

void SequencerLine::mousePressEvent( QGraphicsSceneMouseEvent *event )
{
	emit clicked( this );
}

