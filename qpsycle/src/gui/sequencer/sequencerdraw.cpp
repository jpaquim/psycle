// -*- mode:c++; indent-tabs-mode:t -*-
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
#include <psycle/core/song.h>
#include <psycle/core/player.h>

#include "sequencerview.hpp"
#include "sequencerdraw.hpp"
#include "sequencerline.hpp"
#include "sequenceritem.hpp"
#include "sequencerarea.hpp"
#include "beatruler.hpp"

#include <iostream>
#include <vector>
#include <boost/bind.hpp>

#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QScrollBar>
#include <QGraphicsItem>
#include <QDebug>
#include <QVarLengthArray>

namespace qpsycle {

	SequencerDraw::SequencerDraw( SequencerView *seqView )
		: seqView_(seqView),
		beatPxLength_(6),
		lineHeight_(30)
	{
		setAlignment ( Qt::AlignLeft | Qt::AlignTop );

		QGraphicsScene *scene_ = new QGraphicsScene(this);
		setScene(scene_);
		scene_->setItemIndexMethod(QGraphicsScene::NoIndex);

		seqArea_ = new SequencerArea( this );
		scene_->addItem( seqArea_ );
		seqArea_->setPos( 0, 0 );

		psy::core::PatternSequence* patternSequence = sequencerView()->song()->patternSequence();

		std::vector<psy::core::SequenceLine*>::iterator it = patternSequence->begin();
		for ( ; it < patternSequence->end(); it++) {
			psy::core::SequenceLine* seqLine = *it;
			onNewLineCreated(seqLine);
		}

		if(!lines_.empty()) {
			setSelectedLine(lines_[0]);

			patternSequence->newLineCreated.connect
				(boost::bind(&SequencerDraw::onNewLineCreated,this,_1));
			patternSequence->newLineInserted.connect
				(boost::bind(&SequencerDraw::onNewLineInserted,this,_1,_2));
			/*
			Does not work because of a boost bug in boost 1.33
			as of Mar 28 2007.
			This bug is fixed in boost SVN. I put a workaround in
			makeSequencerLine instead.
			/ Magnus

			patternSequence->lineRemoved.connect
			(boost::bind(&SequencerDraw::onLineRemoved,this,_1));
			*/
			patternSequence->linesSwapped.connect
				(boost::bind(&SequencerDraw::onLinesSwapped,this,_1,_2));
		}

		pLine_ = new PlayLine( this );
		connect( pLine_, SIGNAL( playLineMoved( double ) ), this, SLOT( onPlayLineMoved( double ) ) );
		scene_->addItem( pLine_ );
		setViewportMargins( 0, 30, 0, 0 );
		beatRuler_ = new BeatRuler( this );
		beatRuler_->setGeometry( 2, 2, width()-2, 30 );
	}


	void SequencerDraw::addPattern( psy::core::SinglePattern *pattern )
	{
		if ( selectedLine() ) {
			selectedLine()->addItem( pattern );
		}
	}

	void SequencerDraw::insertTrack()
	{
		if ( lines_.size() == 0 ) {
			//    addSequencerLine();
		} else if ( selectedLine() ) {
			// will cause onNewLineInserted to be fired:
			seqView_->song()->patternSequence()->insertNewLine( selectedLine()->sequenceLine() );
		}
	}

	void SequencerDraw::deleteTrack() {
		if(selectedLine()) {
			// will trigger onLineRemoved
			seqView_->song()->patternSequence()->removeLine(selectedLine()->sequenceLine());
		}
	}

	void SequencerDraw::moveDownTrack() {
		if(selectedLine()) {
			// will trigger onLinesSwapped
			seqView_->song()->patternSequence()->moveDownLine(selectedLine()->sequenceLine());
		}
	}
	void SequencerDraw::moveUpTrack() {
		if(selectedLine()) {
			// will trigger onLinesSwapped
			seqView_->song()->patternSequence()->moveUpLine(selectedLine()->sequenceLine());
		}
	}

	void SequencerDraw::onCollapseButtonCliked()
	{
		selectedLine()->scale(1.0f, 0.5f);
	}

	void SequencerDraw::onExpandButtonCliked()
	{
		selectedLine()->scale(1.0f, 2.0f);
	}

	SequencerLine* SequencerDraw::makeSequencerLine( psy::core::SequenceLine* seqLine ) {
		SequencerLine* line = new SequencerLine( this );
		scene()->addItem(line);
		line->setSequenceLine(seqLine);
		line->setParentItem( seqArea_ );
		connect( line, SIGNAL( clicked( SequencerLine* ) ), this, SLOT( onSequencerLineClick( SequencerLine* ) ) );

		seqLine->wasDeleted.connect(boost::bind(&SequencerDraw::onLineRemoved,this,_1));
		return line;
	}

	void SequencerDraw::onSequencerLineClick( SequencerLine *line )
	{
		if ( selectedLine() ) {
			QRectF oldLineRect = selectedLine()->mapToScene( selectedLine()->boundingRect() ).boundingRect();
			scene()->update( oldLineRect );
		}
		setSelectedLine( line );
		QRectF newLineRect = line->mapToScene( line->boundingRect() ).boundingRect();
		scene()->update( newLineRect );
	}

