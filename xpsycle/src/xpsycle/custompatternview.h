/***************************************************************************
 *   Copyright (C) 2006 by Stefan Nattkemper   *
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

#include <ngrs/npanel.h>
#include <ngrs/npoint3d.h>

/**
@author Stefan Nattkemper
*/

namespace psycle {
	namespace host	{	

		class ColumnEvent {			
			public:

				ColumnEvent( int type );
				~ColumnEvent();

				enum colType { hex2 = 0, hex4 = 1, note = 2 };

				int type() const;
				int cols() const;

			private:
				
				int type_;
		};

		class PatCursor {
		public:

			PatCursor();
			PatCursor( int track, int line, int eventNr, int col );
			~PatCursor();

			void setPosition( int track, int line, int eventNr, int col );

			void setTrack( int x);
			int track() const;
			void setLine( int y);
			int line() const;
			void setEventNr( int event );
			int eventNr() const;
			void setCol( int col);
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

			TrackGeometry( CustomPatternView* patternView );

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
			int visible_;

		};

		class CustomPatternView : public NPanel
		{
			public:

					CustomPatternView();

					~CustomPatternView();

					enum SelDirection { nodir = 0, north = 1, west = 2, east = 4, south = 8};

					virtual int colWidth() const;
					virtual int visibleColWidth( int maxEvents ) const;

					virtual int rowHeight() const;
					virtual int lineNumber() const;

					void setTrackNumber( int number );
					virtual int trackNumber() const;
					
					virtual int beatZoom() const;

					void setPatternStep( int step );
					int patternStep() const;

					void setSeparatorColor( const NColor & color );
					const NColor & separatorColor() const;

					void setLineSeparatorColor( const NColor & color );
					const NColor & lineSeparatorColor() const;

					void setRestAreaColor( const NColor & color );
					const NColor & restArea() const;

					void setBigTrackSeparatorColor( const NColor & selColor );
					const NColor & bigTrackSeparatorColor() const;

					void setSmallTrackSeparatorColor( const NColor & color );
					const NColor & smallTrackSeparatorColor() const;

					void setSelectionColor( const NColor & selColor );
					const NColor & selectionColor() const;

					void setCursorColor( const NColor & cursorColor );
					const NColor & cursorColor() const;

					void setBarColor( const NColor & barColor );
					const NColor & barColor() const;

					void setBeatColor( const NColor & beatColor );
					const NColor & beatColor() const;

					void setPlayBarColor( const NColor & playBarColor );
					const NColor & playBarColor() const;

					void setLineGridEnabled( bool on );
					bool lineGridEnabled() const;

					void setColGridEnabled( bool on );
					bool colGridEnabled() const;

					void setBeatTextColor( const NColor & color );
					const NColor & beatTextColor();

					void setTextColor( const NColor & color);
					const NColor & textColor() const;

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

					virtual void paint(NGraphics* g);

					virtual void onMousePress(int x, int y, int button);
					virtual void onMousePressed(int x, int y, int button);
					virtual void onMouseOver	(int x, int y);

					virtual void onKeyPress(const NKeyEvent & event);
					virtual void onKeyRelease(const NKeyEvent & event);

					void repaintCursorPos( const PatCursor & cursor );
					void repaintBlock( const NSize & block );
					NRect repaintTrackArea(int startLine,int endLine,int startTrack, int endTrack) const;
      		NPoint linesFromRepaint(const NRegion & repaintArea) const;
      		NPoint tracksFromRepaint(const NRegion & repaintArea) const;

					const NSize & selection() const;
					void clearOldSelection();

					void addEvent( const ColumnEvent & event );
					std::string noteToString( int value, bool sharp );

					void drawData(NGraphics* g, int track, int line, int eventnr, int data , bool sharp, const NColor & color);
					// bypass column type
					void drawString(NGraphics* g, int track, int line, int eventnr, const std::string & data , const NColor & color);

					const PatCursor & cursor() const;
					void setCursor( const PatCursor & cursor );

					int moveCursor( int dx, int dy ); // dx is one hex digit

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

					virtual PatCursor intersectCell(int x, int y);
					const PatCursor & selCursor() const;
					virtual void startSel(const PatCursor & p);
					virtual int doSel(const PatCursor & p);
					virtual void endSel();

					virtual int noteCellWidth() const;
					virtual int cellWidth() const;

					const PatCursor & selStartPoint() const;
					bool doSelect() const;
					bool doDrag() const;

					virtual void customPaint( NGraphics* g, int startLine, int endLine, int startTrack, int endTrack );
					
					virtual void drawTrackGrid(NGraphics*g, int startLine, int endLine, int startTrack, int endTrack  );

					virtual void drawColumnGrid(NGraphics*g, int startLine, int endLine, int startTrack, int endTrack  );

					virtual void drawPattern(NGraphics* g, int startLine, int endLine, int startTrack, int endTrack);

					virtual void drawRestArea(NGraphics* g, int startLine, int endLine, int startTrack, int endTrack);

					virtual void drawSelBg( NGraphics* g, const NSize & selArea );

					virtual void drawCellBg(NGraphics* g, const PatCursor & cursor );


			private:

					int dx_, dy_;
					int trackNumber_;

					// selection variables
					bool doDrag_;
					bool doSelect_;
					bool doShiftSel_;
					NSize selection_;
					NSize oldSelection_; // we cut motionButton Events, so not every mousemotion is recognized
					PatCursor selStartPoint_;				

					int defaultSize_;

					//cursor
					PatCursor cursor_;
					PatCursor selCursor_; // for keyboard drag

					NColor separatorColor_;
					NColor selectionColor_;
					NColor cursorColor_;
					NColor barColor_;
					NColor beatColor_;
					NColor playBarColor_;
					NColor bigTrackSeparatorColor_;
					NColor smallTrackSeparatorColor_;
					NColor lineSepColor_;
					NColor textColor_;
					NColor beatTextColor_;
					NColor restAreaColor_;
					NColor cursorTextColor_;

					bool lineGridEnabled_;
					bool colGridEnabled_;

					int patternStep_;

					int colIdent;
					
					void init();

					std::vector<ColumnEvent> events_;

					void drawBlockData( NGraphics * g, int xOff, int line, const std::string & text, const NColor & color );

					void drawStringData(NGraphics* g, int xOff, int line, const std::string & text , const NColor & color);


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
