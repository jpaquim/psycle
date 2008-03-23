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
#include <qpsyclePch.hpp>

#include <psycle/core/song.h>
#include <psycle/core/player.h>

#include "patternview.hpp"
#include "patterndraw.hpp"
#include "patterngrid.hpp"
#include "trackheader.hpp"
#include "linenumbercolumn.hpp"

#include <QDebug>
#include <QEvent>
#include <QKeyEvent>
#include <QVarLengthArray>

namespace qpsycle {

/**
 * PatternDraw.  This is where the drawing of the pattern
 * is done, including the header, the grid, the line number
 * column, etc.
 */
	PatternDraw::PatternDraw( PatternView *patView )
	{
		qWarning( "Created PatternDraw: 0x%p.\n", this);

		patView_ = patView; 
		setAlignment( Qt::AlignLeft | Qt::AlignTop );
		scene_ = new QGraphicsScene(this);
		scene_->setBackgroundBrush( QColor( 30, 30, 30 ) );
		setScene(scene_);
		
		const int trackHeaderHeight_ = TrackHeader::height();
		const int lineNumColWidth_ = LineNumberColumn::width();

		setViewportMargins( lineNumColWidth_, trackHeaderHeight_, 0, 0);

		patGrid_ = new PatternGrid( this );

		lineNumCol_ = new LineNumberColumn( this );
		lineNumCol_->setGeometry( 0, trackHeaderHeight_+2, lineNumColWidth_, height() );

		trackHeader_ = new TrackHeader( this );
		trackHeader_->setGeometry( lineNumColWidth()+2, 0, width(), trackHeaderHeight_ );

		//disabled. This is now called from patternview constructor, via a call to numberOfTracks.
		//setupTrackGeometrics( patView_->numberOfTracks() );
		//alignTracks();

		scene_->addItem( patGrid_ );
		///LineNumCol and TrackHeader are not part of the scene, because they have not to be scrolled by both, x and y.
		patGrid_->setPos( 0, 0 );
	}

	PatternDraw::~PatternDraw()
	{
		///\todo: why so?
		patGrid_->patDraw(0);
		qWarning( "Delete PatternDraw: 0x%p.\n", this);
	}

//FIXME: Several comments here: First, if we are going to modify all values, why not empty the map?
// next, since alignTracks is executed after setupTrackGemetrics, wouldn't it make sense to be the same function?
	void PatternDraw::setupTrackGeometrics( int numberOfTracks, int visibleColumns ) 
	{
		for ( int newTrack = 0; newTrack < numberOfTracks; newTrack++ ) {
			TrackGeometry trackGeometry( *this );
			trackGeometry.setVisibleColumns( visibleColumns );
			trackGeometryMap[ newTrack ] = trackGeometry;
		}

		std::map<int, TrackGeometry>::iterator it;
		it = trackGeometryMap.lower_bound( numberOfTracks );
		while ( it != trackGeometryMap.end() ) {
			trackGeometryMap.erase( it++ );
		}
		alignTracks();
	}

	void PatternDraw::alignTracks() 
	{
		std::map<int, TrackGeometry>::iterator it = trackGeometryMap.begin();
		int offset = 0;
		for ( ; it != trackGeometryMap.end(); it++ ) {
			TrackGeometry & geometry = it->second;
			geometry.setLeft( offset );
			offset+= std::max( 50, geometry.width() ); // 50 is track min width
		}
	}

	const std::map<int, TrackGeometry> & PatternDraw::trackGeometrics() const {
		return trackGeometryMap;
	}

	int PatternDraw::rowHeight( ) const
	{
		return 13;
	}

	int PatternDraw::trackWidth() const
	{
		return 130;
	}

/** 
 * Get the width of the grid up until and including the given track.
 */
	int PatternDraw::gridWidthByTrack( int track ) const 
	{
		std::map<int, TrackGeometry>::const_iterator it;
		it = trackGeometrics().lower_bound( track );
		int gridWidth = 0;
		if ( it != trackGeometrics().end() ) {
			TrackGeometry trackGeom = it->second;
			gridWidth = trackGeom.left() + trackGeom.width();
		}
		return gridWidth;
	}