	void SequencerDraw::onSequencerItemDeleteRequest( SequencerItem *item )
	{
		psy::core::SequenceEntry *entry = item->sequenceEntry();
		if ( psy::core::Player::Instance()->loopSequenceEntry() == entry ) {
			psy::core::Player::Instance()->setLoopSequenceEntry( 0 );
		}
		//entry->track()->removeEntry(entry); // Remove from the song's pattern sequence.
		//scene()->removeItem( item ); // Remove from the GUI. FIXME: think we need to delete the object itself here too.
	}

	void SequencerDraw::onNewLineCreated(psy::core::SequenceLine* seqLine)
	{
		SequencerLine* line = makeSequencerLine(seqLine);
		line->setPos( 0, lines_.size()*lineHeight_ );
		lines_.push_back(line);
		setSelectedLine( line );
	}

	void SequencerDraw::onNewLineInserted(psy::core::SequenceLine* seqLine, psy::core::SequenceLine* position)
	{
		qDebug("onNewLineInserted(%p,%p)\n",seqLine,position);
		SequencerLine* line = makeSequencerLine(seqLine);
		int numLines = lines_.size();
		for(int i=0;i<numLines;i++) {
			if (lines_[i]->sequenceLine() == position) {
				qDebug("%ith line\n",i);
				SequencerLine* l=line;
				for(int j=i;j<numLines;j++) {
					l->setPos( 0, j*lineHeight_ );
					std::swap(l,lines_[j]);
				}
				l->setPos( 0, numLines*lineHeight_ );
				lines_.push_back(l);
				break;
			}
		}

		setSelectedLine( line );
	}

	void SequencerDraw::onLineRemoved(psy::core::SequenceLine* seqLine)
	{
		if (selectedLine() && selectedLine()->sequenceLine() == seqLine)
			setSelectedLine( NULL );
		int numLines = lines_.size();
		for(int i=0;i<numLines;i++) {
			if (lines_[i]->sequenceLine() == seqLine) {
				for(int j=i;j<numLines-1;j++) {
					std::swap(lines_[j],lines_[j+1]);
					lines_[j]->setPos( 0, j*lineHeight_ );
				}
				delete lines_.back();
				lines_.pop_back();
				break;
			}
		}
	}

	void SequencerDraw::onLinesSwapped(psy::core::SequenceLine* a, psy::core::SequenceLine* b)
	{
		lines_iterator ita = lines_.end();
		lines_iterator itb = lines_.end();
		for(lines_iterator i=lines_.begin();i != lines_.end();i++) {
			psy::core::SequenceLine* sl = (*i)->sequenceLine();
			if (sl == a)
				ita = i;
			if (sl == b)
				itb = i;
		}
		if (ita != lines_.end() && itb != lines_.end()) {
			QPointF posa = (*ita)->pos();
			QPointF posb = (*itb)->pos();
			(*ita)->setPos(posb);
			(*itb)->setPos(posa);
			std::swap(*ita,*itb);
		}
	}

	void SequencerDraw::onPlayLineMoved( double newXPos )
	{
		psy::core::Player::Instance()->stop();
		psy::core::Player::Instance()->setPlayPos( newXPos / beatPxLength_ );
	}

	void SequencerDraw::onItemMoved( SequencerItem* item, QPointF diff ) 
	{
		Q_UNUSED( item );
		if ( gridSnap() ) {
			int beatDiff = (int)diff.x() / 5;
			int snappedBeatDiff = beatDiff * 5;
			diff.setX( snappedBeatDiff );
		}

		QList<QGraphicsItem *> selectedItems = scene()->selectedItems();

		int leftMostX = 10000; // Of all selected items, find the left most x pos.
		foreach ( QGraphicsItem *uncastItem, selectedItems )
		{
			if ( SequencerItem *someItem = qgraphicsitem_cast<SequencerItem *>( uncastItem ) ) 
			{
				if ( someItem->pos().x() < leftMostX ) leftMostX = (int)someItem->pos().x();
			}
		}

		int xMoveBy;
		if ( leftMostX + diff.x() < 0 ) {
			xMoveBy = -leftMostX;
		} else {
			xMoveBy = (int)diff.x();
		}

		foreach ( QGraphicsItem *uncastItem, selectedItems )
		{
			if ( SequencerItem *someItem = qgraphicsitem_cast<SequencerItem *>( uncastItem ) ) 
			{
				someItem->moveBy( xMoveBy, 0 );
				int newItemLeft = someItem->pos().x();
				someItem->sequenceEntry()->track()->MoveEntry( someItem->sequenceEntry(), newItemLeft / beatPxLength() );
			}
		}
	}

