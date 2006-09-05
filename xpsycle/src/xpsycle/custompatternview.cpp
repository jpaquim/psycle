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
#include "custompatternview.h"
#include "inputhandler.h"

#include <ngrs/nwindow.h>
#include <ngrs/nfontmetrics.h>
#include <iostream>
#include <iomanip>

namespace psycle {
	namespace host	{	

		template<class T> inline std::string toHex(T value , int nums = 2) {
			std::ostringstream buffer;
			buffer.setf(std::ios::uppercase);

			buffer.str("");
			buffer << std::setfill('0') << std::hex << std::setw( nums );
			buffer << (int) value;
			return buffer.str();
		}


		ColumnEvent::ColumnEvent( int type ) {
			type_ = type;
		}

    ColumnEvent::~ColumnEvent() {
		}

		int ColumnEvent::type() const {
			return type_;
		}

		int ColumnEvent::cols() const {
			int cols_ = 1;
			switch ( type_ ) {
				case ColumnEvent::hex2 : cols_ = 2; break;
				case ColumnEvent::hex4 : cols_ = 4; break;
				case ColumnEvent::note : cols_ = 1; break;
				default: ;
			}
			return cols_;
		}


		PatCursor::PatCursor() :
			track_(0), 
			line_(0), 
			eventNr_(0), 
			col_(0) 
		{
		}

		PatCursor::PatCursor(int track, int line, int eventNr, int col) :
			track_( track ), 
			line_( line ), 
 			eventNr_( eventNr ), 
			col_( col ) 
		{
		}

		PatCursor::~PatCursor() {

		}

		void PatCursor::setPosition( int track, int line, int eventNr, int col ) {
			track_ = track;
			line_  = line;
			eventNr_ = eventNr;
			col_ = col;
		}

		void PatCursor::setTrack( int track ) {
			track_ = track;
		}

		int PatCursor::track() const {
			return track_;
		}

		void PatCursor::setLine( int line ) {
			line_ = line;
		}

		int PatCursor::line() const {
			return line_;
		}

		void PatCursor::setEventNr( int eventNr ) {
			eventNr_ = eventNr;
		}

		int PatCursor::eventNr() const {
			return eventNr_;
		}

		void PatCursor::setCol( int col) {
			col_ = col;
		}

		int PatCursor::col() const {
			return col_;
		}



		CustomPatternView::CustomPatternView()
			 : NPanel(), doDrag_(0),doSelect_(0)
		{
			init();
		}

		CustomPatternView::~CustomPatternView()
		{
		}

		void CustomPatternView::addEvent( const ColumnEvent & event ) {
			events_.push_back( event );
		}
	
		void CustomPatternView::init() {			
			dx_ = 0;
			dy_ = 0;
			separatorColor_ = NColor(200,200,200);
			selectionColor_ = NColor(0,0,255);
			colIdent = 3;
		}

		int CustomPatternView::lineNumber() const {
			return 100;
		}

		int CustomPatternView::trackNumber() const {
			return 16;
		}
	
		int CustomPatternView::colWidth( ) const
		{
			std::vector<ColumnEvent>::const_iterator it = events_.begin();
			int offset = 0;
			for ( ; it < events_.end(); it++) {
				const ColumnEvent & event = *it;

				switch ( event.type() ) {
					case ColumnEvent::hex2 : offset+= 2*cellWidth(); 	break;
					case ColumnEvent::hex4 : offset+= 4*cellWidth(); 	break;
					case ColumnEvent::note : offset+= noteCellWidth(); break;
					default: ;
				}
			}

			return offset + colIdent ;
		}
				
		int CustomPatternView::rowHeight() const {
			return 12;
		}

		int CustomPatternView::beatZoom() const {
			return 4;
		}

		const NColor & CustomPatternView::separatorColor() const {
			return separatorColor_;
		}

		const NColor & CustomPatternView::selectionColor() const {
			return selectionColor_;
		}

		void CustomPatternView::setCursor( const PatCursor & cursor ) {
			cursor_ = cursor;
		}

		const PatCursor & CustomPatternView::cursor() const {
			return cursor_;
		}

		void CustomPatternView::setDx(int dx) {
			dx_ = dx;
		}

		int CustomPatternView::dx() const {
			return dx_;
		}

		void CustomPatternView::setDy(int dy) {
			dy_ = dy;
		}

