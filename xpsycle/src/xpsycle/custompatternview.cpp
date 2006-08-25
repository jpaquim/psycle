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

namespace psycle {
	namespace host	{	

		CustomPatternView::CustomPatternView()
			 : NPanel()
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

		}

		void CustomPatternView::onMousePressed(int x, int y, int button) {

		}

		void CustomPatternView::onMouseOver	(int x, int y) {

		}

		void CustomPatternView::onKeyPress(const NKeyEvent & event) {

		}

		void CustomPatternView::onKeyRelease(const NKeyEvent & event) {

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

	} // end of host namespace
} // end of psycle namespace

