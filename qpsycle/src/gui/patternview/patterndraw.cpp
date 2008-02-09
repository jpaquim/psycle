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

#include "patternview.hpp"
#include "patterndraw.hpp"
#include "patterngrid.hpp"
#include "trackheader.hpp"
#include "linenumbercolumn.hpp"

#include <QDebug>
#include <QEvent>
#include <QKeyEvent>

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
		
	setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOn ); // FIXME: set to always on as AsNeeded has a bug in 4.2
	//setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff ); // Will be fixed in 4.3.
	// see: http://www.trolltech.com/developer/task-tracker/index_html?method=entry&id=152477
	//Mattias: is there a reason why scrollbard were off???

	trackHeaderHeight_ = 20;
	lineNumColWidth_ = 50;

	setViewportMargins( lineNumColWidth_, trackHeaderHeight_, 0, 0);

	patGrid_ = new PatternGrid( this );
	lineNumCol_ = new LineNumberColumn( this );
	lineNumCol_->setGeometry( 0, trackHeaderHeight_+2, lineNumColWidth_, height() );
	trackHeader_ = new TrackHeader( trackHeaderHeight_, this );

	//disabled. This is done by patternview, when setting numberOfTracks.
	//setupTrackGeometrics( patView_->numberOfTracks() );
	//alignTracks();
	
	scene_->addItem( patGrid_ );
	patGrid_->setPos( 0, 0 );
}

PatternDraw::~PatternDraw()
{
	patGrid_->patDraw(0);
	qWarning( "Delete PatternDraw: 0x%p.\n", this);
}


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