	void SequencerDraw::onItemChangedLine( SequencerItem *item, int direction )
	{
		Q_UNUSED( item );
		// Note: for "direction", 0 = up, 1 = down.
		QList<QGraphicsItem *> selectedItems = scene()->selectedItems();

		// Check that the item (or group) doesn't touch the top or bottom.
		bool allowMove = true;
		foreach ( QGraphicsItem *uncastItem, selectedItems )
		{
			if ( SequencerItem *someItem = qgraphicsitem_cast<SequencerItem *>( uncastItem ) ) 
			{
				SequencerLine *parentLine = qgraphicsitem_cast<SequencerLine *>( someItem->parentItem() );  
				if ( lines_[0] == parentLine ) {
					if (direction == 0 ) allowMove = false;
				}
				if ( lines_[lines_.size()-1] == parentLine ) {
					if (direction == 1) allowMove = false;
				}
			}
		}

		if ( allowMove )
		{
			foreach ( QGraphicsItem *uncastItem, selectedItems )
			{
				if ( SequencerItem *someItem = qgraphicsitem_cast<SequencerItem *>( uncastItem ) ) 
				{
					SequencerLine *parentLine = qgraphicsitem_cast<SequencerLine *>( someItem->parentItem() );  
					int parentPos;
					for ( unsigned int i = 0; i < lines_.size(); i++ ) {
						if ( lines_[i] == parentLine ) {
							parentPos = i;
						}
					}
					
					if ( direction == 0 ) {
						parentLine->moveItemToNewLine( someItem, lines_[parentPos-1] );
					} else if ( direction == 1 ) {
						parentLine->moveItemToNewLine( someItem, lines_[parentPos+1] );
					}
				}
			}
		}
	}


	bool SequencerDraw::gridSnap() const 
	{
		return true; ///\todo this should be a user option (so not hardcoded.)
	}

	int SequencerDraw::beatPxLength( ) const
	{
		return beatPxLength_;
	}

	void SequencerDraw::setSelectedLine( SequencerLine *line ) 
	{
		qDebug("setSelectedLine %p\n", line);
		selectedLine_ = line;
	}

	SequencerLine* SequencerDraw::selectedLine() const
	{
		return selectedLine_;
	}

	std::vector<SequencerLine*> SequencerDraw::lines() const
	{
		return lines_;
	}

	void SequencerDraw::drawBackground( QPainter * painter, const QRectF & rect ) 
	{
		// "rect" provides the dimensions of the part
		// of the background that needs drawing.

		// Work out where to start drawing the lines from.
		qreal left = int(rect.left()) - ( int(rect.left()) % beatPxLength_ );

		QVarLengthArray<QLineF, 100> lines;

		for ( qreal x = left; x < rect.right(); x += beatPxLength_ ) {
			lines.append( QLineF( x, rect.top(), x, rect.bottom() ) );
		}

		painter->fillRect( rect, Qt::black );
		painter->setPen( QColor( 30, 30, 30 ) );

		painter->drawLines( lines.data(), lines.size() );
	}

	void SequencerDraw::scrollContentsBy ( int dx, int dy ) 
	{
		beatRuler_->update();
		QGraphicsView::scrollContentsBy( dx, dy );
	}

	void SequencerDraw::setBeatPxLength( int beatPxLength )
	{
		beatPxLength_ = beatPxLength;

		// Update the position in the scene of each sequencer item based on
		// the new beatpxlength.
		for ( lines_iterator it = lines_.begin(); it != lines_.end(); it++ ) 
		{
			QList<QGraphicsItem*> children = (*it)->children();
			foreach ( QGraphicsItem *child, children )
			{
				SequencerItem *seqItem = qgraphicsitem_cast<SequencerItem*>( child );
				psy::core::SequenceEntry *entry = seqItem->sequenceEntry();
				seqItem->setPos( entry->tickPosition() * beatPxLength_, 0 );
			}
		}
	}





	PlayLine::PlayLine( QGraphicsView *seqDraw )
	{ 
		m_seqDraw = seqDraw;
		setFlags( ItemIsMovable | ItemIsFocusable );
		setCursor( Qt::SizeHorCursor );
		setZValue( 100 );
	}

	void PlayLine::mouseMoveEvent( QGraphicsSceneMouseEvent *event )
	{
		if ( event->buttons() & Qt::LeftButton )
		{
			QPointF newPos(mapToParent(event->pos()) - matrix().map(event->buttonDownPos(Qt::LeftButton)));
			setPos( std::max( 0, (int)newPos.x() ), 0 ); // x can't be less than 0; y is always 0.
		} else {
			event->ignore();
		}
	}

	void PlayLine::mouseReleaseEvent( QGraphicsSceneMouseEvent *event )
	{
		QGraphicsItem::mouseReleaseEvent( event );
		emit playLineMoved( pos().x() );
	}

	QRectF PlayLine::boundingRect() const
	{
		return QRectF( 0, m_seqDraw->verticalScrollBar()->value(), 1, m_seqDraw->height() );

	}

	void PlayLine::paint ( QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget ) 
	{
		Q_UNUSED( option ); Q_UNUSED( widget );
		painter->fillRect( boundingRect(), Qt::red );
	}

} // namespace qpsycle
