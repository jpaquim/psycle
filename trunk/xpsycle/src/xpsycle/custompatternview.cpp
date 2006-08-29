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
		}

		int CustomPatternView::lineNumber() const {
			return 100;
		}

		int CustomPatternView::trackNumber() const {
			return 16;
		}
	
		int CustomPatternView::colWidth( ) const
		{
			return 100;
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

			for (int i = startTrack; i <= endTrack; i++) // 3px space at begin of trackCol
      g->fillRect(i*colWidth()-dx(),0,3,lineHeight);

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
					g->drawLine(x*colWidth()+col-dx(),0,x*colWidth()+col-dx(),lineHeight);
				}
			}
		}

		void CustomPatternView::drawPattern(NGraphics* g, int startLine, int endLine, int startTrack, int endTrack) {

		}

		void CustomPatternView::drawDataN(NGraphics* g, int track, int line, int eventnr, int data ) {
			if ( eventnr < events_.size() ) {
				const ColumnEvent & event = events_.at( eventnr );
				switch ( event.type() ) {
					case ColumnEvent::hex2 :
						drawBlockData( g, track, line, eventOffset(eventnr), toHex(data,2) );
					break;
					case ColumnEvent::hex4 :
						drawBlockData( g, track, line, eventOffset(eventnr), toHex(data,4) );
					break;
					case ColumnEvent::note :					
						drawStringData( g, track, line, eventOffset(eventnr), noteToString(data) );
					break;
					default: ;
				}
			}
		}

		void CustomPatternView::drawBlockData( NGraphics * g, int track, int line, int eventOffset, const std::string & text )
		{
			int xOff = track * colWidth()+3 + eventOffset - dx();
			int yOff = line  * rowHeight() + rowHeight()  - dy();

			int col = 0;
			for (int i = 0; i < text.length(); i++) {
				g->drawText(xOff + col,yOff,text.substr(i,1));
				col += cellWidth();
			}
		}

		void CustomPatternView::drawStringData(NGraphics* g, int track, int line, int eventOffset, const std::string & text )
		{
			int xOff = track * colWidth()+3 + eventOffset - dx();
			int yOff = line  * rowHeight() + rowHeight()  - dy();

			g->drawText(xOff,yOff,text);
		}

		int CustomPatternView::eventOffset( int eventnr ) const {
			std::vector<ColumnEvent>::const_iterator it = events_.begin();
			int nr = 0;
			int offset = 0;
			for ( ; it < events_.end(); it++, nr++ ) {
				const ColumnEvent & event = *it;
				if (nr == eventnr) {
					return offset;
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
    		NPoint3D p = intersectCell(x,y);
    		startSel(p);
  		}
		}

		void CustomPatternView::onMousePressed(int x, int y, int button) {
			if (button == 1) {
				endSel(); 
			}
		}

		void CustomPatternView::onMouseOver	(int x, int y) {
			if (doDrag_) {
    		NPoint3D p = intersectCell(x,y);
    		doSel(p);
  		}
		}

		void CustomPatternView::onKeyPress(const NKeyEvent & event) {

		}

		void CustomPatternView::onKeyRelease(const NKeyEvent & event) {
			if ( event.scancode() == XK_Shift_L || event.scancode() == XK_Shift_R ) {
				endSel();
			}
		}
		
		void CustomPatternView::repaintBlock( const NSize & block ) {
  		window()->repaint(this,repaintTrackArea(block.top(),block.bottom(),block.left(), block.right()));
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

		void CustomPatternView::startSel(const NPoint3D & p)
		{
			selStartPoint_ = p;
			selection_.setSize( p.x(), p.y(), p.x(), p.y() );
  
			oldSelection_ = selection_;
			doDrag_ = true;
			doSelect_ = false;
		}

		void CustomPatternView::endSel( )
		{
			doDrag_ = false;
			doSelect_ = false;
		}

		void CustomPatternView::doSel(const NPoint3D & p )
		{
			doSelect_=true;
			if (p.x() < selStartPoint().x()) {
        selection_.setLeft(std::max(p.x(),0)); 
        int startTrack  = dx() / colWidth();
        //if (selection_.left() < startTrack && startTrack > 0) {
        //    pView->hScrBar()->setPos( (startTrack-1)* pView->colWidth());
       // }
    	}
			else
			if (p.x() == selStartPoint_.x()) {
      	selection_.setLeft (std::max(p.x(),0));
				selection_.setRight(std::min(p.x()+1, trackNumber()));
			} else
			if (p.x() > selStartPoint_.x()) {
				selection_.setRight(std::min(p.x()+1, trackNumber()));
				int startTrack  = dx() / colWidth();
				int trackCount  = clientWidth() / colWidth();
			//	if (selection_.right() > startTrack + trackCount) {
		//			pView->hScrBar()->setPos( (startTrack+2) * pView->colWidth());
			//	}// else
			//	if (selection_.right() < startTrack && startTrack > 0) {
			//		pView->hScrBar()->setPos( (startTrack-1)* pView->colWidth());
			//	}
			}
			if (p.y() < selStartPoint_.y()) {
				selection_.setTop(std::max(p.y(),0));
				int startLine  = dy() / rowHeight();
				//if (selection_.top() < startLine && startLine >0) {
				//	pView->vScrBar()->setPos( (startLine-1) * pView->rowHeight());
				//}
			} else
			if (p.y() == selStartPoint_.y()) {
				selection_.setTop (p.y());
				selection_.setBottom(p.y()+1);
			} else
			if (p.y() > selStartPoint_.y()) {
				selection_.setBottom(std::min(p.y()+1, lineNumber()));
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

		const NPoint3D & CustomPatternView::selStartPoint() const {
			return selStartPoint_;
		}

		NPoint3D CustomPatternView::intersectCell( int x, int y ){
			return NPoint3D(0,0,0);
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

