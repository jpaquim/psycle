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
#include "configuration.h"
#include "global.h"
#include "custompatternview.h"
#include "inputhandler.h"

#include <ngrs/nwindow.h>
#include <ngrs/nfontmetrics.h>
#include <ngrs/nstatusmodel.h>
#include <iostream>
#include <iomanip>

#ifdef _MSC_VER
#undef min 
#undef max
#endif

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


    ColumnEvent::ColumnEvent( ColumnEvent::ColType type ) {
      type_ = type;
    }

    ColumnEvent::~ColumnEvent() {
    }

    ColumnEvent::ColType ColumnEvent::type() const {
      return type_;
    }

    int ColumnEvent::cols() const {
      int cols_ = 1;
      switch ( type_ ) {
                case ColumnEvent::hex2 : 
                  cols_ = 2; 
                  break;
                case ColumnEvent::hex4 : 
                  cols_ = 4; 
                  break;
                case ColumnEvent::note : 
                  cols_ = 1; 
                  break;
                default: ;
      }
      return cols_;
    }		
    // end of ColumnEvent




    //
    // start of PatCursor class
    //
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


    TrackGeometry::TrackGeometry( ) :
    pView( 0 ),
      left_(0),
      width_(0),
      visibleColumns_(0),
      visible_(1)
    {

    }

    TrackGeometry::TrackGeometry( CustomPatternView & patternView ) :
    pView( &patternView ),
      left_(0),
      width_(0),
      visibleColumns_(0),
      visible_(1)
    {

    }

    TrackGeometry::~TrackGeometry() {

    }

    void TrackGeometry::setLeft( int left ) {
      left_ = left;
    }

    int TrackGeometry::left() const {
      return left_;
    }

    int TrackGeometry::width() const {
      return pView->visibleColWidth( visibleColumns() );
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


    // start of CustomPatternView
    CustomPatternView::CustomPatternView()
      : ngrs::NPanel(), doDrag_(0), doSelect_(0)
    {
      init();
    }

    CustomPatternView::~CustomPatternView( )
    {
    }

    void CustomPatternView::addEvent( const ColumnEvent & event ) {
      events_.push_back( event );
    }

    void CustomPatternView::init() {			
      dx_ = 0;
      dy_ = 0;
      trackNumber_ = 0;
      defaultSize_ = 0;
      separatorColor_.setHCOLORREF(0x00400000);
      selectionColor_ = ngrs::NColor(0,0,255);
      cursorColor_.setHCOLORREF(0x000000e0);
      patternStep_ = 1;
      colIdent = 3;
      trackMinWidth_ = 20;
      lineGridEnabled_ = true;
      colGridEnabled_ = true;
      textColor_ = ngrs::NColor( 0, 0, 0 );
      defaultNoteStr_ = "---";
      trackLeftIdent_ = 10;
      trackRightIdent_ = 20;
    }

    int CustomPatternView::lineNumber() const {
      return 100;
    }

    void CustomPatternView::setTrackNumber( int number ) {
      for ( int newTrack = trackNumber_; newTrack < number; newTrack++ ) {
        TrackGeometry trackGeometry( *this );
        trackGeometry.setVisibleColumns( defaultSize_ );
        trackGeometryMap[ newTrack ] = trackGeometry;
      }

      std::map<int, TrackGeometry>::iterator it;
      it = trackGeometryMap.lower_bound( number );
      while ( it != trackGeometryMap.end() ) {
        trackGeometryMap.erase( it++ );
      }			
      trackNumber_ = number;

      alignTracks();
    }

    int CustomPatternView::trackNumber() const {
      return trackNumber_;
    }

    int CustomPatternView::colWidth( ) const
    {
      std::vector<ColumnEvent>::const_iterator it = events_.begin();
      int offset = 0;
      for ( ; it < events_.end(); it++ ) {
        const ColumnEvent & event = *it;

        switch ( event.type() ) {
                    case ColumnEvent::hex2 : offset+= 2*cellWidth(); 	break;
                    case ColumnEvent::hex4 : offset+= 4*cellWidth(); 	break;
                    case ColumnEvent::note : offset+= noteCellWidth();  break;
                    default: ;
        }
      }

      return offset + colIdent + trackLeftIdent() ;
    }

    int CustomPatternView::visibleColWidth( int maxEvents ) const {
      std::vector<ColumnEvent>::const_iterator it = events_.begin();
      int offset = 0;
      int eventCount = 0;
      for ( ; it < events_.end() && eventCount < maxEvents; it++, eventCount++ ) {
        const ColumnEvent & event = *it;

        switch ( event.type() ) {
                    case ColumnEvent::hex2 : offset+= 2*cellWidth(); 	break;
                    case ColumnEvent::hex4 : offset+= 4*cellWidth(); 	break;
                    case ColumnEvent::note : offset+= noteCellWidth(); break;
                    default: ;
        }
      }

      return offset + colIdent + trackLeftIdent() + trackRightIdent();
    }

    int CustomPatternView::tracksWidth() const {
      std::map<int, TrackGeometry>::const_iterator it = trackGeometryMap.begin();
      int offset = 0;
      for ( ; it != trackGeometryMap.end(); it++ ) {
        const TrackGeometry & geometry = it->second;
        offset+= geometry.width();		
      }
      return offset;
    }

    const std::map<int, TrackGeometry> & CustomPatternView::trackGeometrics() const {
      return trackGeometryMap;
    }

    int CustomPatternView::rowHeight() const {
      return 12;
    }

    int CustomPatternView::beatZoom() const {
      return 4;
    }

    void CustomPatternView::setSeparatorColor( const ngrs::NColor & color ) {
      separatorColor_ = color;
    }

    const ngrs::NColor & CustomPatternView::separatorColor() const {
      return separatorColor_;
    }

    void CustomPatternView::setSelectionColor( const ngrs::NColor & selColor ) {
      selectionColor_ = selColor;
    }

    const ngrs::NColor & CustomPatternView::selectionColor() const {
      return selectionColor_;
    }

    void CustomPatternView::setCursorColor( const ngrs::NColor & cursorColor )
    {
      cursorColor_ = cursorColor;
    }

    void CustomPatternView::setRestAreaColor( const ngrs::NColor & color ) {
      restAreaColor_ = color;
    }

    const ngrs::NColor & CustomPatternView::restArea() const {
      return restAreaColor_;
    }

    const ngrs::NColor & CustomPatternView::cursorColor() const {
      return cursorColor_;
    }

    void CustomPatternView::setBarColor( const ngrs::NColor & barColor ) {
      barColor_ = barColor;
    }

    const ngrs::NColor & CustomPatternView::barColor() const {
      return barColor_;
    }

    void CustomPatternView::setBeatColor( const ngrs::NColor & beatColor ) {
      beatColor_ = beatColor;
    }

    const ngrs::NColor & CustomPatternView::beatColor() const {
      return beatColor_;
    }

    void CustomPatternView::setPlayBarColor( const ngrs::NColor & playBarColor ) {
      playBarColor_ = playBarColor;
    }

    const ngrs::NColor & CustomPatternView::playBarColor() const {
      return playBarColor_;
    }

    void CustomPatternView::setBigTrackSeparatorColor( const ngrs::NColor & color ) {
      bigTrackSeparatorColor_ = color;
    }

    const ngrs::NColor & CustomPatternView::bigTrackSeparatorColor() const {
      return bigTrackSeparatorColor_;
    }

    void CustomPatternView::setSmallTrackSeparatorColor( const ngrs::NColor & color ) {
      smallTrackSeparatorColor_ = color;
    }

    const ngrs::NColor & CustomPatternView::smallTrackSeparatorColor() const {
      return smallTrackSeparatorColor_;
    }

    void CustomPatternView::setLineSeparatorColor( const ngrs::NColor & color ) {
      lineSepColor_ = color;
    }

    const ngrs::NColor & CustomPatternView::lineSeparatorColor() const {
      return lineSepColor_;
    }

    void CustomPatternView::setLineGridEnabled( bool on ) {
      lineGridEnabled_ = on;
    }

    bool CustomPatternView::lineGridEnabled() const {
      return lineGridEnabled_;
    }

    void CustomPatternView::setColGridEnabled( bool on ) {
      colGridEnabled_ = on;
    }

    bool CustomPatternView::colGridEnabled() const {
      return colGridEnabled_;
    }

    void CustomPatternView::setTextColor( const ngrs::NColor & color ) {
      textColor_ = color;
    }

    const ngrs::NColor & CustomPatternView::textColor() const {
      return textColor_;
    }

    void CustomPatternView::setBeatTextColor( const ngrs::NColor & color ) {
      beatTextColor_ = color;
    }

    const ngrs::NColor & CustomPatternView::beatTextColor() {
      return beatTextColor_;
    }

    void CustomPatternView::setBigTrackSeparatorWidth( int ident ) {
      colIdent = ident;
    }

    int CustomPatternView::bigTrackSeparatorWidth() const {
      return colIdent;
    }


    void CustomPatternView::setCursor( const PatCursor & cursor ) {
      cursor_ = cursor;
      updateStatusBar();
    }

    const PatCursor & CustomPatternView::cursor() const {
      return cursor_;
    }

    void CustomPatternView::setPatternStep( int step ) {
      patternStep_ = step;
    }

    int CustomPatternView::patternStep() const {
      return patternStep_;
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

    void CustomPatternView::paint( ngrs::NGraphics* g ) {

      ngrs::NPoint lineArea = linesFromRepaint(g->repaintArea());
      int startLine = lineArea.x();
      int endLine   = lineArea.y();

      ngrs::NPoint trackArea = tracksFromRepaint(g->repaintArea());
      int startTrack = trackArea.x();
      int endTrack   = trackArea.y();

      customPaint(g, startLine, endLine, startTrack, endTrack);

    }

    void CustomPatternView::drawRestArea( ngrs::NGraphics* g, int startLine, int endLine, int startTrack, int endTrack ) {
      g->setForeground( restAreaColor_ );
      int endTop     = lineNumber() * rowHeight() - dy();
      int endHeight  = std::max(0, clientHeight() - endTop);
      g->fillRect(0,endTop,clientWidth(),endHeight);

      std::map<int, TrackGeometry>::const_iterator it;
      it = trackGeometrics().lower_bound( trackNumber()-1 );
      int endLeft = 0;
      if ( it != trackGeometrics().end() )
        endLeft = it->second.left() + it->second.width() - dx();
      int endWidth = std::max(0, clientWidth() - endLeft);
      g->fillRect(endLeft,0,endWidth,clientHeight());
    }

    void CustomPatternView::drawTrackGrid( ngrs::NGraphics*g, int startLine, int endLine, int startTrack, int endTrack  ) {
      std::map<int, TrackGeometry>::const_iterator it;
      it = trackGeometrics().lower_bound( endTrack );
      int trackWidth = 0;
      if ( it != trackGeometrics().end() )
        trackWidth = it->second.left() + it->second.width();
      trackWidth-= dx();

      int lineHeight = ((endLine +1) * rowHeight()) - dy();

      if ( lineGridEnabled() ) {
        g->setForeground( lineSeparatorColor() );
        for (int y = startLine; y <= endLine; y++)
          g->drawLine(0,y* rowHeight() - dy(),trackWidth,y* rowHeight()-dy());
      }

      g->setForeground( bigTrackSeparatorColor() );
      it = trackGeometrics().lower_bound( startTrack );
      for ( ; it != trackGeometrics().end() && it->first <= endTrack; it++) //  oolIdent px space at begin of trackCol{
        g->fillRect( it->second.left() - dx(),0,colIdent,lineHeight);

      g->setForeground( background() );
      it = trackGeometrics().lower_bound( startTrack );
      for ( ; it != trackGeometrics().end() && it->first <= endTrack; it++) {
        // now refill the left and right ident areas
        g->fillRect( it->second.left() - dx() + colIdent,0,trackLeftIdent(),lineHeight);
        // the right
        g->fillRect( it->second.left() + std::max( it->second.width(), trackMinWidth_ ) - trackRightIdent() - dx(),0,trackRightIdent(),lineHeight);
      }

      g->setForeground( smallTrackSeparatorColor() );
      it = trackGeometrics().lower_bound( startTrack );
      for ( ; it != trackGeometrics().end() && it->first <= endTrack; it++) // track small separators
        g->drawLine( it->second.left()-dx(),0, it->second.left()-dx(),lineHeight);

      g->setForeground( foreground( ) );

    }

    int CustomPatternView::noteCellWidth( ) const {            	
      return cellWidth() * 3;
    }

    int CustomPatternView::cellWidth( ) const {
      ngrs::NFontMetrics metrics( font( ) );
      return metrics.maxCharWidth( );
    }


    void CustomPatternView::setTrackLeftIdent( int ident ) {
      trackLeftIdent_ = ident;
    }

    int CustomPatternView::trackLeftIdent() const {
      return trackLeftIdent_;
    }

    void CustomPatternView::setTrackRightIdent( int ident ) {
      trackRightIdent_ = ident;
    }

    int CustomPatternView::trackRightIdent() const {
      return trackRightIdent_;
    }

    void CustomPatternView::drawColumnGrid( ngrs::NGraphics*g, int startLine, int endLine, int startTrack, int endTrack ) {
      if ( events_.size() == 0 || !colGridEnabled() ) return;

      g->setForeground( separatorColor() );
      int lineHeight = ((endLine +1) * rowHeight()) - dy();

      std::map<int, TrackGeometry>::const_iterator it;
      it = trackGeometrics().lower_bound( startTrack );

      for ( ; it != trackGeometrics().end() && it->first <= endTrack; it++) {
        const TrackGeometry & trackGeometry = it->second;
        std::vector<ColumnEvent>::iterator it = events_.begin();
        int col = 0;
        int eventCount = 0;
        for ( ; it < events_.end()-1 && eventCount < trackGeometry.visibleColumns() ; it++, eventCount++ ) {
          ColumnEvent & event = *it;
          switch ( event.type() ) {
                        case ColumnEvent::hex2 : col+= 2*cellWidth(); 	break;
                        case ColumnEvent::hex4 : col+= 4*cellWidth(); 	break;
                        case ColumnEvent::note : col+= noteCellWidth(); break;
          }
          g->drawLine(trackGeometry.left()+colIdent+trackLeftIdent()+col-dx(),0,trackGeometry.left()+colIdent+trackLeftIdent()+col-dx(),lineHeight);
        }
      }
    }

    void CustomPatternView::drawPattern( ngrs::NGraphics* g, int startLine, int endLine, int startTrack, int endTrack ) {

    }

    void CustomPatternView::drawData( ngrs::NGraphics* g, int track, int line, int eventnr, int data, bool sharp, const ngrs::NColor & color ) {

      std::map<int, TrackGeometry>::const_iterator it;
      it = trackGeometrics().lower_bound( track );
      if ( it == trackGeometrics().end() || eventnr >= it->second.visibleColumns()  ) return;

      int xOff = it->second.left() + colIdent + trackLeftIdent() - dx();			

      if ( eventnr < events_.size() ) {
        const ColumnEvent & event = events_.at( eventnr );
        switch ( event.type() ) {
                    case ColumnEvent::hex2 :
                      drawBlockData( g, xOff + eventOffset(eventnr,0), line, toHex(data,2), color );
                      // check if cursor is on event and draw digit in cursortextColor
                      if ( cursor().track() == track && cursor().line() == line && 
                        cursor().eventNr() == eventnr && cursor().col() < 2 ) {
                          drawBlockData( g, xOff + eventOffset(eventnr,0) + cursor().col()*cellWidth() , line, toHex(data,2).substr(cursor().col(),1) ,cursorTextColor_ );
                      }
                      break;
                    case ColumnEvent::hex4 :
                      drawBlockData( g, xOff + eventOffset(eventnr,0), line, toHex(data,4), color );
                      // check if cursor is on event and draw digit in cursortextColor
                      if ( cursor().track() == track && cursor().line() == line && 
                        cursor().eventNr() == eventnr && cursor().col() < 4 ) {
                          drawBlockData( g, xOff + eventOffset(eventnr,0) + cursor().col()*cellWidth(), line, toHex(data,4).substr(cursor().col(),1) ,cursorTextColor_ );
                      }

                      break;
                    case ColumnEvent::note :					
                      if ( cursor().track() == track && cursor().line() == line && 
                        cursor().eventNr() == eventnr ) {
                          drawStringData( g, xOff + eventOffset(eventnr,0), line, noteToString(data, sharp),cursorTextColor_ );
                      } else
                        drawStringData( g, xOff + eventOffset(eventnr,0), line, noteToString(data, sharp),color );
                      break;
                    default: ;
        }
      }
    }

    void CustomPatternView::drawString( ngrs::NGraphics* g, int track, int line, int eventnr, const std::string & data , const ngrs::NColor & color ) {
      std::map<int, TrackGeometry>::const_iterator it;
      it = trackGeometrics().lower_bound( track );
      if ( it == trackGeometrics().end() || eventnr >= it->second.visibleColumns()  ) return;

      int xOff = it->second.left() + colIdent + trackLeftIdent() - dx();

      drawStringData( g, xOff + eventOffset(eventnr,0), line, data, color );
    }

    void CustomPatternView::drawBlockData( ngrs::NGraphics * g, int xOff, int line, const std::string & text, const ngrs::NColor & color)
    {					
      int yp = ( rowHeight() - g->textHeight()) / 2  + g->textAscent();
      int yOff = line  * rowHeight() + yp  - dy();
      int col = 0;
      for (int i = 0; i < text.length(); i++) {
        g->drawText(xOff + col,yOff,text.substr(i,1), color);
        col += cellWidth();
      }
    }

    void CustomPatternView::drawStringData( ngrs::NGraphics* g, int xOff, int line, const std::string & text, const ngrs::NColor & color )
    {
      int yp = ( rowHeight() - g->textHeight()) / 2  + g->textAscent();
      int yOff = line  * rowHeight() + yp  - dy();

      g->drawText(xOff,yOff,text, color );
    }

    void CustomPatternView::drawCellBg( ngrs::NGraphics* g, const PatCursor& cursor ) {
      std::map<int, TrackGeometry>::const_iterator it;
      it = trackGeometrics().lower_bound( cursor.track() );
      if ( it == trackGeometrics().end() ) return;

      int xOff = it->second.left() + colIdent + trackLeftIdent() - dx();
      int yOff = cursor.line()  * rowHeight()  - dy();
      int colOffset = eventOffset( cursor.eventNr(), cursor.col() );
      g->setForeground( cursorColor_ );
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

    void CustomPatternView::customPaint( ngrs::NGraphics* g, int startLine, int endLine, int startTrack, int endTrack ) {
      drawTrackGrid(g, startLine, endLine, startTrack, endTrack);			
      drawColumnGrid(g, startLine, endLine, startTrack, endTrack);
      drawPattern(g, startLine, endLine, startTrack, endTrack);
      drawRestArea(g, startLine, endLine, startTrack, endTrack);
      drawSelBg( g, selection() );
    }

    void CustomPatternView::drawSelBg( ngrs::NGraphics* g, const ngrs::NSize & selArea) {			
      int x1Off = xOffByTrack( selArea.left() );
      int y1Off = selArea.top()  * rowHeight() ;

      int x2Off = xOffByTrack( selArea.right() );
      int y2Off = selArea.bottom() * rowHeight();

      g->setForeground( selectionColor() );
      g->fillRect(x1Off - dx(), y1Off -dy(), x2Off-x1Off, y2Off-y1Off);
    }

    void CustomPatternView::onMousePress( int x, int y, int button ) {
      if ( button == 1) {
        clearOldSelection();
        PatCursor p = intersectCell(x,y);
        startSel(p);
      }
    }

    void CustomPatternView::onMousePressed( int x, int y, int button ) {
      if (button == 1) {
        endSel(); 
        if ( !doSelect() ) cursor_ = intersectCell(x,y);
        repaint();    
      }
    }

    void CustomPatternView::onMouseOver( int x, int y ) {
      if (doDrag_) {
        PatCursor p = intersectCell(x,y);
        doSel(p);
      }
    }

    void CustomPatternView::onKeyPress( const ngrs::NKeyEvent & event ) {

      int key = Global::pConfig()->inputHandler().getEnumCodeByKey(Key( event.shift(),event.scancode()));

      // Keybased block selection commands.
      if (key == cdefSelectUp || key == cdefSelectDn || 
        key == cdefSelectLeft || key == cdefSelectRight ||
        key == cdefSelectTop || key == cdefSelectBottom ||
        key == cdefSelectCol) {
          oldSelection_ = selection_;
          PatCursor crs = cursor();
          int newLeft, newRight, newTop, newBottom;
          int newCursorTrack = cursor().track();
          int newCursorLine = cursor().line();
          int newCursorCol = cursor().col();
          switch (key) {		
                                        case cdefSelectUp:
                                          {
                                            if (doingKeybasedSelect()) {
                                              // if above line is not already selected then select it...
                                              if (!lineAlreadySelected(crs.line())) {
                                                // don't set selection out of bounds of grid...
                                                newTop = std::max(oldSelection_.top()-1, 0);
                                                newBottom = oldSelection_.bottom();
                                              } else { // else if it is selected, deselect it...
                                                newTop = oldSelection_.top();
                                                newBottom = oldSelection_.bottom()-1;
                                              }
                                              newLeft = oldSelection_.left(); // left&right stay the same.
                                              newRight = oldSelection_.right();
                                              selection_.setSize(newLeft,newTop,newRight,newBottom); 
                                            } else {
                                              startKeybasedSelection(crs.track(), crs.track()+1,
                                                std::max(0,crs.line()-1),
                                                crs.line()+1);
                                            }
                                            newCursorLine = std::max(0,cursor().line() - 1);
                                          }
                                          break;
                                        case cdefSelectDn:
                                          if (doingKeybasedSelect()) {
                                            // if line beneath is not selected...
                                            if (!lineAlreadySelected(crs.line()+1)) {
                                              // select line beneath.
                                              newTop = oldSelection_.top();
                                              newBottom = std::min(oldSelection_.bottom()+1,lineNumber());
                                            } else { // line beneath is selected...
                                              // deselect line beneath.
                                              newTop = oldSelection_.top()+1;
                                              newBottom = oldSelection_.bottom();
                                            }
                                            newLeft = oldSelection_.left(); // left&right stay the same.
                                            newRight = oldSelection_.right();
                                            selection_.setSize(newLeft,newTop,newRight,newBottom); 

                                          } else {
                                            startKeybasedSelection(crs.track(), crs.track()+1,
                                              crs.line(),
                                              std::min(lineNumber(),crs.line()+2));
                                          }
                                          newCursorLine = std::min(lineNumber()-1,cursor().line() + 1);
                                          break;
                                        case cdefSelectLeft:
                                          {
                                            if (doingKeybasedSelect()) {
                                              // if track to left is not selected...
                                              if (!trackAlreadySelected(crs.track()-1)) {
                                                // select track to left.
                                                newLeft = std::max(0,oldSelection_.left()-1);
                                                newRight = oldSelection_.right();
                                              } else { // track to left is selected...
                                                // deselect current track.
                                                newLeft = oldSelection_.left();
                                                newRight = oldSelection_.right()-1;
                                              }
                                              newTop = oldSelection_.top(); // top&bottom stay the same.
                                              newBottom = oldSelection_.bottom();
                                              selection_.setSize(newLeft,newTop,newRight,newBottom); 

                                            } else { // start a keyboard-based selection. 
                                              startKeybasedSelection(std::max(0,crs.track()-1),
                                                crs.track()+1,
                                                crs.line(), crs.line()+1);
                                            }
                                            newCursorTrack = std::max(0,cursor().track()-1);
                                            newCursorLine = cursor().line(); 
                                            //        newCursorCol = cursor().col()+1;
                                          }
                                          break;
                                        case cdefSelectRight:
                                          {
                                            if (doingKeybasedSelect()) {
                                              // if track to right is not selected...
                                              if (!trackAlreadySelected(crs.track()+1)) {
                                                // select track to right.
                                                newLeft = oldSelection_.left();
                                                newRight = std::min(oldSelection_.right()+1, trackNumber());
                                              } else { // track to right is selected...
                                                // deselect current track.
                                                newLeft = oldSelection_.left()+1;
                                                newRight = oldSelection_.right();
                                              }
                                              newTop = oldSelection_.top(); // top&bottom stay the same.
                                              newBottom = oldSelection_.bottom();
                                              selection_.setSize(newLeft,newTop,newRight,newBottom); 
                                            } else {
                                              startKeybasedSelection(crs.track(), 
                                                std::min(trackNumber(),crs.track()+2),
                                                crs.line(), crs.line()+1);
                                            }
                                            newCursorTrack = std::min(trackNumber()-1,cursor().track()+1);
                                            newCursorLine = cursor().line(); 
                                          }
                                          break;
                                        case cdefSelectTop:
                                          {
                                            if (doingKeybasedSelect()) {
                                              // select all the way to the top. 
                                              // if line above is not selected...
                                              if (!lineAlreadySelected(crs.line()-1)) {
                                                // bottom stays as it is.
                                                newBottom = oldSelection_.bottom();
                                              } else { // line above is selected.
                                                // set bottom to old selection top.
                                                newBottom = oldSelection_.top()+1;
                                              }
                                              newTop = 0; // top also goes to top.
                                              newLeft = oldSelection_.left(); // left&right stay the same.
                                              newRight = oldSelection_.right();
                                              selection_.setSize(newLeft,newTop,newRight,newBottom); 
                                            } else {
                                              startKeybasedSelection(crs.track(),crs.track()+1,
                                                0, crs.line()+1);
                                            }
                                            newCursorTrack = cursor().track();
                                            newCursorLine = 0;
                                          }
                                          break;
                                        case cdefSelectBottom:
                                          {
                                            if (doingKeybasedSelect()) {
                                              // if line below is not selected...
                                              if (!lineAlreadySelected(crs.line()+1)) {
                                                // top stays the same.
                                                newTop = oldSelection_.top();
                                              } else { // line below is selected.
                                                // top becomes old bottom.
                                                newTop = oldSelection_.bottom()-1;
                                              }
                                              newBottom = lineNumber();
                                              newLeft = oldSelection_.left(); // left&right stay the same.
                                              newRight = oldSelection_.right();
                                              selection_.setSize(newLeft,newTop,newRight,newBottom); 
                                            } else {
                                              startKeybasedSelection(crs.track(), crs.track()+1,
                                                crs.line(), lineNumber());
                                            }
                                            newCursorTrack = cursor().track();
                                            newCursorLine = lineNumber()-1;
                                          }
                                          break;
                                        case cdefSelectCol:
                                          selectColumn(cursor());
                                          break;
          }
          repaintCursorPos(crs);
          repaintCursorPos(cursor()); 
          setCursor(PatCursor(newCursorTrack, newCursorLine, 0, 0));
          if (oldSelection_ != selection_) {
            repaintSelection();
          }
      }

      /*if (doDrag() != (NApp::system().keyState() & ShiftMask) &&
      !(NApp::system().keyState() & ControlMask)) {
      if (!doDrag()) {
      clearOldSelection();
      startSel( cursor() );
      selCursor_ = cursor();
      selCursor_.setEventNr(0);
      selCursor_.setCol(0);
      }
      }*/

      // navigation
      switch (key) {
                case cdefNavPageUp:
                  clearOldSelection();
                  break;
                case cdefNavPageDn:				
                  clearOldSelection();
                  break;
                case cdefNavTop: 
                  {
                    PatCursor oldCursor = cursor();
                    cursor_.setLine( 0 );
                    repaintCursorPos( oldCursor );
                    repaintCursorPos( cursor() ); 
                  }
                  break;
                case cdefNavBottom:
                  {
                    PatCursor oldCursor = cursor();
                    cursor_.setLine( lineNumber() -1 );
                    repaintCursorPos(oldCursor);
                    repaintCursorPos( cursor() ); 
                  }
                  break;
                case cdefColumnNext:
                  if ( cursor().track()+1 < trackNumber() ) {
                    PatCursor oldCursor = cursor();
                    setCursor( PatCursor( cursor().track()+1, cursor().line(),0,0 ) );
                    repaintCursorPos(oldCursor);
                    repaintCursorPos( cursor() ); 
                  }
                  break;
                case cdefNavFirstTrack:
                  {
                    PatCursor oldCursor = cursor();
                    cursor_.setTrack(0);
                    repaintCursorPos(oldCursor);
                    repaintCursorPos( cursor() ); 
                  }
                  break;
                case cdefNavLastTrack:
                  {
                    PatCursor oldCursor = cursor();
                    cursor_.setTrack(trackNumber()-1);
                    repaintCursorPos(oldCursor);
                    repaintCursorPos( cursor() ); 
                  }
                  break;
                case cdefColumnPrev: // todo ngrs nk code
                  if ( cursor().track() > 0 ) {
                    PatCursor oldCursor = cursor();
                    setCursor( PatCursor( cursor().track()-1, cursor().line(),0,0 ) );
                    repaintCursorPos(oldCursor);
                    repaintCursorPos( cursor() ); 
                  }
                  break;
                case cdefNavLeft:
                  moveCursor(-1,0);
                  clearOldSelection();              
                  break;
                case cdefNavRight:
                  moveCursor(1,0);
                  clearOldSelection();                    
                  break;
                case cdefNavUp:
                  if ( cursor().line() - patternStep() >= 0 ) {
                    moveCursor(0, -patternStep() );
                  } else {
                    moveCursor(0, -cursor().line() );
                  }
                  clearOldSelection();
                  break;
                case cdefNavDn:
                  if ( cursor().line()+patternStep() < lineNumber() ) {
                    moveCursor( 0, patternStep() );
                  } else {
                    moveCursor( 0, lineNumber()-1 - cursor().line() );
                  }
                  clearOldSelection();
                  break;
      }

    }

    // Start a new block selection using the keyboard.
    void CustomPatternView::startKeybasedSelection(int leftPos, int rightPos, int topPos, int bottomPos) {
      PatCursor crs = cursor();
      selStartPoint_ = crs;
      selCursor_ = crs;
      doingKeybasedSelect_ = true;
      selection_.setSize(leftPos, topPos, rightPos, bottomPos);
    }

    void CustomPatternView::repaintSelection() {
      // these is totally unoptimized todo repaint only new area
      ngrs::NSize clipBox = selection_.clipBox(oldSelection_);
      ngrs::NRect r = repaintTrackArea(clipBox.top(),clipBox.bottom(),clipBox.left(),clipBox.right());
      window()->repaint(this,r);
      oldSelection_ = selection_;
    }

    bool CustomPatternView::lineAlreadySelected(int lineNumber) {
      if (lineNumber > selection_.top() && lineNumber < selection_.bottom()) {
        return true;
      } else {
        return false;
      }
    }

    bool CustomPatternView::trackAlreadySelected(int trackNumber) {
      if (trackNumber >= selection_.left() && trackNumber < selection_.right()) {
        return true;
      } else {
        return false;
      }
    }

    void CustomPatternView::moveCursor( int dx, int dy) {
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
            if (eventnr + 1 < visibleEvents( cursor_.track()) ) {
              cursor_.setCol( 0 );
              cursor_.setEventNr( eventnr + 1);
            } else 
              if (cursor_.track()+1 < trackNumber() ) {
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
                cursor_.setEventNr( visibleEvents( cursor_.track() -1 )-1 );
                const ColumnEvent & event = events_.at( cursor_.eventNr() );
                cursor_.setCol( event.cols() - 1 );
              }		
            }
            window()->repaint(this,repaintTrackArea( cursor_.line(), cursor_.line(), cursor_.track(), oldCursor.track()) );
        }

        if ( dy != 0 && (dy + cursor_.line() >= 0) ) {
          cursor_.setLine( std::min(cursor_.line() + dy, lineNumber()-1));
          window()->repaint(this,repaintTrackArea( oldCursor.line(), oldCursor.line(), oldCursor.track(), oldCursor.track()) );
          window()->repaint(this,repaintTrackArea( cursor_.line(), cursor_.line(), cursor_.track(), cursor_.track()) );
        } else if (dy!=0) {
          window()->repaint(this,repaintTrackArea( cursor_.line(), cursor_.line(), cursor_.track(), cursor_.track()) );
        }
        updateStatusBar();
    }

    void CustomPatternView::onKeyRelease( const ngrs::NKeyEvent& event ) {
      if ( event.shift() & ngrs::nsShift ) {
        endSel();
      }
    }

    char hex_value(char c) { 
      if(c >= 'A') return 10 + c - 'A'; else return c - '0'; 
    }

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

    void CustomPatternView::repaintBlock( const ngrs::NSize & block ) {
      window()->repaint(this,repaintTrackArea(block.top(),block.bottom(),block.left(), block.right()));
    }

    void CustomPatternView::repaintCursorPos( const PatCursor & cursor ) {
      window()->repaint(this,repaintTrackArea(cursor.line(),cursor.line(),cursor.track(), cursor.track()));
    }

    ngrs::NRect CustomPatternView::repaintTrackArea(int startLine,int endLine,int startTrack, int endTrack) const {
      int top    = startLine    * rowHeight()  + absoluteTop()  - dy_;
      int bottom = (endLine+1)  * rowHeight()  + absoluteTop()  - dy_;
      int left   = xOffByTrack( startTrack)  + absoluteLeft() - dx_;
      int right  = xEndByTrack( endTrack  )  + absoluteLeft() - dx_;

      return ngrs::NRect(left,top,right - left,bottom - top);
    }

    ngrs::NPoint CustomPatternView::linesFromRepaint( const ngrs::NRegion& repaintArea ) const {
      ngrs::NRect repaintRect = repaintArea.rectClipBox();
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
      return ngrs::NPoint(start,end);
    }

    ngrs::NPoint CustomPatternView::tracksFromRepaint( const ngrs::NRegion& repaintArea ) const {
      ngrs::NRect repaintRect = repaintArea.rectClipBox();

      int absLeft = absoluteLeft();
      int cw      = clientWidth();

      // the start for whole repaint
      int start    =  std::max( 0, findTrackByScreenX( dx_ ) );
      // the offset for the repaint expose request
      int startOff = std::max((repaintRect.left() - absLeft) / colWidth(), (long)0);
      // the start
      start        = std::min(start + startOff, trackNumber()-1);

      // the endtrack for whole repaint
      int end     = findTrackByScreenX( dx_ + cw );
      if ( end == -1 ) end = trackNumber()-1;

      // the offset for the repaint expose request
      int endOff  = std::max((cw-(repaintRect.left()-absLeft + repaintRect.width())) / colWidth()
        ,(long)0);
      // the end
      end         = std::min(end - endOff, trackNumber()-1);
      return ngrs::NPoint(start,end);
    }

    bool CustomPatternView::doSelect() const {
      return doSelect_;
    }

    bool CustomPatternView::doDrag() const {
      return doDrag_;
    }

    bool CustomPatternView::doingKeybasedSelect() const {
      return doingKeybasedSelect_;
    }

    void CustomPatternView::clearOldSelection( )
    {
      ngrs::NSize oldSel = selection_;  
      selection_.setSize(0,0,0,0);
      repaintBlock( oldSel );
      doingKeybasedSelect_ = false;
    }

    const ngrs::NSize & CustomPatternView::selection() const {
      return selection_;
    }

    const PatCursor & CustomPatternView::selCursor() const {
      return selCursor_;
    }

    void CustomPatternView::startSel( const PatCursor & p )
    {
      selStartPoint_ = p;
      selCursor_ = p;
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

    void CustomPatternView::selectAll( const PatCursor & cursor ) {
      std::cout << "select all" << std::endl;
      doingKeybasedSelect_=true;
      // FIXME: selects but doesn't repaint properly, not sure why...
      // (compare with trackNumber()-1, repaints fine)
      selection_.setSize( 0, 0, trackNumber(), lineNumber() );

      //                if (oldSelection_ != selection_) {
      repaintSelection();
      //                }

      selCursor_ = cursor;
    }

    void CustomPatternView::selectColumn( const PatCursor& cursor ) {
      startKeybasedSelection(cursor.track(), cursor.track()+1, 0, lineNumber());
    }

    int CustomPatternView::doSel( const PatCursor& p )
    {
      int dir = nodir;
      doSelect_=true;
      if (p.track() < selStartPoint().track()) {
        selection_.setLeft(std::max(p.track(),0)); 
        dir = west;
      }
      else
        if (p.track() == selStartPoint_.track()) {
          selection_.setLeft (std::max(p.track(),0));
          selection_.setRight(std::min(p.track()+1, trackNumber()));
        } else
          if (p.track() > selStartPoint_.track()) {
            selection_.setRight(std::min(p.track()+1, trackNumber()));
            dir = east;
          }
          if (p.line() < selStartPoint_.line()) {
            selection_.setTop(std::max(p.line(),0));
            selection_.setBottom( std::min(selection_.bottom(), selStartPoint_.line()+1));
            dir |= north;
          } else
            if (p.line() == selStartPoint_.line()) {
              selection_.setTop (p.line());
              selection_.setBottom(p.line()+1);
            } else
              if (p.line() > selStartPoint_.line()) {
                selection_.setBottom(std::min(p.line()+1, lineNumber()));
                selection_.setTop( std::max(selection_.top(), selStartPoint_.line()));
                dir |= south;
              }

              if (oldSelection_ != selection_) {
                repaintSelection();
              }
              selCursor_ = p;
              return dir;
    }

    const PatCursor & CustomPatternView::selStartPoint() const {
      return selStartPoint_;
    }

    PatCursor CustomPatternView::intersectCell( int x, int y ) {
      int track = findTrackByScreenX( x + dx() );
      int line  = ( y + dy() ) / rowHeight();
      int colOff   = ( x + dx() ) -  xOffByTrack(track);

      std::vector<ColumnEvent>::const_iterator it = events_.begin();
      int nr = 0;
      int offset = colIdent + trackLeftIdent();
      int lastOffset = colIdent + trackLeftIdent();
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

    std::string CustomPatternView::noteToString( int value, bool sharp )
    {
      switch (value) {
        case cdefTweakM : return "twk"; break;
        case cdefTweakE : return "twf"; break;
        case cdefMIDICC : return "mcm"; break;
        case cdefTweakS : return "tws"; break;
        case 120        : return "off"; break;
        case 255        : return "---"; break;  // defaultNoteStr_; break;
      }

      int octave = value / 12;

      if (sharp) switch (value % 12) {
        case 0:   return "C-" + stringify( octave ); break;
        case 1:   return "C#" + stringify( octave ); break;
        case 2:   return "D-" + stringify( octave ); break;
        case 3:   return "D#" + stringify( octave ); break;
        case 4:   return "E-" + stringify( octave ); break;
        case 5:   return "F-" + stringify( octave ); break;
        case 6:   return "F#" + stringify( octave ); break;
        case 7:   return "G-" + stringify( octave ); break;
        case 8:   return "G#" + stringify( octave ); break;
        case 9:   return "A-" + stringify( octave ); break;
        case 10:  return "A#" + stringify( octave ); break;
        case 11:  return "B-" + stringify( octave ); break;
      } else
      switch (value % 12) {
        case 0:   return "C-" + stringify(octave); break;
        case 1:   return "Db" + stringify(octave); break;
        case 2:   return "D-" + stringify(octave); break;
        case 3:   return "Eb" + stringify(octave); break;
        case 4:   return "E-" + stringify(octave); break;
        case 5:   return "F-" + stringify(octave); break;
        case 6:   return "Gb" + stringify(octave); break;
        case 7:   return "G-" + stringify(octave); break;
        case 8:   return "Ab" + stringify(octave); break;
        case 9:   return "A-" + stringify(octave); break;
        case 10:  return "Bb" + stringify(octave); break;
        case 11:  return "B-" + stringify(octave); break;
      }
      return "err";
    }

    void CustomPatternView::alignTracks() {
      std::map<int, TrackGeometry>::iterator it = trackGeometryMap.begin();
      int offset = 0;
      for ( ; it != trackGeometryMap.end(); it++ ) {
        TrackGeometry & geometry = it->second;
        geometry.setLeft( offset );
        offset+= std::max( trackMinWidth_, geometry.width() );		
      }
    }

    int CustomPatternView::findTrackByScreenX( int x ) const {
      // todo write a binary search here
      // is used from intersectCell
      std::map<int, TrackGeometry>::const_iterator it = trackGeometryMap.begin();
      int offset = 0;
      for ( ; it != trackGeometryMap.end(); it++ ) {
        const TrackGeometry & geometry = it->second;
        offset+= geometry.width();				
        if ( offset > x ) return it->first;
      }
      return -1; // no track found
    }

    int CustomPatternView::xOffByTrack( int track ) const {
      std::map<int, TrackGeometry>::const_iterator it;
      it = trackGeometrics().lower_bound( track );
      int trackOff = 0;
      if ( it != trackGeometrics().end() )
        trackOff = it->second.left();
      return trackOff;
    }

    int CustomPatternView::xEndByTrack( int track ) const {
      std::map<int, TrackGeometry>::const_iterator it;
      it = trackGeometrics().lower_bound( track );
      int trackOff = 0;
      if ( it != trackGeometrics().end() )
        trackOff = it->second.left() + it->second.width();
      return trackOff;
    }

    int CustomPatternView::trackWidth( int track ) const {
      std::map<int, TrackGeometry>::const_iterator it;
      it = trackGeometrics().lower_bound( track );

      if ( it != trackGeometrics().end() )
        return it->second.width();
      return 0;
    }

    void CustomPatternView::setVisibleEvents( int track, int eventCount ) {
      if ( eventCount <=  events_.size() ) {
        std::map<int, TrackGeometry>::iterator it;
        it = trackGeometryMap.lower_bound( track );

        if ( it != trackGeometryMap.end() ) {
          it->second.setVisibleColumns( eventCount );
        }

        alignTracks();
      }
    }

    int CustomPatternView::visibleEvents( int track ) const {
      std::map<int, TrackGeometry>::const_iterator it;
      it = trackGeometrics().lower_bound( track );

      if ( it != trackGeometrics().end() )
        return it->second.visibleColumns();
      return 0;
    }

    void CustomPatternView::setDefaultVisibleEvents( int defaultSize ) {
      defaultSize_ = defaultSize;
    }

    void CustomPatternView::setTrackMinWidth( int size ) {
      trackMinWidth_ = size;
      alignTracks();
    }

    void CustomPatternView::updateStatusBar() {
      if ( window()->statusModel() ) {
        window()->statusModel()->setText( "Ln " + stringify( cursor().col() ), 2 );
        window()->statusModel()->setText( "Tr " + stringify( cursor().line() ), 3 );
      }
    }

  } // end of host namespace
} // end of psycle namespace