	int PatternDraw::findTrackByXPos( int x ) const  // FIXME: change to findTrackNumberByXPos
	{
		// todo write a binary search here
		// is used from intersectCell
		std::map<int, TrackGeometry>::const_iterator it = trackGeometrics().begin();
		int offset = 0;
		for ( ; it != trackGeometrics().end(); it++ ) {
			const TrackGeometry & geometry = it->second;
			offset+= geometry.width();
			if ( offset > x ) return it->first;
		}
		return -1; // no track found
	}

	int PatternDraw::xOffByTrack( int track ) const 
	{
		std::map<int, TrackGeometry>::const_iterator it;
		it = trackGeometrics().lower_bound( track );
		int trackOff = 0;
		if ( it != trackGeometrics().end() )
			trackOff = it->second.left();
		return trackOff;
	}

	int PatternDraw::xEndByTrack( int track ) const {
		std::map<int, TrackGeometry>::const_iterator it;
		it = trackGeometrics().lower_bound( track );
		int trackEnd = 0;
		if ( it != trackGeometrics().end() )
			trackEnd = it->second.left() + it->second.width();
		return trackEnd;
	}

	int PatternDraw::trackWidthByTrack( int track ) const 
	{
		std::map<int, TrackGeometry>::const_iterator it;
		it = trackGeometrics().lower_bound( track );
		int trackWidth = 0;
		if ( it != trackGeometrics().end() )
			trackWidth = it->second.width();
		return trackWidth;
	}

	TrackGeometry PatternDraw::findTrackGeomByTrackNum( int trackNum )
	{ // FIXME: just use a std::map operation?
		std::map<int, TrackGeometry>::const_iterator it;
		it = trackGeometrics().lower_bound( trackNum );
		if ( it != trackGeometrics().end() ) {
			return it->second;
		}
		return TrackGeometry( *this );
	}

	int PatternDraw::lineNumColWidth() const
	{
		return lineNumCol_->width();
	}
	int PatternDraw::trackHeaderHeight() const
	{
		return trackHeader_->height();
	}

	void PatternDraw::scrollContentsBy ( int dx, int dy ) 
	{
		lineNumCol_->update();
		trackHeader_->update();
		QGraphicsView::scrollContentsBy( dx, dy );
	}

	bool PatternDraw::event( QEvent *event )
	{
		switch (event->type()) {
		case QEvent::KeyPress: {
			QKeyEvent *k = (QKeyEvent *)event;
			QGraphicsView::keyPressEvent(k);
			if (k->key() == Qt::Key_Backtab
			    || (k->key() == Qt::Key_Tab && (k->modifiers() & Qt::ShiftModifier))
			    || (k->key() == Qt::Key_Tab) ) {
				event->accept();
			}
			return true;
		}
		default:
			return QGraphicsView::event( event );
		}
	}