		int CustomPatternView::dy() const {
			return dy_;
		}

		void CustomPatternView::paint(NGraphics* g) {

			NPoint lineArea = linesFromRepaint(g->repaintArea());
			int startLine = lineArea.x();
    	int endLine   = lineArea.y();

    	NPoint trackArea = tracksFromRepaint(g->repaintArea());
    	int startTrack = trackArea.x();
    	int endTrack   = trackArea.y();

			customPaint(g, startLine, endLine, startTrack, endTrack);
				
		}

		void CustomPatternView::drawRestArea(NGraphics* g, int startLine, int endLine, int startTrack, int endTrack) {
			g->setForeground(NColor(0,0,80));
			int endTop     = lineNumber() * rowHeight() - dy();
			int endHeight  = std::max(0, clientHeight() - endTop);
			g->fillRect(0,endTop,clientWidth(),endHeight);

			int endLeft     = trackNumber() * colWidth() - dx();
			int endWidth    = std::max(0, clientWidth() - endLeft);
			g->fillRect(endLeft,0,endWidth,clientHeight());
		}

		void CustomPatternView::drawTrackGrid(NGraphics*g, int startLine, int endLine, int startTrack, int endTrack  ) {
			int trackWidth = ((endTrack+1) * colWidth()) - dx();
  		int lineHeight = ((endLine +1) * rowHeight()) - dy();

			g->setForeground( foreground() );

			for (int y = startLine; y <= endLine; y++)
      g->drawLine(0,y* rowHeight() - dy(),trackWidth,y* rowHeight()-dy());

			for (int i = startTrack; i <= endTrack; i++) //  oolIdent px space at begin of trackCol
      g->fillRect(i*colWidth()-dx(),0,colIdent,lineHeight);

			g->setForeground( separatorColor() );
				for (int i = startTrack; i <= endTrack; i++)  // col separators
      g->drawLine(i* colWidth()-dx(),0,i* colWidth()-dx(),lineHeight);

    	g->setForeground( foreground() );

		}

		int CustomPatternView::noteCellWidth() const {
			NFontMetrics metrics(font());
			int width = metrics.textWidth("C#-10");
			return width;
		}

		int CustomPatternView::cellWidth() const {
			return 12;
		}

		void CustomPatternView::drawColumnGrid(NGraphics*g, int startLine, int endLine, int startTrack, int endTrack  ) {
			int trackWidth = ((endTrack+1) * colWidth()) - dx();
			int lineHeight = ((endLine +1) * rowHeight()) - dy();

			for (int x = startTrack; x <= endTrack; x++) {
				std::vector<ColumnEvent>::iterator it = events_.begin();
				int col = 0;
				for ( ; it < events_.end(); it++) {
					ColumnEvent & event = *it;
        	switch ( event.type() ) {
						case ColumnEvent::hex2 : col+= 2*cellWidth(); 	break;
						case ColumnEvent::hex4 : col+= 4*cellWidth(); 	break;
						case ColumnEvent::note : col+= noteCellWidth(); break;
					}
					g->drawLine(x*colWidth()+colIdent+col-dx(),0,x*colWidth()+colIdent+col-dx(),lineHeight);
				}
			}
		}

		void CustomPatternView::drawPattern(NGraphics* g, int startLine, int endLine, int startTrack, int endTrack) {

		}

		void CustomPatternView::drawData(NGraphics* g, int track, int line, int eventnr, int data ) {
			if ( eventnr < events_.size() ) {
				const ColumnEvent & event = events_.at( eventnr );
				switch ( event.type() ) {
					case ColumnEvent::hex2 :
						drawBlockData( g, track, line, eventOffset(eventnr,0), toHex(data,2) );
					break;
					case ColumnEvent::hex4 :
						drawBlockData( g, track, line, eventOffset(eventnr,0), toHex(data,4) );
					break;
					case ColumnEvent::note :					
						drawStringData( g, track, line, eventOffset(eventnr,0), noteToString(data) );
					break;
					default: ;
				}
			}
		}

		void CustomPatternView::drawBlockData( NGraphics * g, int track, int line, int eventOffset, const std::string & text )
		{
			int xOff = track * colWidth()+ colIdent + eventOffset - dx();
			int yOff = line  * rowHeight() + rowHeight()  - dy();

			int col = 0;
			for (int i = 0; i < text.length(); i++) {
				g->drawText(xOff + col,yOff,text.substr(i,1));
				col += cellWidth();
			}
		}

