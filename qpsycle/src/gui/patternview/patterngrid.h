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
#ifndef PATTERNGRID_H
#define PATTERNGRID_H

#include <psycle/core/singlepattern.h>

#include <map>

#include <QGraphicsItem>
class QKeyEvent;
#include <QFont>

class PatternView;
class PatternDraw;
class PatternGrid;
class TrackGeometry;
class BehaviourStrategy;

/**
	* Selection.
	*/
class Selection {
public:
	Selection() {}

	void set( int left, int right, int top, int bottom )
		{ left_ = left; right_ = right; top_ = top; bottom_ = bottom; }
	int left() { return left_; }
	int right() { return right_; }
	int top() { return top_; }
	int bottom() { return bottom_; }
	void setLeft( int left ) { left_ = left; }
	void setRight( int right ) { right_ = right; };
	void setTop( int top ) { top_ = top; }
	void setBottom( int bottom ){ bottom_ = bottom; }
	void clear() { left_=0; right_=0; top_=0; bottom_=0; }
	bool isEmpty() { return left_ || right_ || top_ || bottom_ ? false : true; }

private:
	int left_, right_, top_, bottom_;

};

/**
	* PatCursor.
	*/
class PatCursor {
public:
	PatCursor();
	PatCursor( int track, int line, int eventNr, int col );
	~PatCursor();

	void setPosition( int track, int line, int eventNr, int col );

	void setTrack( int x );
	int track() const;
	void setLine( int y );
	int line() const;
	void setEventNr( int event );
	int eventNr() const;
	void setCol( int col );
	int col() const;

private:

	int track_;
	int line_;
	int eventNr_;
	int col_;

};

/**
	* ColumnEvent.
	*/
class ColumnEvent {																																																
public:
	enum ColType { hex2 = 0, hex4 = 1, note = 2 };

	ColumnEvent( ColType type );

	~ColumnEvent();

	ColType type() const;
	int cols() const;

private:

	ColType type_;
};



/**
	* PatternGrid.
	*/
class PatternGrid : public QGraphicsItem {

public:
	PatternGrid( PatternDraw *pDraw );

	void addEvent( const ColumnEvent & event );
	psy::core::SinglePattern *pattern();
	enum SelDirection { nodir = 0, north = 1, west = 2, east = 4, south = 8};
	PatternDraw *patDraw() { return patDraw_; }


	// Painting.
	void paint( QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget );

	void drawGrid( QPainter *painter, int startLine, int endLine, int startTrack, int endTrack  );
	void drawPattern( QPainter *painter, int startLine, int endLine, int startTrack, int endTrack  );
	void drawData( QPainter *painter, int startLine, int endLine, int startTrack, int endTrack, bool sharp, const QColor & color );
	void drawSelBg( QPainter *painter, Selection selArea );
	void drawBlockData( QPainter *painter, int xOff, int line, const std::string & text, const QColor & color);
	void drawStringData( QPainter *painter, int xOff, int line, const std::string & text, const QColor & color );
	std::string noteToString( int value, bool sharp );
	void drawString( QPainter *painter, int track, int line, int eventnr, const std::string & data , const QColor & color );
	void drawCellBg( QPainter *painter, const PatCursor& cursor );
	QRectF repaintTrackArea(int startLine,int endLine,int startTrack, int endTrack) const;
	unsigned char convertDigit( int defaultValue, int scanCode, unsigned char oldByte, int col ) const;

	// Geometry.
	QRectF boundingRect() const;
	const std::map<int, TrackGeometry> & trackGeometrics() const;
	int lineHeight() const;
	bool isNote( int key );
	bool isHex( QKeyEvent *ev );
	int cellWidth( ) const;
	int eventOffset( int eventnr, int col ) const;
	int eventWidth( int eventnr ) const;
	int eventColWidth( int eventnr ) const;
	int noteCellWidth( ) const;
	void setBigTrackSeparatorWidth( int ident );
	int bigTrackSeparatorWidth() const;

	// Settings.
	bool lineGridEnabled() const;
	int visibleEvents( int track ) const;
	int numberOfTracks() const;
	int endTrackNumber() const;
	int numberOfLines() const;
	int endLineNumber() const;
	int beatZoom() const;
	int patternStep(); 
	int navStep();
	bool ignorePatStepForNav();
	bool ft2HomeEndBehaviour();
	void setFt2HomeEndBehaviour( bool setit );
	bool shiftArrowForSelect();
	void setShiftArrowForSelect( bool setit );
	bool wrapAround();
	void setWrapAround( bool setit );
	bool centerCursor();
	void setCenterCursor( bool setit );