	// Draws the grid lines of the pattern grid.
	void PatternDraw::drawBackground( QPainter * painter, const QRectF & rect )  
	{
		psy::core::TimeSignature signature;

		///\ todo: colours shouldn't be hardcoded.
		QColor backgroundColor = Qt::black;
		QColor separatorColor = QColor( 145, 147, 147 );
		QColor barColor = QColor( 70, 71, 69 );
		QColor beatColor = QColor( 50, 51, 49 );


		
		int sceneInvalidatedXStart = rect.left();
		int startTrackNum = findTrackByXPos( sceneInvalidatedXStart );
		int sceneInvalidatedXEnd = rect.right();
		int endTrackNum = findTrackByXPos( sceneInvalidatedXEnd );
		if ( endTrackNum == -1 )  endTrackNum = patternView()->numberOfTracks();

		int sceneInvalidatedYStart = rect.top();
		int startLineNum = static_cast<int>( sceneInvalidatedYStart / rowHeight() );
		int sceneInvalidatedYEnd = rect.bottom();
		int endLineNum = static_cast<int>( sceneInvalidatedYEnd / rowHeight() );
		if ( endLineNum > patternView()->numberOfLines()-1 )
			endLineNum = patternView()->numberOfLines()-1;

		int invalidatedWidth = static_cast<int>( rect.width() );
		int invalidatedHeight = static_cast<int>( rect.height() );

		// Draw horizontal lines to demarcate the pattern lines.
		bool lineGridEnabled = false; ///\todo make this an option somewhere.
		if ( lineGridEnabled )
		{
			QVarLengthArray<QLineF, 100> horizontalLines;
			for ( int line = startLineNum; line <= endLineNum; line++ ) {
				horizontalLines.append( QLineF ( sceneInvalidatedXStart, line * rowHeight(), invalidatedWidth, line * rowHeight() ) );
			}
			painter->setPen( separatorColor );
			painter->drawLines( horizontalLines.data(), horizontalLines.size() );
		}

		QVector<QRectF> barRects;
		QVector<QRectF> beatRects;
		QVector<QRectF> trackSepRects;

		// Construct barRects and beatRects.
		for (int line = startLineNum; line <= endLineNum; line++ ) 
		{
			float position = line / (float) patternView()->beatZoom();
			if (!(line == patternView()->playPos() ) || !psy::core::Player::Instance()->playing() ) 
			{
				if ( !(line % patternView()->beatZoom() ) ) 
				{
					if ( patternView()->pattern()->barStart(position, signature) ) {
						barRects.append( QRectF( sceneInvalidatedXStart, line*rowHeight(), invalidatedWidth, rowHeight() ) );
					} else {
						beatRects.append( QRectF( sceneInvalidatedXStart, line * rowHeight(), invalidatedWidth, rowHeight() ) );
					}
				}
			}
		}

		// Construct the vertical track separator rects.
		std::map<int, TrackGeometry>::const_iterator it;
		it = trackGeometrics().lower_bound( startTrackNum );
		for ( ; it != trackGeometrics().end() && it->first <= endTrackNum; it++ ) { 
			trackSepRects.append( QRectF( it->second.left(), sceneInvalidatedYStart, 5, invalidatedHeight ) );
		}

		// Paint all the rects.
		painter->fillRect( rect, backgroundColor );

		painter->setPen( Qt::black );
		painter->setBrush( beatColor );
		painter->drawRects( beatRects );
		painter->setBrush( barColor );
		painter->drawRects( barRects );

		painter->setPen( separatorColor );
		painter->setBrush( separatorColor );
		painter->drawRects( trackSepRects );
	}




//
//
// TrackGeometry
//

	TrackGeometry::TrackGeometry( ) :
		pDraw( 0 ),
		left_(0),
		width_(0),
		visibleColumns_(0),
		visible_(1)
	{ }

	TrackGeometry::TrackGeometry( PatternDraw & patternDraw ) :
		pDraw( &patternDraw ),
		left_(0),
		width_(0),
		visibleColumns_(0),
		visible_(1)
	{ }

	TrackGeometry::~TrackGeometry() { }

	void TrackGeometry::setLeft( int left ) {
		left_ = left;
	}

	int TrackGeometry::left() const {
		return left_;
	}

	int TrackGeometry::width() const {
		return pDraw->patternGrid()->visibleColWidth( visibleColumns() );
	}

	void TrackGeometry::setVisibleColumns( int cols ) {
		visibleColumns_= cols;
	}

	int TrackGeometry::visibleColumns() const {
		return visibleColumns_;
	}

	void TrackGeometry::setVisible( bool on) {
		visible_ = on;
	}

	bool TrackGeometry::visible() const {
		return visible_;
	}

} // namespace qpsycle