		void CustomPatternView::drawStringData(NGraphics* g, int track, int line, int eventOffset, const std::string & text )
		{
			int xOff = track * colWidth()+ colIdent + eventOffset - dx();
			int yOff = line  * rowHeight() + rowHeight()  - dy();

			g->drawText(xOff,yOff,text);
		}

		void CustomPatternView::drawCellBg(NGraphics* g, const PatCursor & cursor, const NColor & bgColor) {
			int xOff = cursor.track() * colWidth() + colIdent - dx();
  		int yOff = cursor.line()  * rowHeight()  - dy();
  		int colOffset = eventOffset( cursor.eventNr(), cursor.col() );
			g->setForeground(bgColor);
			g->fillRect( xOff + colOffset, yOff, eventColWidth( cursor.eventNr() ), rowHeight() );
		}

		int CustomPatternView::eventOffset( int eventnr, int col ) const {
			std::vector<ColumnEvent>::const_iterator it = events_.begin();
			int nr = 0;
			int offset = 0;
			for ( ; it < events_.end(); it++, nr++ ) {
				const ColumnEvent & event = *it;
				if (nr == eventnr) {
					int colOff = col * cellWidth();
					return offset + colOff;
				}
				switch ( event.type() ) {
					case ColumnEvent::hex2 : offset+= 2*cellWidth(); 	break;
					case ColumnEvent::hex4 : offset+= 4*cellWidth(); 	break;
					case ColumnEvent::note : offset+= noteCellWidth(); break;
					default: ;
				}
			}
			return -1;
		}

		int CustomPatternView::eventWidth( int eventnr ) const {	
			int eventWidth_ = 0;
			if ( eventnr < events_.size() ) {
				const ColumnEvent & event = events_.at(eventnr);
			
				switch ( event.type() ) {
					case ColumnEvent::hex2 : eventWidth_= 2*cellWidth(); 	break;
					case ColumnEvent::hex4 : eventWidth_= 4*cellWidth(); 	break;
					case ColumnEvent::note : eventWidth_= noteCellWidth(); break;
					default: ;
				}
			}
			return eventWidth_;
		}

		int CustomPatternView::eventColWidth( int eventnr ) const {
			int eventColWidth_ = 0;
			if ( eventnr < events_.size() ) {
				const ColumnEvent & event = events_.at(eventnr);
			
				switch ( event.type() ) {
					case ColumnEvent::hex2 : eventColWidth_= cellWidth(); 	break;
					case ColumnEvent::hex4 : eventColWidth_= cellWidth(); 	break;
					case ColumnEvent::note : eventColWidth_= noteCellWidth(); break;
					default: ;
				}
			}
			return eventColWidth_;
		}

		void CustomPatternView::customPaint( NGraphics* g, int startLine, int endLine, int startTrack, int endTrack ) {
			drawTrackGrid(g, startLine, endLine, startTrack, endTrack);			
			drawColumnGrid(g, startLine, endLine, startTrack, endTrack);
			drawPattern(g, startLine, endLine, startTrack, endTrack);
			drawRestArea(g, startLine, endLine, startTrack, endTrack);
			drawSelBg( g, selection() );
		}

		void CustomPatternView::drawSelBg(NGraphics* g, const NSize & selArea) {
			int x1Off = selArea.left() * colWidth()  ;
			int y1Off = selArea.top()  * rowHeight() ;

			int x2Off = selArea.right()  * colWidth() ;
			int y2Off = selArea.bottom() * rowHeight();

			g->setForeground( selectionColor() );
			g->fillRect(x1Off - dx(), y1Off -dy(), x2Off-x1Off, y2Off-y1Off);
		}

		void CustomPatternView::onMousePress(int x, int y, int button) {
			if ( button == 1) {
    		clearOldSelection();
    		PatCursor p = intersectCell(x,y);
    		startSel(p);
  		}
		}

		void CustomPatternView::onMousePressed(int x, int y, int button) {
			if (button == 1) {
				endSel(); 
    		if ( !doSelect() ) cursor_ = intersectCell(x,y);
    		repaint();    
			}
		}

		void CustomPatternView::onMouseOver	(int x, int y) {
			if (doDrag_) {
    		PatCursor p = intersectCell(x,y);
    		doSel(p);
  		}
		}