	// Cursor.
	const PatCursor & cursor() const;
	void setCursor( const PatCursor & cursor );
	void moveCursor( int dx, int dy );
	const QFont & font() const;
	void setFont( QFont font );

	// Colour.
	void setSeparatorColor( const QColor & color );
	const QColor & separatorColor() const;
	void setLineSeparatorColor( const QColor & color );
	const QColor & lineSeparatorColor() const;
	void setRestAreaColor( const QColor & color );
	const QColor & restArea() const;
	void setBigTrackSeparatorColor( const QColor & selColor );
	const QColor & bigTrackSeparatorColor() const;
	void setSmallTrackSeparatorColor( const QColor & color );
	const QColor & smallTrackSeparatorColor() const;
	void setSelectionColor( const QColor & selColor );
	const QColor & selectionColor() const;
	void setCursorColor( const QColor & cursorColor );
	const QColor & cursorColor() const;
	void setCursorTextColor( const QColor & cursorTextColor );
	const QColor & cursorTextColor() const;
	void setBarColor( const QColor & barColor );
	const QColor & barColor() const;
	void setBeatColor( const QColor & beatColor );
	const QColor & beatColor() const;
	void setPlayBarColor( const QColor & playBarColor );
	const QColor & playBarColor() const;
	void setBeatTextColor( const QColor & color );
	const QColor & beatTextColor();
	void setTextColor( const QColor & color);
	const QColor & textColor() const;

	bool doingKeybasedSelect() { return doingKeybasedSelect_; }
	void setDoingKeybasedSelect( bool setit ) { doingKeybasedSelect_ = setit; }
	bool lineAlreadySelected( int lineNumber );
	bool trackAlreadySelected( int trackNumber );
	void startKeybasedSelection(int leftPos, int rightPos, int topPos, int bottomPos);
	void startMouseSelection( const PatCursor & p );
	void repaintSelection();
	Selection selection() const;
	void repaintCursor();
	void setOldCursor( const PatCursor & p ) { oldCursor_ = p; }
	std::vector<ColumnEvent> events() { return events_; }
	const PatCursor & selStartPoint() const;
	PatCursor intersectCell( int x, int y );
	int visibleColWidth( int maxEvents ) const;

	// Actions.
	void doNoteEvent( int note );
	void doInstrumentEvent( int keyChar );
	void doMachineSelectionEvent( int keyChar );
	void doVolumeEvent( int keyChar );
	void doCommandOrParameterEvent( int keyChar );

	void startBlock( const PatCursor & cursor );
	void changeBlock( const PatCursor & cursor );
	void endBlock( const PatCursor & cursor );
	void unmarkBlock();

	void copyBlock( bool cutit );
	void pasteBlock( int tx,int lx,bool mix );
	void deleteBlock( );

	void selectUp();
	void selectDown();
	void selectLeft();
	void selectRight();
	void selectTrack();
	void selectAll();
	void navTop();
	void navBottom();
	void trackPrev();
	void trackNext();
	void centerOnCursor();
	void checkLeftScroll( const PatCursor & cursor );
	void checkRightScroll( const PatCursor & cursor );
	void checkUpScroll( const PatCursor & cursor );
	void checkDownScroll( const PatCursor & cursor );


protected:
	bool event(QEvent *event);
	void mousePressEvent( QGraphicsSceneMouseEvent *event );
	void mouseReleaseEvent( QGraphicsSceneMouseEvent *event );
	void mouseMoveEvent( QGraphicsSceneMouseEvent *event );
	void keyPressEvent( QKeyEvent *event );

private:
	Selection selection_;
	Selection oldSelection_;
	PatCursor selStartPoint_;
	PatCursor selCursor_;

	bool doingKeybasedSelect_;
	bool doingMouseSelect_;

	bool ft2HomeEndBehaviour_;
	bool shiftArrowForSelect_;
	bool wrapAround_;
	bool centerCursor_;

	bool blockSelected_;

	bool isBlockCopied_;
	psy::core::SinglePattern pasteBuffer;
	float lastXmlLineBeatPos;
	int xmlTracks;
	float xmlBeats;

	int cellWidth_;


	PatternDraw *patDraw_;

	std::vector<ColumnEvent> events_;

	PatCursor cursor_;
	PatCursor oldCursor_;
	QFont font_;

	QColor textColor_;
	QColor separatorColor_;
	QColor selectionColor_;
	QColor cursorColor_;
	QColor cursorTextColor_ ;
	QColor barColor_ ;
	QColor beatColor_;
	QColor beatTextColor_; 
	QColor playBarColor_;
	QColor bigTrackSeparatorColor_; 
	QColor smallTrackSeparatorColor_;
	QColor lineSepColor_;
	QColor restAreaColor_ ;

};

#endif
