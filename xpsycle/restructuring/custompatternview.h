/***************************************************************************
*   Copyright (C) 2006 by  Stefan Nattkemper   *
*   natti@linux   *
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
#ifndef CUSTOMPATTERNVIEW_H
#define CUSTOMPATTERNVIEW_H

#include <ngrs/panel.h>
#include <ngrs/point3d.h>

#include "inputhandler.h"

/**
@author  Stefan Nattkemper
*/

namespace psy {
	namespace host	{	

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

		class CustomPatternView;

		class TrackGeometry {
		public:			
			TrackGeometry();

			TrackGeometry( CustomPatternView & patternView );

			~TrackGeometry();

			void setLeft( int left );
			int left() const;

			void setWidth( int width );
			int width() const;			

			void setVisibleColumns( int cols );
			int visibleColumns() const;

			void setVisible( bool on);
			bool visible() const;

		private:

			CustomPatternView* pView;
			int left_;
			int width_;
			int visibleColumns_;
			bool visible_;

		};

		class CustomPatternView : public ngrs::Panel {
		public:
			CustomPatternView();

			~CustomPatternView();

			enum SelDirection { nodir = 0, north = 1, west = 2, east = 4, south = 8};

			virtual int colWidth() const;
			virtual int visibleColWidth( int maxEvents ) const;

			virtual int rowHeight() const;
			virtual int lineNumber() const;
			virtual bool lineAlreadySelected(int lineNumber); 

			void setTrackNumber( int number );
			virtual int trackNumber() const;
			virtual bool trackAlreadySelected(int trackNumber); 

			virtual int beatZoom() const;

			void setPatternStep( int step );
			int patternStep() const;

			void setSeparatorColor( const ngrs::Color & color );
			const ngrs::Color & separatorColor() const;

			void setLineSeparatorColor( const ngrs::Color & color );
			const ngrs::Color & lineSeparatorColor() const;

			void setRestAreaColor( const ngrs::Color & color );
			const ngrs::Color & restArea() const;

			void setBigTrackSeparatorColor( const ngrs::Color & selColor );
			const ngrs::Color & bigTrackSeparatorColor() const;

			void setSmallTrackSeparatorColor( const ngrs::Color & color );
			const ngrs::Color & smallTrackSeparatorColor() const;

			void setSelectionColor( const ngrs::Color & selColor );
			const ngrs::Color & selectionColor() const;

			void setCursorColor( const ngrs::Color & cursorColor );
			const ngrs::Color & cursorColor() const;

			void setBarColor( const ngrs::Color & barColor );
			const ngrs::Color & barColor() const;

			void setBeatColor( const ngrs::Color & beatColor );
			const ngrs::Color & beatColor() const;

			void setPlayBarColor( const ngrs::Color & playBarColor );
			const ngrs::Color & playBarColor() const;

			void setLineGridEnabled( bool on );
			bool lineGridEnabled() const;

			void setColGridEnabled( bool on );
			bool colGridEnabled() const;

			void setBeatTextColor( const ngrs::Color & color );
			const ngrs::Color & beatTextColor();

			void setTextColor( const ngrs::Color & color);
			const ngrs::Color & textColor() const;

			void setTrackLeftIdent( int ident );
			int trackLeftIdent() const;

			void setTrackRightIdent( int ident );
			int trackRightIdent() const;

			void setBigTrackSeparatorWidth( int ident );
			int bigTrackSeparatorWidth() const;

			void setDx(int dx);
			int dx() const;
			void setDy(int dy);
			int dy() const;

			virtual void paint( ngrs::Graphics& g );

			virtual void onMousePress( int x, int y, int button );
			virtual void onMousePressed( int x, int y, int button );
			virtual void onMouseOver( int x, int y );

			virtual void onKeyPress( const ngrs::KeyEvent & event );
			virtual void onKeyRelease( const ngrs::KeyEvent & event );

			void repaintCursorPos( const PatCursor & cursor );
			void repaintBlock( const ngrs::Size & block );
			ngrs::Rect repaintTrackArea( int startLine, int endLine, int startTrack, int endTrack ) const;
			ngrs::Point linesFromRepaint( const ngrs::Region & repaintArea) const;
			ngrs::Point tracksFromRepaint( const ngrs::Region & repaintArea) const;

			const ngrs::Size & selection() const;
			void clearOldSelection();
			void repaintSelection();
			void startKeybasedSelection( int leftPos, int rightPos, int topPos, int bottomPos );