		void CustomPatternView::onKeyPress(const NKeyEvent & event) {
			if (doDrag() != (NApp::system().keyState() & ShiftMask) &&
                     !(NApp::system().keyState() & ControlMask)) {
				if (!doDrag()) {
					clearOldSelection();
					startSel( cursor() );
					selCursor_ = cursor();
					selCursor_.setEventNr(0);
					selCursor_.setCol(0);
      	}
			}

			// navigation
			switch (event.scancode()) {
				case XK_Tab  :
				break;
				case XK_Left :
					moveCursor(-1,0);
				break;
				case XK_Right:
					moveCursor(1,0);
				break;
				case XK_Up:
					moveCursor(0,-1);
				break;
				case XK_Down:
					moveCursor(0,1);
				break;
				case XK_Page_Up:
				break;
				case XK_Page_Down:
				break;
			}

		}

		int CustomPatternView::moveCursor( int dx, int dy) {
			// dx -1 left hex digit move
			// dx +1 rigth hex digit move
			// dy in lines
			PatCursor oldCursor = cursor_;
			int eventnr = cursor().eventNr();
			if ( dx > 0 ) {			
				if ( eventnr < events_.size() ) {
					const ColumnEvent & event = events_.at( eventnr );
					int maxCols = event.cols();
					if ( cursor_.col() + dx < maxCols ) {
						cursor_.setCol( cursor_.col() + dx);
					} else
					if (eventnr + 1 < events_.size() ) {
						cursor_.setCol( 0 );
						cursor_.setEventNr( eventnr + 1);
					} else {
						cursor_.setTrack( cursor_.track() + 1 );
						cursor_.setEventNr(0);
						cursor_.setCol(0);
					}
					window()->repaint(this,repaintTrackArea( cursor_.line(), cursor_.line(), oldCursor.track(), cursor_.track()) );
				}
			} else 
			if ( dx < 0 ) {
				if ( cursor_.col() + dx >= 0 ) {
					cursor_.setCol( cursor_.col() + dx);
				} else 
				if ( cursor_.eventNr() > 0 ) {
					cursor_.setEventNr( cursor_.eventNr() - 1 );
					const ColumnEvent & event = events_.at( cursor_.eventNr() );
					cursor_.setCol( event.cols() - 1 );					
				} else {
					if ( cursor_.track() > 0 ) {
						cursor_.setTrack( cursor_.track() -1 );
						cursor_.setEventNr( events_.size() -1 );
						const ColumnEvent & event = events_.at( cursor_.eventNr() );
						cursor_.setCol( event.cols() - 1 );
					}		
				}
				window()->repaint(this,repaintTrackArea( cursor_.line(), cursor_.line(), cursor_.track(), oldCursor.track()) );
			}

			if ( dy != 0 && (dy + cursor_.line() >= 0) ) {
				cursor_.setLine( cursor_.line() + dy);
				window()->repaint(this,repaintTrackArea( oldCursor.line(), oldCursor.line(), oldCursor.track(), oldCursor.track()) );
				window()->repaint(this,repaintTrackArea( cursor_.line(), cursor_.line(), cursor_.track(), cursor_.track()) );
			} else if (dy!=0) {
				window()->repaint(this,repaintTrackArea( cursor_.line(), cursor_.line(), cursor_.track(), cursor_.track()) );
			}		
		}

		void CustomPatternView::onKeyRelease(const NKeyEvent & event) {
			if ( event.scancode() == XK_Shift_L || event.scancode() == XK_Shift_R ) {
				endSel();
			}
		}

		char hex_value(char c) { if(c >= 'A') return 10 + c - 'A'; else return c - '0'; }

		unsigned char CustomPatternView::convertDigit( int defaultValue, int scanCode, unsigned char oldByte, int col ) const {
			unsigned char newByte = 0;
 			if (col == 0) {
        if (oldByte == defaultValue)
          newByte = ( 0 & 0x0F ) | ( 0xF0 & (hex_value(scanCode) << 4 ) );
        else
				  newByte = ( oldByte & 0x0F ) | ( 0xF0 & ( hex_value(scanCode) << 4) );
      }
			else {
        if (oldByte == defaultValue)
				  newByte = ( 0 & 0xF0 ) | ( 0x0F & (hex_value(scanCode)) );
        else
          newByte = ( oldByte & 0xF0 ) | ( 0x0F & (hex_value(scanCode) ) );
      }

			return newByte;
		}

