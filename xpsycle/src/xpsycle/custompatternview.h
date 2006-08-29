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

			private:
				
				int type_;
		};

		class CustomPatternView : public NPanel
		{
			public:
					CustomPatternView();

					~CustomPatternView();

					virtual int colWidth() const;
					virtual int rowHeight() const;
					virtual int lineNumber() const;
					virtual int trackNumber() const;
					virtual int beatZoom() const;

					const NColor & separatorColor() const;
					const NColor & selectionColor() const;

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

					void repaintBlock( const NSize & block );
					NRect repaintTrackArea(int startLine,int endLine,int startTrack, int endTrack) const;
      		NPoint linesFromRepaint(const NRegion & repaintArea) const;
      		NPoint tracksFromRepaint(const NRegion & repaintArea) const;

					const NSize & selection() const;
					void clearOldSelection();

					void addEvent( const ColumnEvent & event );
					std::string noteToString( int value );

					void drawDataN(NGraphics* g, int track, int line, int eventnr, int data );

			protected:

					virtual NPoint3D intersectCell(int x, int y);
					virtual void startSel(const NPoint3D & p);
					virtual void doSel(const NPoint3D & p);
					virtual void endSel();

					virtual int noteCellWidth() const;
					virtual int cellWidth() const;

					const NPoint3D & selStartPoint() const;
					bool doSelect() const;
					bool doDrag() const;

					virtual void customPaint( NGraphics* g, int startLine, int endLine, int startTrack, int endTrack );
					

					virtual void drawTrackGrid(NGraphics*g, int startLine, int endLine, int startTrack, int endTrack  );

					virtual void drawColumnGrid(NGraphics*g, int startLine, int endLine, int startTrack, int endTrack  );

					virtual void drawPattern(NGraphics* g, int startLine, int endLine, int startTrack, int endTrack);

					virtual void drawRestArea(NGraphics* g, int startLine, int endLine, int startTrack, int endTrack);

					virtual void drawSelBg( NGraphics* g, const NSize & selArea );


			private:

					int dx_, dy_;

					// selection variables
					bool doDrag_;
					bool doSelect_;
					bool doShiftSel_;
					NSize selection_;
					NSize oldSelection_; // we cut motionButton Events, so not every mousemotion is recognized
					NPoint3D selStartPoint_;				

					NColor separatorColor_;
					NColor selectionColor_;

					void init();

					std::vector<ColumnEvent> events_;

					void drawBlockData( NGraphics * g, int track, int line, int eventOffset, const std::string & text );

					void drawStringData(NGraphics* g, int track, int line, int eventOffset, const std::string & text );

					int eventOffset( int eventnr ) const;

		};
	}
}

#endif