			void addEvent( const ColumnEvent & event );
			std::string noteToString( int value, bool sharp );

			void drawData( ngrs::Graphics& g, int track, int line, int eventnr, int data , bool sharp, const ngrs::Color & color );
			// bypass column type
			void drawString( ngrs::Graphics& g, int track, int line, int eventnr, const std::string & data , const ngrs::Color & color );

			const PatCursor & cursor() const;
			void setCursor( const PatCursor & cursor );

			void moveCursor( int dx, int dy ); // dx is one hex digit

			unsigned char convertDigit( int defaultValue, int scanCode, unsigned char oldByte, int col ) const;
			bool isHex( int scanCode );

			int tracksWidth() const;

			const std::map<int, TrackGeometry> & trackGeometrics() const;

			int findTrackByScreenX( int x ) const;
			int xOffByTrack( int track ) const;
			int xEndByTrack( int track ) const;
			int trackWidth( int track ) const;

			void setVisibleEvents( int track , int eventCount );
			int visibleEvents( int track ) const;

			void setDefaultVisibleEvents( int defaultSize );
			void setTrackMinWidth( int size );

			void alignTracks();

		protected:

			virtual PatCursor intersectCell( int x, int y );
			const PatCursor & selCursor() const;
			virtual void startSel( const PatCursor & p );
			virtual int doSel( const PatCursor & p );
			virtual void endSel();
			virtual void selectAll( const PatCursor & p );
			virtual void selectColumn( const PatCursor & p );

			virtual int noteCellWidth() const;
			virtual int cellWidth() const;

			const PatCursor & selStartPoint() const;
			bool doSelect() const;
			bool doDrag() const;
			bool doingKeybasedSelect() const;

			virtual void customPaint( ngrs::Graphics& g, int startLine, int endLine, int startTrack, int endTrack );

			virtual void drawTrackGrid( ngrs::Graphics&g, int startLine, int endLine, int startTrack, int endTrack  );

			virtual void drawColumnGrid( ngrs::Graphics&g, int startLine, int endLine, int startTrack, int endTrack  );

			virtual void drawPattern( ngrs::Graphics& g, int startLine, int endLine, int startTrack, int endTrack );

			virtual void drawRestArea( ngrs::Graphics& g, int startLine, int endLine, int startTrack, int endTrack );

			virtual void drawSelBg( ngrs::Graphics& g, const ngrs::Size & selArea );

			virtual void drawCellBg( ngrs::Graphics& g, const PatCursor & cursor );


		private:

			InputHandler inputHandler;
			int dx_, dy_;
			int trackNumber_;

			// selection variables
			bool doDrag_;
			bool doSelect_;
			bool doingKeybasedSelect_;
			ngrs::Size selection_;
			ngrs::Size oldSelection_; // we cut motionButton Events, so not every mousemotion is recognized
			PatCursor selStartPoint_;				

			int defaultSize_;

			//cursor
			PatCursor cursor_;
			PatCursor selCursor_; // for keyboard drag

			ngrs::Color separatorColor_;
			ngrs::Color selectionColor_;
			ngrs::Color cursorColor_;
			ngrs::Color barColor_;
			ngrs::Color beatColor_;
			ngrs::Color playBarColor_;
			ngrs::Color bigTrackSeparatorColor_;
			ngrs::Color smallTrackSeparatorColor_;
			ngrs::Color lineSepColor_;
			ngrs::Color textColor_;
			ngrs::Color beatTextColor_;
			ngrs::Color restAreaColor_;
			ngrs::Color cursorTextColor_;

			bool lineGridEnabled_;
			bool colGridEnabled_;

			int patternStep_;
			int colIdent;

			void init();

			std::vector<ColumnEvent> events_;

			void drawBlockData( ngrs::Graphics& g, int xOff, int line, const std::string & text, const ngrs::Color & color );

			void drawStringData( ngrs::Graphics& g, int xOff, int line, const std::string & text , const ngrs::Color & color);

			void updateStatusBar();

			int eventOffset( int eventnr, int col ) const;
			int eventWidth( int eventnr ) const;
			int eventColWidth( int eventnr ) const;

			std::map<int, TrackGeometry> trackGeometryMap;


			int trackMinWidth_;
			int trackLeftIdent_;
			int trackRightIdent_;

			std::string defaultNoteStr_;
		};

	}
}

#endif