		bool CustomPatternView::isHex( int c ) {
			if ( ( (c >= 'a') && (c <='f') ) || ( (c>='0' && c <='9') ) ) return true;
			return false;
		}
		
		void CustomPatternView::repaintBlock( const NSize & block ) {
  		window()->repaint(this,repaintTrackArea(block.top(),block.bottom(),block.left(), block.right()));
		}

		void CustomPatternView::repaintCursorPos( const PatCursor & cursor ) {
			window()->repaint(this,repaintTrackArea(cursor.line(),cursor.line(),cursor.track(), cursor.track()));
		}

		NRect CustomPatternView::repaintTrackArea(int startLine,int endLine,int startTrack, int endTrack) const {
			int top    = startLine    * rowHeight()  + absoluteTop()  - dy_;
  		int bottom = (endLine+1)  * rowHeight()  + absoluteTop()  - dy_;
  		int left   = startTrack   * colWidth()   + absoluteLeft() - dx_;
  		int right  = (endTrack+1) * colWidth()   + absoluteLeft() - dx_;

  		return NRect(left,top,right - left,bottom - top);
		}

		NPoint CustomPatternView::linesFromRepaint(const NRegion & repaintArea) const {
			NRect repaintRect = repaintArea.rectClipBox();
  		int absTop  = absoluteTop();
  		int ch      = clientHeight();
  		// the start for whole repaint
  		int start    = dy_ / rowHeight();
  		// the offset for the repaint expose request
  		int startOff = std::max((repaintRect.top() - absTop) / rowHeight(),(long)0);
  		// the start
  		start        = std::min(start + startOff, lineNumber()-1);
			// the endline for whole repaint
			int end     = (dy_ + ch) / rowHeight();
			// the offset for the repaint expose request
			int endOff  = std::max((ch-(repaintRect.top()-absTop + repaintRect.height())) / rowHeight(), (long)0);
			// the end
			end         = std::min(end - endOff, lineNumber()-1);
			return NPoint(start,end);
		}

		NPoint CustomPatternView::tracksFromRepaint(const NRegion & repaintArea) const {
			NRect repaintRect = repaintArea.rectClipBox();

			int absLeft = absoluteLeft();
			int cw      = clientWidth();

			// the start for whole repaint
			int start    = dx_ / colWidth();
			// the offset for the repaint expose request
			int startOff = std::max((repaintRect.left() - absLeft) / colWidth(), (long)0);
			// the start
			start        = std::min(start + startOff, trackNumber()-1);

			// the endtrack for whole repaint
			int end     = (dx_ + cw) / colWidth();
			// the offset for the repaint expose request
			int endOff  = std::max((cw-(repaintRect.left()-absLeft + repaintRect.width())) / colWidth()
                        ,(long)0);
			// the end
			end         = std::min(end - endOff, trackNumber()-1);
			return NPoint(start,end);
		}

		bool CustomPatternView::doSelect() const {
			return doSelect_;
		}

		bool CustomPatternView::doDrag() const {
			return doDrag_;
		}

		void CustomPatternView::clearOldSelection( )
		{
			NSize oldSel = selection_;  
  		selection_.setSize(0,0,0,0);
  		repaintBlock( selection_ );
		}

		const NSize & CustomPatternView::selection() const {
			return selection_;
		}

		void CustomPatternView::startSel(const PatCursor & p)
		{
			selStartPoint_ = p;
			selection_.setSize( p.track(), p.line(), p.track(), p.line() );
  
			oldSelection_ = selection_;
			doDrag_ = true;
			doSelect_ = false;
		}

		void CustomPatternView::endSel( )
		{
			doDrag_ = false;
			doSelect_ = false;
		}

