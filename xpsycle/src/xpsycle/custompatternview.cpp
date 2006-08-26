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

#include <ngrs/nwindow.h>

namespace psycle {
	namespace host	{	

		CustomPatternView::CustomPatternView()
			 : NPanel(), doDrag_(0),doSelect_(0)
		{
			init();
		}

		CustomPatternView::~CustomPatternView()
		{
		}
	
		void CustomPatternView::init() {			
			dx_ = 0;
			dy_ = 0;
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

			g->setForeground(NColor(0,0,80));
			int endTop     = lineNumber() * rowHeight() - dy();
			int endHeight  = std::max(0, clientHeight() - endTop);
			g->fillRect(0,endTop,clientWidth(),endHeight);

			int endLeft     = trackNumber() * colWidth() - dx();
			int endWidth    = std::max(0, clientWidth() - endLeft);
			g->fillRect(endLeft,0,endWidth,clientHeight());

		}

		void CustomPatternView::customPaint( NGraphics* g, int startLine, int endLine, int startTrack, int endTrack ) {

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

	} // end of host namespace
} // end of psycle namespace