		void CustomPatternView::doSel(const PatCursor & p )
		{
			doSelect_=true;
			if (p.track() < selStartPoint().track()) {
        selection_.setLeft(std::max(p.track(),0)); 
        int startTrack  = dx() / colWidth();
        //if (selection_.left() < startTrack && startTrack > 0) {
        //    pView->hScrBar()->setPos( (startTrack-1)* pView->colWidth());
       // }
    	}
			else
			if (p.track() == selStartPoint_.track()) {
      	selection_.setLeft (std::max(p.track(),0));
				selection_.setRight(std::min(p.track()+1, trackNumber()));
			} else
			if (p.track() > selStartPoint_.track()) {
				selection_.setRight(std::min(p.track()+1, trackNumber()));
				int startTrack  = dx() / colWidth();
				int trackCount  = clientWidth() / colWidth();
			//	if (selection_.right() > startTrack + trackCount) {
		//			pView->hScrBar()->setPos( (startTrack+2) * pView->colWidth());
			//	}// else
			//	if (selection_.right() < startTrack && startTrack > 0) {
			//		pView->hScrBar()->setPos( (startTrack-1)* pView->colWidth());
			//	}
			}
			if (p.line() < selStartPoint_.line()) {
				selection_.setTop(std::max(p.line(),0));
				int startLine  = dy() / rowHeight();
				//if (selection_.top() < startLine && startLine >0) {
				//	pView->vScrBar()->setPos( (startLine-1) * pView->rowHeight());
				//}
			} else
			if (p.line() == selStartPoint_.line()) {
				selection_.setTop (p.line());
				selection_.setBottom(p.line()+1);
			} else
			if (p.line() > selStartPoint_.line()) {
				selection_.setBottom(std::min(p.line()+1, lineNumber()));
				int startLine  = dy() / rowHeight();
				int lineCount  = clientHeight() / rowHeight();
				//if (selection_.bottom() > startLine + lineCount) {
				//	pView->vScrBar()->setPos( (startLine+1) * pView->rowHeight());
				//} else
				//if (selection_.bottom() < startLine && startLine >0) {
				//	pView->vScrBar()->setPos( (startLine-1) * pView->rowHeight());
				//}
			}

			if (oldSelection_ != selection_) {
				// these is totally unoptimized todo repaint only new area
				NSize clipBox = selection_.clipBox(oldSelection_);
				NRect r = repaintTrackArea(clipBox.top(),clipBox.bottom(),clipBox.left(),clipBox.right());
				window()->repaint(this,r);
				oldSelection_ = selection_;
			}
		}

		const PatCursor & CustomPatternView::selStartPoint() const {
			return selStartPoint_;
		}

		PatCursor CustomPatternView::intersectCell( int x, int y ) {
			int track = ( x + dx() ) / colWidth();
			int line  = ( y + dy() ) / rowHeight();
			int colOff   = ( x + dx() ) -  (track*colWidth() - colIdent);

			std::vector<ColumnEvent>::const_iterator it = events_.begin();
			int nr = 0;
			int offset = colIdent;
			int lastOffset = colIdent;
			for ( ; it < events_.end(); it++, nr++ ) {				
				const ColumnEvent & event = *it;				
				switch ( event.type() ) {
					case ColumnEvent::hex2 : offset+= 2*cellWidth(); 	break;
					case ColumnEvent::hex4 : offset+= 4*cellWidth(); 	break;
					case ColumnEvent::note : offset+= noteCellWidth(); break;
					default: ;
				}
				if (offset > colOff) {
					// found our event
					if ( event.type() == ColumnEvent::note ) 
						return PatCursor(track,line,nr,0);
					else {
						int cellStart = colOff - lastOffset;
						int col = cellStart  / cellWidth();
						return PatCursor(track,line,nr,col);
					}			
				}
				lastOffset = offset;
			}
			return PatCursor();
		}

		std::string CustomPatternView::noteToString( int value )
		{
			if (value==255) return "";
			switch (value) {
				case cdefTweakM: return "twk"; break;
				case cdefTweakE: return "twf"; break;
				case cdefMIDICC: return "mcm"; break;
				case cdefTweakS: return "tws"; break;
				case 120       : return "off"; break;
				case 255       : return "";    break;
			}

			int octave = value / 12;

			switch (value % 12) {
      	case 0:   return "C-" + stringify(octave); break;
				case 1:   return "C#" + stringify(octave); break;
				case 2:   return "D-" + stringify(octave); break;
				case 3:   return "D#" + stringify(octave); break;
				case 4:   return "E-" + stringify(octave); break;
				case 5:   return "F-" + stringify(octave); break;
				case 6:   return "F#" + stringify(octave); break;
				case 7:   return "G-" + stringify(octave); break;
				case 8:   return "G#" + stringify(octave); break;
				case 9:   return "A-" + stringify(octave); break;
				case 10:  return "A#" + stringify(octave); break;
				case 11:  return "B-" + stringify(octave); break;
			}
			return "err";
		}


	} // end of host namespace
} // end of psycle namespace

