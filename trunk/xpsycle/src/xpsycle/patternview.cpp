/***************************************************************************
*   Copyright (C) 2006 by  Stefan   *
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
#include "patternview.h"
#include "configuration.h"
#include "inputhandler.h"
#include "defaultbitmaps.h"
#include "zoombar.h"
#include <psycore/machine.h>
#include <psycore/song.h>
#include <ngrs/app.h>
#include <ngrs/alignlayout.h>
#include <ngrs/window.h>
#include <ngrs/menuitem.h>
#include <ngrs/menuseperator.h>
#include <ngrs/frameborder.h>
#include <ngrs/fontmetrics.h>
#include <ngrs/toolbar.h>
#include <ngrs/combobox.h>
#include <ngrs/item.h>
#include <ngrs/itemevent.h>
#include <ngrs/label.h>
#include <ngrs/system.h>
#include <ngrs/splitbar.h>
#include <ngrs/checkbox.h>

#ifdef _MSC_VER
#undef min 
#undef max
#endif

namespace psy { 
  namespace host {


    template<class T> inline T str_hex(const std::string &  value) {
      T result;

      std::stringstream str;
      str << value;
      str >> std::hex >> result;

      return result;
    }


    /// a undo pattern class, that holds the infos of the last operation change
    UndoPattern::UndoPattern() {

    }

    UndoPattern::UndoPattern( int patternId, const ngrs::Size & changedBlock, const PatCursor & cursor ) {
      changedBlock_ = changedBlock;
      patternId_ = patternId;
      cursor_ = cursor;
    }

    UndoPattern:: ~UndoPattern() {

    }

    int UndoPattern::patternId() {
      return patternId_;
    }

    const ngrs::Size & UndoPattern::changedBlock() const {
      return changedBlock_;
    }

    const PatCursor & UndoPattern::oldCursor() {
      return cursor_;
    }


    // the PatternUndoManager

    PatternUndoManager::PatternUndoManager( ) {
      pattern_ = 0;
      pSong_ = 0;
    }

    PatternUndoManager::~PatternUndoManager() {

    }

    void PatternUndoManager::setSong( psy::core::Song* pSong  ) {
      pSong_ = pSong;
    }

    void PatternUndoManager::setPattern( psy::core::SinglePattern* pattern ) {
      pattern_ = pattern;
    }


    void PatternUndoManager::addUndo( const ngrs::Size & block, const PatCursor & cursor ) {
      if ( pattern_ ) {
        // create a undoPattern
        UndoPattern dstPattern( pattern_->id() , block, cursor );
        psy::core::SinglePattern srcPattern = pattern_->block( block.left(), block.right(), block.top(), block.bottom() );
        dstPattern.copyBlock( 0,0, srcPattern, block.right() - block.left(), srcPattern.beats() );
        push_back( dstPattern );
      }	
    }

    void PatternUndoManager::addUndo( const PatCursor & cursor ) {
      addUndo ( ngrs::Size( cursor.track(), cursor.line(), cursor.track()+1, cursor.line()+1), cursor );
    }

    void PatternUndoManager::doUndo() {
      if ( size() > 0) {
        PatternUndoManager::iterator it = end()-1;

        UndoPattern * lastUndoPattern = 0;

        for ( ; ; it--) {
          lastUndoPattern = & *it;
          if ( lastUndoPattern->patternId() == pattern_->id() ) {
            break;
          }
          if ( it == begin() ) {
            it = end();
            break;
          }
        }

        if ( it != end() ) {
          psy::core::SinglePattern* pattern = pSong_->patternSequence().patternData()->findById( pattern_->id() );
          if ( pattern ) {
            ngrs::Size changedBlock = lastUndoPattern->changedBlock();
            pattern->copyBlock( changedBlock.left(), changedBlock.top(), *lastUndoPattern, changedBlock.right() - changedBlock.left(), lastUndoPattern->beats() );
            erase( it );		
          } else {
            // pattern was deleted
          }
        }
      }
    }


    /// The pattern Main Class , a container for the inner classes LineNumber, Header, and PatternDraw
    PatternView::PatternView( psy::core::Song* song )
      : ngrs::Panel()
    {
      _pSong = song;
      setLayout( ngrs::AlignLayout() );

      hBar = 0;
      vBar = 0;
      tweakHBar = 0;
      tweakGUI = 0;
      header = 0;
      drawArea = 0;
      tweakHeader = 0;
      pattern_ = 0;

      vBar = new ngrs::ScrollBar();
      vBar->setWidth( 15 );
      vBar->setOrientation( ngrs::nVertical );
      vBar->change.connect( this, &PatternView::onVScrollBar );
      add( vBar, ngrs::nAlRight );

      // create the patternview toolbar
      add(toolBar = new ngrs::ToolBar() ,ngrs::nAlTop);
      initToolBar();

      // create the left linenumber panel
      Panel* linePanel = new ngrs::Panel();
      linePanel->setLayout( ngrs::AlignLayout() );
      lineHeaderLabel = new ngrs::Panel();
      linePanel->add( lineHeaderLabel, ngrs::nAlTop );
      linePanel->add( lineNumber_ = new LineNumber(this), ngrs::nAlClient);
      add( linePanel, ngrs::nAlLeft );

      // sub group tweaker and patterndraw
      Panel* drawGroup = new ngrs::Panel();
      drawGroup->setLayout( ngrs::AlignLayout() );
      // create tweak gui
      tweakGroup = new ngrs::Panel();
      tweakGroup->setLayout( ngrs::AlignLayout() );
      tweakGroup->add( tweakHeader = new TweakHeader(this), ngrs::nAlTop);
      tweakHBar = new ngrs::ScrollBar();
      tweakHBar->setOrientation(ngrs::nHorizontal);
      tweakHBar->setPreferredSize(100,15);
      tweakHBar->change.connect(this,&PatternView::onHTweakScrollBar);
      tweakGroup->add( tweakHBar, ngrs::nAlBottom );
      tweakGroup->add( tweakGUI = new TweakGUI(this), ngrs::nAlClient) ;      
      tweakGroup->setMinimumWidth(1);
      tweakGUI->setTrackMinWidth( tweakHeader->skinColWidth() );
      drawGroup->add ( tweakGroup, ngrs::nAlLeft );
      drawGroup->setPreferredSize( 2*tweakGUI->colWidth() , 100);
      // splitbar
      splitBar = new ngrs::SplitBar();
      drawGroup->add( splitBar, ngrs::nAlLeft );
      // create the pattern panel
      Panel* subGroup = new ngrs::Panel();
      subGroup->setLayout( ngrs::AlignLayout() );
      // create the headertrack panel
      subGroup->add(header      = new Header(this), ngrs::nAlTop);
      lineHeaderLabel->setPreferredSize( 20, header->preferredHeight() );
      tweakHeader->setPreferredSize( 20, header->preferredHeight() );
      // hbar with beat zoom
      Panel* hBarPanel = new ngrs::Panel();
      hBarPanel->setLayout( ngrs::AlignLayout() );
      zoomHBar = new ZoomBar();
      zoomHBar->setRange(1,100);
      zoomHBar->setPos(4);
      zoomHBar->posChanged.connect(this, &PatternView::onZoomHBarPosChanged);
      hBarPanel->add(zoomHBar, ngrs::nAlRight);
      hBar = new ngrs::ScrollBar();
      hBar->setOrientation(ngrs::nHorizontal);
      hBar->setPreferredSize(100,15);
      hBar->change.connect(this,&PatternView::onHScrollBar);
      hBarPanel->add(hBar, ngrs::nAlClient);
      subGroup->add(hBarPanel, ngrs::nAlBottom);
      // create the drawArea
      subGroup->add(drawArea    = new PatternDraw(this), ngrs::nAlClient);
      drawGroup->add( subGroup, ngrs::nAlClient );
      add(drawGroup, ngrs::nAlClient);

      drawArea->setFont( ngrs::Font("8x13bold",8, ngrs::nMedium | ngrs::nStraight ));
      lineNumber_->setFont( ngrs::Font("8x13bold",8, ngrs::nMedium | ngrs::nStraight ));
      tweakGUI->setFont( ngrs::Font("8x13bold",8, ngrs::nMedium | ngrs::nStraight ));
      editPosition_ = 0;
      prevEditPosition_ = 0;
      playPos_  = 0;
      outtrack = 0;
      editOctave_ = 4;

      moveCursorWhenPaste_ = false;  
      selectedMacIdx_ = 255;
      undoManager_.setSong( song );

      updateSkin();
    }

    PatternView::~PatternView()
    {
    }

    void PatternView::updateSkin() {

      setColorInfo ( SkinReader::Instance()->patternview_color_info() );
      header->setHeaderCoordInfo( SkinReader::Instance()->headerCoordInfo() );

      drawArea->setLineGridEnabled(SkinReader::Instance()->patview_line_sep_enabled());
      drawArea->setColGridEnabled(SkinReader::Instance()->patview_col_sep_enabled());

      drawArea->setTrackLeftIdent( SkinReader::Instance()->patview_track_left_ident() );
      drawArea->setTrackRightIdent( SkinReader::Instance()->patview_track_right_ident() );
      drawArea->setBigTrackSeparatorWidth( SkinReader::Instance()->patview_track_big_sep_width() );

      tweakGUI->setLineGridEnabled(SkinReader::Instance()->patview_line_sep_enabled());
      tweakGUI->setColGridEnabled(SkinReader::Instance()->patview_col_sep_enabled());

      tweakGUI->setTrackLeftIdent( SkinReader::Instance()->patview_track_left_ident() );
      tweakGUI->setTrackRightIdent( SkinReader::Instance()->patview_track_right_ident() );
      tweakGUI->setBigTrackSeparatorWidth( SkinReader::Instance()->patview_track_big_sep_width() );

      drawArea->alignTracks();
      tweakGUI->alignTracks();
      tweakHeader->setPreferredSize( 20, header->preferredHeight() );
      lineHeaderLabel->setPreferredSize( 20, header->preferredHeight() );
    }

    void PatternView::setFocus() {
      drawArea->setFocus();
    }

    void PatternView::onZoomHBarPosChanged(ZoomBar* zoomBar, double newPos) {
      setBeatZoom( (int) newPos );
      repaint();
    }

    void PatternView::onHScrollBar( ngrs::ScrollBar * sender )
    {
      int newPos = drawArea->xOffByTrack( drawArea->findTrackByScreenX( static_cast<int>( sender->pos() ) ) );

      if (newPos != drawArea->dx()) {
        header->setScrollDx( newPos );
        header->repaint();

        int diffX  = newPos - drawArea->dx();
        if (diffX < drawArea->clientWidth()) {
          ngrs::Rect rect = drawArea->blitMove(diffX,0, drawArea->absoluteSpacingGeometry());
          drawArea->setDx(newPos);
          window()->repaint(drawArea,rect);
        } else {
          drawArea->setDx(newPos);
          drawArea->repaint(drawArea);
        }
      }
    }

    void PatternView::onHTweakScrollBar( ngrs::ScrollBar * sender )
    {
      int newPos = static_cast<int>( ( sender->pos() / tweakGUI->colWidth()) * tweakGUI->colWidth() );

      if (newPos != tweakGUI->dx()) {
        tweakHeader->setScrollDx( newPos );
        tweakHeader->repaint();

        int diffX  = newPos - tweakGUI->dx();
        if (diffX < tweakGUI->clientWidth()) {
          ngrs::Rect rect = tweakGUI->blitMove(diffX,0, tweakGUI->absoluteSpacingGeometry());
          tweakGUI->setDx(newPos);
          window()->repaint(tweakGUI,rect);
        } else {
          tweakGUI->setDx(newPos);
          tweakGUI->repaint();
        }
      }
    }


    void PatternView::onVScrollBar( ngrs::ScrollBar * sender )
    {
      if ( sender->pos() >= 0) {
        int newPos = static_cast<int> ( (sender->pos() / rowHeight() ) * rowHeight() );

        if (newPos != drawArea->dy()) {
          int diffY  = newPos - lineNumber_->dy();
          if (diffY < drawArea->clientHeight()) {
            ngrs::Rect rect = lineNumber_->blitMove(0,diffY,lineNumber_->absoluteSpacingGeometry());
            lineNumber_->setDy(newPos);
            window()->repaint(lineNumber_,rect);

            rect = drawArea->blitMove(0,diffY,drawArea->absoluteSpacingGeometry());
            drawArea->setDy(newPos);
            window()->repaint(drawArea,rect);

            rect = tweakGUI->blitMove(0,diffY,tweakGUI->absoluteSpacingGeometry());
            tweakGUI->setDy(newPos);
            window()->repaint(tweakGUI,rect);

          } else {
            lineNumber_->setDy(newPos);
            drawArea->setDy(newPos);
            lineNumber_->repaint();
            drawArea->repaint();
            tweakGUI->repaint();
          }
        }
      }
    }

    void PatternView::updateRange() {
      // set the Range to the new headerwidth  
      if ( !hBar || !tweakHeader || !header || !drawArea ||! tweakHBar) return;

      hBar->setRange( 0, header->preferredWidth() - drawArea->clientWidth());
      // this is only a hint for the standard colwidth.. not 100 percent correct
      hBar->setSmallChange( drawArea->colWidth() );
      hBar->setLargeChange( drawArea->colWidth() );

      // set the Range to the new headerwidth
      tweakHBar->setRange( 0, tweakHeader->preferredWidth() - tweakGUI->clientWidth());
      tweakHBar->setSmallChange( tweakGUI->colWidth() );
      tweakHBar->setLargeChange( tweakGUI->colWidth() );

      int count = (drawArea->clientHeight()-headerHeight()) / rowHeight();
      vBar->setRange( 0, (lineNumber()-1-count)*rowHeight());
      vBar->setSmallChange( drawArea->rowHeight() ); 
      vBar->setLargeChange( drawArea->rowHeight() );
    }

    void PatternView::initToolBar( )
    {
      meterCbx = new ngrs::ComboBox();
      meterCbx->add(new ngrs::Item("4/4"));
      meterCbx->add(new ngrs::Item("3/4"));
      meterCbx->setPreferredSize( 50, 15 );
      meterCbx->setIndex(0);
      meterCbx->enableFocus(false);
      toolBar->add(meterCbx);
      ngrs::Button* btn = toolBar->add(new ngrs::Button("add Bar"));
      btn->clicked.connect(this,&PatternView::onAddBar);
      btn->enableFocus( false);
      btn = toolBar->add(new ngrs::Button("delete Bar"));
      btn->clicked.connect(this,&PatternView::onDeleteBar);
      btn->enableFocus(false);
      toolBar->add(new ngrs::Label("Pattern Step"));
      patternCombo_ = new ngrs::ComboBox();
      for (int i = 1; i <=16; i++) 
        patternCombo_->add( new ngrs::Item( stringify(i) ) );
      patternCombo_->setIndex( 0 );
      patternCombo_->itemSelected.connect(this,&PatternView::onPatternStepChange);
      patternCombo_->setPreferredSize( 40, 20 );
      patternCombo_->enableFocus(false);
      toolBar->add(patternCombo_);

      toolBar->add(new ngrs::Label("Octave"));
      octaveCombo_ = new ngrs::ComboBox();
      for (int i=0; i<9; i++) octaveCombo_->add(new ngrs::Item(stringify(i)));
      octaveCombo_->itemSelected.connect(this,&PatternView::onOctaveChange);
      octaveCombo_->setPreferredSize( 40, 20 );
      octaveCombo_->setIndex(4);
      octaveCombo_->enableFocus(false);
      setEditOctave(4);
      toolBar->add(octaveCombo_);

      toolBar->add(new ngrs::Label("Tracks"));
      trackCombo_ = new ngrs::ComboBox();
      trackCombo_->setPreferredSize( 40, 20 );
      trackCombo_->itemSelected.connect(this,&PatternView::onTrackChange);
      for( int i=4; i<=64; i++ ) {
        trackCombo_->add(new ngrs::Item(stringify(i)));
      }
//      trackCombo_->setIndex( _pSong->tracks() - 4 );  // starts at 4 .. so 16 - 4 = 12 ^= 16
      toolBar->add(trackCombo_);

      sharpBtn_ = new ngrs::Button("#");
      sharpBtn_->clicked.connect(this,&PatternView::onToggleSharpMode);
      toolBar->add( sharpBtn_ );


      sideBox = new ngrs::NCheckBox("Tweak left/right");
      sideBox->clicked.connect(this,&PatternView::onSideChange);
      toolBar->add(sideBox);
    }

    void PatternView::onAddBar( ngrs::ButtonEvent * ev )
    {
      if ( pattern_ ) {
        std::string meter = meterCbx->text();
        int pos = meter.find("/");
        std::string num   = meter.substr( 0, pos);
        std::string denom = meter.substr( pos+1 );

        pattern_->addBar( psy::core::TimeSignature( ngrs::str<int>(num), ngrs::str<int>(denom)) );
        resize();
        repaint();
      }
    }

    void PatternView::onToggleSharpMode( ngrs::ButtonEvent* ev ) {
      drawArea->setSharpMode( !drawArea->sharpMode() );
      if ( drawArea->sharpMode() ) {
        sharpBtn_->setText("#");
      } else
        sharpBtn_->setText("b");
    }

    void PatternView::onDeleteBar( ngrs::ButtonEvent* ev )
    {
      if ( pattern_ ) {
        float position = drawArea->cursor().line() / (float) pattern_->beatZoom();
        pattern_->removeBar(position);
        resize();
        repaint();
      }
    }

    void PatternView::onSideChange( ngrs::ButtonEvent* ev ) {
      if (sideBox->checked() ) {
        tweakGroup->setAlign(ngrs::nAlRight);
        splitBar->setAlign( ngrs::nAlRight );
      } else {
        tweakGroup->setAlign(ngrs::nAlLeft);
        splitBar->setAlign( ngrs::nAlLeft );
      }
      resize();
      repaint();
    }

    void PatternView::resize( )
    {
      // calls the AlignLayout to reorder scroll-,-header,-linenumber- and patternpanels
      ngrs::Panel::resize();
      updateRange();

    }

    void PatternView::setSeparatorColor( const ngrs::Color & separatorColor )
    {
      separatorColor_ = separatorColor;
    }

    const ngrs::Color & PatternView::separatorColor( )
    {
      return separatorColor_;
    }

    int PatternView::trackNumber( ) const
    {
      return drawArea->trackNumber();
    }

    int PatternView::rowHeight( ) const
    {
      return 13;
    }

    int PatternView::colWidth() const {
      return drawArea->colWidth();
    }

    int PatternView::tweakColWidth() const {
      return tweakGUI->colWidth();
    }

    int PatternView::headerHeight( ) const
    {
      return header->height()-1;
    }

    int PatternView::headerWidth( ) const
    {
      return header->skinColWidth();
    }

    int PatternView::tweakHeaderWidth( ) const
    {
      return tweakHeader->skinColWidth();
    }

    int PatternView::lineNumber( ) const
    {
      return ( pattern_) ? static_cast<int> ( pattern_->beatZoom() * pattern_->beats() ) : 0;  
    }

    int PatternView::playPos( ) const
    {
      return playPos_;
    }

    void PatternView::setPrevEditPosition( int pos )
    {
      prevEditPosition_ = pos;
    }

    int PatternView::prevEditPosition( ) const
    {
      return prevEditPosition_;
    }

    void PatternView::setEditPosition( int pos )
    {
      editPosition_ = pos;
      int count = (drawArea->clientHeight()-headerHeight()) / rowHeight();
      vBar->setRange( 0, (lineNumber()-1-count)*rowHeight());
    }

    int PatternView::editPosition( ) const
    {
      return editPosition_;
    }

    ngrs::ScrollBar * PatternView::vScrBar( )
    {
      return vBar;
    }

    ngrs::ScrollBar * PatternView::hScrBar( )
    {
      return hBar;
    }

    void PatternView::setEditOctave( int octave )
    {
      editOctave_ = octave;
      octaveCombo_->setIndex( octave );
      octaveCombo_->repaint();
    }

    int PatternView::editOctave( ) const
    {
      return editOctave_;
    }

    void PatternView::setMoveCursorWhenPaste( bool on) {
      moveCursorWhenPaste_ = on;
    }

    bool PatternView::moveCursorWhenPaste() const {
      return moveCursorWhenPaste_;
    }

    const std::map<int, TrackGeometry> & PatternView::trackGeometrics() const {
      return drawArea->trackGeometrics();
    }

    /// End of PatternView main Class










    ///
    ///
    /// The Header Panel Class
    ///
    ///
    PatternView::Header::Header( PatternView * pPatternView ) : pView(pPatternView) , ngrs::Panel()
    {
      setHeight( coords_.bgCoords.height() );
      skinColWidth_ = coords_.bgCoords.width();
      setTransparent(false);
    }

    PatternView::Header::~ Header( )
    {
    }

    void PatternView::Header::paint( ngrs::Graphics& g )
    {
      DefaultBitmaps & icons = SkinReader::Instance()->bitmaps();

      ngrs::Bitmap & bitmap = icons.pattern_header_skin();
      ngrs::Bitmap & patNav = icons.patNav();

      int startTrack = pView->drawArea->findTrackByScreenX( scrollDx() );

      g.setForeground(pView->separatorColor());

      std::map<int, TrackGeometry>::const_iterator it;
      it = pView->trackGeometrics().lower_bound( startTrack );

      for ( ; it != pView->trackGeometrics().end() && it->first <= pView->trackNumber() - 1; it++) {
        const TrackGeometry & trackGeometry = it->second;

        const int trackX0 = it->first / 10;
        const int track0X = it->first % 10;
        int xOff = trackGeometry.left();
        if (xOff - scrollDx() > spacingWidth() ) break;
        int center =  ( trackGeometry.width() - patNav.width() - skinColWidth()) / 2;		
        if ( center > -patNav.width() ) {
          int xOffc = xOff + patNav.width()  + center;
          g.putBitmap(xOffc,0,std::min((int) coords_.bgCoords.width(),trackGeometry.width()) ,coords_.bgCoords.height(), bitmap, 
            coords_.bgCoords.left(), coords_.bgCoords.top());
          g.putBitmap(xOffc+coords_.dgX0Coords.x(),0+coords_.dgX0Coords.y(),coords_.noCoords.width(),coords_.noCoords.height(), bitmap,
            trackX0*coords_.noCoords.width(), coords_.noCoords.top());
          g.putBitmap(xOffc+coords_.dg0XCoords.x(),0+coords_.dg0XCoords.y(),coords_.noCoords.width(),coords_.noCoords.height(), bitmap,
            track0X*coords_.noCoords.width(), coords_.noCoords.top());

          // blit the mute LED
//          if (  pView->pSong()->_trackMuted[it->first]) {
//            g.putBitmap(xOffc+coords_.dMuteCoords.x(),0+coords_.dMuteCoords.y(),coords_.sMuteCoords.width(),coords_.sMuteCoords.height(), bitmap,
//              coords_.sMuteCoords.left(), coords_.sMuteCoords.top());
//          }

          // blit the solo LED
//          if ( pView->pSong()->_trackSoloed == it->first) {
//            g.putBitmap(xOffc+coords_.dSoloCoords.x(),0+coords_.dSoloCoords.y(),coords_.sSoloCoords.width(),coords_.sSoloCoords.height(), bitmap,
//              coords_.sSoloCoords.left(), coords_.sSoloCoords.top());
//          }

          // blit the record LED
//          if ( pView->pSong()->_trackArmed[it->first]) {
//            g.putBitmap(xOffc+coords_.dRecCoords.x(),0+coords_.dRecCoords.y(),coords_.sRecCoords.width(),coords_.sRecCoords.height(), bitmap,
//              coords_.sRecCoords.left(), coords_.sRecCoords.top());
//          }
        }
        g.putBitmap(xOff, 0,patNav.width(),patNav.height(), patNav, 
          0, 0);

        if (it->first!=0) g.drawLine( xOff, 0, xOff, clientHeight()); // col seperator
      }
    }

    void PatternView::Header::onMousePress( int x, int y, int button )
    {
      DefaultBitmaps & icons = SkinReader::Instance()->bitmaps();
      // check if left mouse button pressed
      if (button == 1)
      {
        // determine the track column, the mousepress occured on
        int track = pView->drawArea->findTrackByScreenX( x );

        // find out the start offset of the header bitmap
        ngrs::Bitmap & patNav = icons.patNav();
        ngrs::NPoint off( patNav.width()+ pView->drawArea->xOffByTrack( track ) + (pView->drawArea->trackWidth( track ) - patNav.width() - skinColWidth()) / 2,0);
        // find out the start offset of the nav buttons
        ngrs::NPoint navOff( pView->drawArea->xOffByTrack( track ),0); 

        ngrs::Rect decCol( navOff.x() , navOff.y(), 10, patNav.height() );
        ngrs::Rect incCol( navOff.x() +10, navOff.y(), 10, patNav.height() );
        ngrs::Rect xCol( navOff.x() +20, navOff.y(), 10, patNav.height() );

        // the rect area of the solo led
        ngrs::Rect solo(off.x() + coords_.dSoloCoords.x(), off.y() + coords_.dSoloCoords.y(), coords_.sSoloCoords.width(), coords_.sSoloCoords.height());

        std::map<int, TrackGeometry>::const_iterator it;
        it = pView->trackGeometrics().lower_bound( track );

        if (incCol.intersects(x,y)) {
          int visibleEvents = pView->drawArea->visibleEvents( track );
          pView->drawArea->setVisibleEvents( track,  visibleEvents +1);
          pView->updateRange();
          pView->repaint();
        } else 
          if (decCol.intersects(x,y)) {
            int visibleEvents = pView->drawArea->visibleEvents( track );
            pView->drawArea->setVisibleEvents( track, std::max(1,visibleEvents -1));
            pView->updateRange();
            pView->repaint();
          } else
            if (xCol.intersects(x,y)) {
              int visibleEvents = pView->drawArea->visibleEvents( track );
              pView->drawArea->setVisibleEvents( track, 1);
              pView->updateRange();
              pView->repaint();
            }

            // now check point intersection for solo
            if (solo.intersects(x,y)) {
              onSoloLedClick(track);
            } else {
              // the rect area of the solo led
              ngrs::Rect mute(off.x() + coords_.dMuteCoords.x(), off.y() + coords_.dMuteCoords.y(), coords_.sMuteCoords.width(), coords_.sMuteCoords.height());
              // now check point intersection for solo
              if (mute.intersects(x,y)) {
                onMuteLedClick(track);
              } else
              {
                // the rect area of the record led
                ngrs::Rect record(off.x() + coords_.dRecCoords.x(), off.y() + coords_.dRecCoords.y(), coords_.sRecCoords.width(), coords_.sRecCoords.height());
                // now check point intersection for solo
                if (record.intersects(x,y)) {
                  onRecLedClick(track);
                }
              }
            }
      }
    }

    void PatternView::Header::onSoloLedClick( int track )
    {
      repaint();
    }

    void PatternView::Header::onMuteLedClick( int track )
    {
      repaint();
    }

    void PatternView::Header::onRecLedClick(int track) {
      repaint();
    }

    int PatternView::Header::preferredWidth( )
    {
      return (pView->drawArea->tracksWidth());
    }

    int PatternView::Header::skinColWidth( )
    {
      return skinColWidth_;
    }

    void PatternView::Header::setHeaderCoordInfo( const HeaderCoordInfo & info ) {
      coords_ = info;
      setHeight( coords_.bgCoords.height() );
      skinColWidth_ = coords_.bgCoords.width();
    }

    /// End of Header Class


    ///
    /// The Line Number Panel Class
    ///
    PatternView::LineNumber::LineNumber( PatternView * pPatternView ) : ngrs::Panel(), dy_(0)
    {
      setBorder( ngrs::FrameBorder() );
      pView = pPatternView;
      setWidth(60);
      setTransparent(false);
    }

    PatternView::LineNumber::~ LineNumber( )
    {
    }

    void PatternView::LineNumber::setTextColor( const ngrs::Color& textColor ) {
      textColor_ = textColor;
    }

    const ngrs::Color & PatternView::LineNumber::textColor() const {
      return textColor_;
    }

    void PatternView::LineNumber::paint( ngrs::Graphics& g )
    {
      psy::core::TimeSignature signature;

      ngrs::Rect repaintRect = g.repaintArea().rectClipBox();
      int absTop  = absoluteTop();
      int ch      = clientHeight();
      // the start for whole repaint
      int start    = dy_ / pView->rowHeight();
      // the offset for the repaint expose request
      int startOff = std::max((repaintRect.top() - absTop) / (pView->rowHeight()),(long)0);
      // the start
      start        = std::min(start + startOff, pView->lineNumber()-1);
      // the endline for whole repaint
      int end     = (dy_ + ch) / (pView->rowHeight());
      // the offset for the repaint expose request
      int endOff  = std::max((ch-(repaintRect.top()-absTop + repaintRect.height())) / (pView->rowHeight()), (long)0);
      // the end
      end         = std::min(end - endOff, pView->lineNumber()-1);

      int startLine = start;
      int endLine   = end;

      for (int i = startLine; i <= endLine; i++)
        g.drawLine(0,(i+1)*pView->rowHeight() - dy() -1,
        clientWidth(),(i+1)*pView->rowHeight() -1 - dy() );

      g.setForeground(pView->separatorColor());
      g.drawLine(0,0,0,clientHeight() );

      for (int i = startLine; i <= endLine; i++) {
        //if (i+startLine == pView->cursor().line()) {
        //  g.setForeground(Global::pConfig()->pvc_cursor);
        //  g.fillRect(0,i*pView->rowHeight()+pView->headerHeight(),clientWidth()-1,pView->rowHeight());
        //}
        std::string text = stringify( i );
        if ( pView->pattern() ) {
          float position = i  / (float) pView->pattern()->beatZoom();
          psy::core::SinglePattern::iterator it = pView->pattern()->find_nearest(i);
          if (it != pView->pattern()->end()) {
            // check out how many hidden lines there are
            int lastLine = d2i (it->first * pView->pattern_->beatZoom());
            int y = lastLine;
            psy::core::SinglePattern::iterator it2 = it;
            int count = 0;
            do {
              y = d2i (it2->first * pView->pattern_->beatZoom());
              if ( y != lastLine) break;
              it2++;
              count++;
            } while (it2 != pView->pattern()->end() && y == lastLine);

            if ( count > 1 ) {
              text+= std::string(":") + stringify( count );
            }
            // check if line is on beatzoom raster else draw arrow up or down hint
            if ( std::abs(it->first - position) > 0.001) {
              int xOff = clientWidth()-g.textWidth(text)- 10 ;
              int yOff = i*pView->rowHeight()+pView->rowHeight() -1 - 3 -dy();
              g.drawLine( xOff , yOff+1, xOff, yOff - pView->rowHeight() + 5);
              if (it->first < position) {
                g.drawLine( xOff , yOff - pView->rowHeight() + 5, xOff-3, yOff - pView->rowHeight() + 8);
                g.drawLine( xOff , yOff - pView->rowHeight() + 5, xOff+4, yOff - pView->rowHeight() + 9);
              } else {
                g.drawLine( xOff , yOff +1, xOff-3, yOff - 2);
                g.drawLine(  xOff , yOff +1, xOff+4, yOff - 3);
              }
            }
          }
          if ( pView->pattern()->barStart(position, signature) ) {
            std::string caption = stringify(signature.numerator())+"/"+stringify(signature.denominator());
            // vcenter text
            int yp = (pView->rowHeight() - g.textHeight()) / 2  + g.textAscent();
            g.drawText(0,i*pView->rowHeight()+ yp - dy(),caption, textColor() );
          }
        }
        int yp = (pView->rowHeight() - g.textHeight()) / 2  + g.textAscent();
        g.drawText(clientWidth()-g.textWidth(text)-3,i*pView->rowHeight()+yp-dy(),text, textColor() );
      }
    }

    void PatternView::LineNumber::setDy( int dy ) {
      dy_ = dy;
    }

    int PatternView::LineNumber::dy( ) const {
      return dy_;
    }

    int PatternView::LineNumber::preferredWidth() const {
      return 70;
    }

    /// End of Line Number Panel


    PatternView::TweakHeader::TweakHeader( PatternView* pPatternView ) {
      pView = pPatternView;

      bgCoords.setPosition(0,0,59,18);
      noCoords.setPosition(0,18,7,12);
    }

    PatternView::TweakHeader::~TweakHeader() {

    }

    int PatternView::TweakHeader::skinColWidth() const {
      return bgCoords.width();
    }

    void PatternView::TweakHeader::paint( ngrs::Graphics& g ) {

      DefaultBitmaps & icons = SkinReader::Instance()->bitmaps();
      ngrs::Bitmap & bitmap = icons.tweakHeader();

      int startTrack = scrollDx() / pView->tweakColWidth();
      int trackCount = spacingWidth() / pView->tweakColWidth();

      g.setForeground(pView->separatorColor());
      for (int i = startTrack; i <= std::min(startTrack + trackCount ,pView->trackNumber() - 1); i++) {
        int parameterIndex = 0;
        int machineIndex   = 0xFF;    
        if ( pView->pattern() ) {
          psy::core::TweakTrackInfo info = pView->pattern()->tweakInfo( i );
          parameterIndex = info.parameterIdx();
          machineIndex   = info.machineIdx();    
        }

        const int digit1_X0 = machineIndex / 16;
        const int digit1_0X = machineIndex % 16;
        const int digit2_X0 = parameterIndex / 16;
        const int digit2_0X = parameterIndex % 16;

        int xOff = i* pView->tweakColWidth();
        int center = ( pView->tweakColWidth() - skinColWidth() ) / 2;
        xOff += center;
        g.putBitmap(xOff,0,bgCoords.width(),bgCoords.height(), bitmap, 
          bgCoords.left(), bgCoords.top());

        g.putBitmap(xOff+14,3,noCoords.width(),noCoords.height(), bitmap,
          digit1_X0*noCoords.width(), noCoords.top());
        g.putBitmap(xOff+22,3,noCoords.width(),noCoords.height(), bitmap,
          digit1_0X*noCoords.width(), noCoords.top());  

        g.putBitmap(xOff+23 + 14,3,noCoords.width(),noCoords.height(), bitmap,
          digit2_X0*noCoords.width(), noCoords.top());
        g.putBitmap(xOff+23 + 22,3,noCoords.width(),noCoords.height(), bitmap,
          digit2_0X*noCoords.width(), noCoords.top());  

        if (i!=0) g.drawLine(i*pView->tweakColWidth(),0,i*pView->tweakColWidth(),clientWidth()); // col seperator
      }

    }

    int PatternView::TweakHeader::preferredWidth() {
      return (16)*pView->tweakColWidth();  
    }


    PatternView::TweakGUI::TweakGUI( PatternView* pPatternView)
    {
      pView = pPatternView;
      addEvent( ColumnEvent::hex4 );
      setDefaultVisibleEvents( 1 );
      setMinimumWidth(1);
      setTransparent(false);
      setTrackNumber( 16 );
    }

    PatternView::TweakGUI::~TweakGUI() {

    }

    void PatternView::TweakGUI::customPaint( ngrs::Graphics& g, int startLine, int endLine, int startTrack, int endTrack) {
      if (pView->pattern()) {
        psy::core::TimeSignature signature;

        int trackWidth = ((endTrack+1) * colWidth()) - dx();
        int lineHeight = ((endLine +1) * rowHeight()) - dy();

        for (int y = startLine; y <= endLine; y++) {
          float position = y / (float) beatZoom();
          //if (!(y == pView->playPos() || !Player::Instance()->_playing) /*|| pView->editPosition() != Global::pPlayer()->_playPosition*/) {
          if ( !(y % beatZoom())) {
            if ((pView->pattern()->barStart(position, signature) )) {
              g.setForeground( barColor() );
              g.fillRect(0, y*rowHeight() - dy(),trackWidth, rowHeight());
            } else {
              g.setForeground( beatColor() );
              g.fillRect(0, y* rowHeight() - dy(),trackWidth, rowHeight());
            }
          }
          //} else  {
          //g.setForeground( playBarColor() );
          //g.fillRect(0, y*rowHeight() - dy(), trackWidth, rowHeight());
          //} ///\todo add
        }

        drawSelBg(g,selection());
        drawColumnGrid(g, startLine, endLine, startTrack, endTrack);
        drawTrackGrid(g, startLine, endLine, startTrack, endTrack);			
        drawPattern(g, startLine, endLine, startTrack, endTrack);
        drawRestArea(g, startLine, endLine, startTrack, endTrack);
      }
    }

    void PatternView::TweakGUI::resize() {
      pView->updateRange();
    }

    void PatternView::TweakGUI::onKeyPress( const ngrs::KeyEvent& event ) {
      CustomPatternView::onKeyPress( event );

      int key = 0;
      //Global::pConfig()->inputHandler().getEnumCodeByKey( Key( event.shift(), event.scancode() ) );
      switch (key) {		
        case ' ':
          break;
        case cdefNavPageUp:
          {
            std::cout << "tweak pg up" << std::endl;
            psy::core::TimeSignature signature;
            for (int y = cursor().line()-1; y >= 0; y--) {
              float position = y / (float) beatZoom();
              if ((pView->pattern()->barStart(position, signature) )) {
                moveCursor(0, y - cursor().line() );
                pView->checkUpScroll( cursor() );
                break;
              }
            }
          }
          break;
        case cdefNavPageDn:
          {
            std::cout << "tweak pg dn" << std::endl;
            psy::core::TimeSignature signature;
            for (int y = cursor().line()+1; y < lineNumber(); y++) {
              float position = y / (float) beatZoom();
              if ((pView->pattern()->barStart(position, signature) )) {
                moveCursor(0,y - cursor().line());
                pView->checkDownScroll( cursor() );
                break;
              }
            }
          }
          break;
        case cdefNavLeft:
          checkLeftScroll( cursor() );
          return;
          break;
        case cdefNavRight:
          checkRightScroll( cursor() );
          return;
          break;
        case cdefNavDn:
          pView->checkDownScroll( cursor() );
          return;
          break;
        case cdefNavUp:
          pView->checkUpScroll( cursor() );
          return;
          break;
        case cdefNavBottom:
          pView->checkDownScroll( cursor() );
          return;
          break;
        case cdefNavTop:
          pView->checkUpScroll( cursor() );
          return;
          break;
        case cdefColumnNext:
          checkRightScroll( cursor() );
          return;
          break;
        case cdefColumnPrev: // todo ngrs eydef code
          checkLeftScroll( cursor() );
          return;
          break;
        case cdefRowClear:
          std::cout << "tweak row clear" << std::endl;
          if ( !pView->pattern()->lineIsEmpty( cursor().line() ) ) {
            pView->pattern()->clearTweakTrack( cursor().line(), cursor().track() );
          }
          moveCursor(0,-1); 
          pView->checkUpScroll( cursor() );
          return;
          break;
        case cdefRowDelete:
          std::cout << "tweak row delete" << std::endl;
          if ( !pView->pattern()->lineIsEmpty( cursor().line() ) ) {
            pView->pattern()->clearTweakTrack( cursor().line(), cursor().track() );
          }
          moveCursor(0,1); 
          pView->checkDownScroll( cursor() );
          return;
          break;
        default: ;
      }



      if (isHex(event.scancode()))
        if ( cursor().eventNr() == 0) {
          // comand or parameter
          psy::core::PatternEvent patEvent = pView->pattern()->tweakEvent( cursor().line(), cursor().track() );			
          if (cursor().col() < 2 ) {
            unsigned char newByte = convertDigit( 0x00, event.scancode(), patEvent.command(), cursor().col() );
            patEvent.setCommand( newByte );
            pView->pattern()->setTweakEvent( cursor().line(), cursor().track(), patEvent );
            moveCursor(1,0);
          }
          else {
            unsigned char newByte = convertDigit( 0x00, event.scancode(), patEvent.parameter(), cursor().col() - 2 );
            patEvent.setParameter( newByte );
            pView->pattern()->setTweakEvent( cursor().line(), cursor().track(), patEvent );
            if (cursor().col() < 3)
              moveCursor(1,0);			
            else
              moveCursor(-3,1);
            pView->checkDownScroll( cursor() );
          }			
        }

    }

    void PatternView::TweakGUI::drawPattern( ngrs::Graphics& g, int startLine, int endLine, int startTrack, int endTrack) {
      if ( pView->pattern() ) {
        drawCellBg( g, cursor() );

        psy::core::SinglePattern::iterator it = pView->pattern_->find_lower_nearest(startLine);

        int lastLine = -1;
        for ( ; it != pView->pattern_->end(); it++ ) {
          psy::core::PatternLine & line = it->second;
          int y = d2i (it->first * pView->pattern_->beatZoom());
          if (y > endLine) break;
          if (y != lastLine) {
            std::map<int, psy::core::PatternEvent>::iterator eventIt = line.tweaks().lower_bound(startTrack);
            for(; eventIt != line.tweaks().end() && eventIt->first <= endTrack; ++eventIt) {
              psy::core::PatternEvent event = eventIt->second;
              int x = eventIt->first;
              if (event.command() != 0 || event.parameter() != 0) {
                drawData( g, x, y, 0, (event.command() << 8) | event.parameter(), true, textColor() );
              }
              lastLine = y;
            }
          }
        }
      }
    }

    int PatternView::TweakGUI::colWidth() const {
      return std::max( pView->tweakHeaderWidth(), CustomPatternView::colWidth() );
    }

    int PatternView::TweakGUI::rowHeight() const {
      return pView->rowHeight();
    }

    int PatternView::TweakGUI::lineNumber() const {
      return pView->lineNumber();
    }

    int PatternView::TweakGUI::trackNumber() const {
      return 16; // hardcoded so far
    }

    int PatternView::TweakGUI::beatZoom() const {
      return pView->beatZoom();
    }

    void PatternView::TweakGUI::checkLeftScroll( const PatCursor & cursor ) {
      // check for scroll
      if ( ( cursor.track()) * colWidth() - dx() < 0) {
        pView->tweakHBar->setPos( std::max( cursor.track(), 0) * colWidth() );
      }	
    }

    void PatternView::TweakGUI::checkRightScroll( const PatCursor & cursor ) {
      //check for scroll
      if ( (cursor.track()+1) * colWidth() - dx() > clientWidth() ) {
        pView->tweakHBar->setPos( std::min( cursor.track()+1 , trackNumber()) * colWidth() - clientWidth() );
      }
    }

    int PatternView::TweakGUI::doSel(const PatCursor & selCursor) {
      int dir = CustomPatternView::doSel( selCursor );

      pView->checkDownScroll( selCursor );
      pView->checkUpScroll( selCursor );
      checkLeftScroll( selCursor );
      checkRightScroll( selCursor );

      return dir;
    }

    /// end of tweak GUI Class









    ///
    ///
    /// The PatternDraw class, that displays the data
    ///
    ///


    PatternView::PatternDraw::PatternDraw( PatternView * pPatternView ) : CustomPatternView()
    {
      addEvent( ColumnEvent::note );
      addEvent( ColumnEvent::hex2 );
      addEvent( ColumnEvent::hex2 );
      addEvent( ColumnEvent::hex2 );
      addEvent( ColumnEvent::hex4 );

      // just for test !
      addEvent( ColumnEvent::hex4 );
      addEvent( ColumnEvent::hex4 );
      addEvent( ColumnEvent::hex4 );
      // end of multi paraCmd

      sharpMode_ = true;

      setName("mainview");

      setDefaultVisibleEvents( 6 );
      setTrackNumber( 64 );

      setTransparent(false);

      pView = pPatternView;
      editPopup_ = new ngrs::NPopupMenu();
      add(editPopup_);
      editPopup_->add(new ngrs::MenuItem("Undo"));
      editPopup_->add(new ngrs::MenuItem("Redo"));
      editPopup_->add(new ngrs::MenuSeperator());
      ngrs::MenuItem* blockCutItem_ = new ngrs::MenuItem("Block cut");
      blockCutItem_->click.connect(this,&PatternView::PatternDraw::onPopupBlockCut);
      editPopup_->add(blockCutItem_);
      ngrs::MenuItem* blockCopyItem_ = new ngrs::MenuItem("Block copy");
      blockCopyItem_->click.connect(this,&PatternView::PatternDraw::onPopupBlockCopy);
      editPopup_->add(blockCopyItem_);
      ngrs::MenuItem* blockPasteItem_ = new ngrs::MenuItem("Block paste");
      blockPasteItem_->click.connect(this,&PatternView::PatternDraw::onPopupBlockPaste);
      editPopup_->add(blockPasteItem_);
      ngrs::MenuItem* blockPasteMixItem_ = new ngrs::MenuItem("Block mix paste");
      blockPasteMixItem_->click.connect(this,&PatternView::PatternDraw::onPopupBlockMixPaste);
      editPopup_->add(blockPasteMixItem_);

      ngrs::MenuItem* blockDelItem = new ngrs::MenuItem("Block delete");
      blockDelItem->click.connect(this,&PatternView::PatternDraw::onPopupBlockDelete);
      editPopup_->add(blockDelItem);

      editPopup_->add(new ngrs::MenuSeperator());
      editPopup_->add(new ngrs::MenuItem("Interpolate Effect"));
      editPopup_->add(new ngrs::MenuItem("Change Generator"));
      editPopup_->add(new ngrs::MenuItem("Change Instrument"));
      editPopup_->add(new ngrs::MenuSeperator());

      ngrs::MenuItem* blockT1Item = new ngrs::MenuItem("Transpose+1");
      blockT1Item->click.connect(this,&PatternView::PatternDraw::onPopupTranspose1);
      editPopup_->add(blockT1Item);

      ngrs::MenuItem* blockT_1Item = new ngrs::MenuItem("Transpose-1");
      blockT_1Item->click.connect(this,&PatternView::PatternDraw::onPopupTranspose_1);
      editPopup_->add(blockT_1Item);

      ngrs::MenuItem* blockT12Item = new ngrs::MenuItem("Transpose12");
      blockT12Item->click.connect(this,&PatternView::PatternDraw::onPopupTranspose12);
      editPopup_->add(blockT12Item);

      ngrs::MenuItem* blockT_12Item = new ngrs::MenuItem("Transpose-12");
      blockT_12Item->click.connect(this,&PatternView::PatternDraw::onPopupTranspose_12);
      editPopup_->add(blockT_12Item);

      editPopup_->add(new ngrs::MenuSeperator());

      editPopup_->add(new ngrs::MenuItem("Block Swing Fill"));
      editPopup_->add(new ngrs::MenuItem("Block Track Fill"));
      editPopup_->add(new ngrs::MenuSeperator());
      ngrs::MenuItem* blockPatPropItem_ = new ngrs::MenuItem("Pattern properties");
      blockPatPropItem_->click.connect(this,&PatternView::PatternDraw::onPopupPattern);
      editPopup_->add(blockPatPropItem_);
    }

    PatternView::PatternDraw::~ PatternDraw( )
    {
    }

    void PatternView::PatternDraw::setSharpMode( bool on ) {
      sharpMode_ = !sharpMode_;
    }

    bool PatternView::PatternDraw::sharpMode() const {
      return sharpMode_;
    }

    void PatternView::PatternDraw::resize() {
      pView->updateRange();
    }

    int PatternView::PatternDraw::colWidth() const {
      int col = CustomPatternView::colWidth();
      return std::max( pView->headerWidth() , col );
    }

    int PatternView::PatternDraw::rowHeight() const {
      return pView->rowHeight();
    }

    int PatternView::PatternDraw::lineNumber() const {
      return pView->lineNumber();
    }

    int PatternView::PatternDraw::beatZoom() const {
      return pView->beatZoom();
    }

    void PatternView::PatternDraw::customPaint( ngrs::Graphics& g, int startLine, int endLine, int startTrack, int endTrack)
    {
      if (pView->pattern()) {
        psy::core::TimeSignature signature;

        int trackWidth = xEndByTrack( endTrack ) - dx();
        int lineHeight = ((endLine +1) * rowHeight()) - dy();

        drawSelBg(g,selection());

        for (int y = startLine; y <= endLine; y++) {
          float position = y / (float) beatZoom();
          if (!(y == pView->playPos())/*!Player::Instance()->playing()*/ ) {
            if ( !(y % beatZoom())) {
              if ((pView->pattern()->barStart(position, signature) )) {

                g.setForeground( barColor() );
                g.fillRect(0, y*rowHeight() - dy(),trackWidth, rowHeight());

                if ( y >= selection().top() && y < selection().bottom()) {
                  int left  = xOffByTrack( selection().left() );
                  int right = xOffByTrack( selection().right() );
                  g.setForeground( pView->colorInfo().sel_bar_bg_color );
                  g.fillRect( left - dx(), y*rowHeight() - dy(), right - left, rowHeight());
                }

              } else {
                g.setForeground( beatColor() );
                g.fillRect(0, y* rowHeight() - dy(),trackWidth, rowHeight());

                if ( y >= selection().top() && y < selection().bottom()) {
                  int left  = xOffByTrack( selection().left() );
                  int right = xOffByTrack( selection().right() );
                  g.setForeground( pView->colorInfo().sel_beat_bg_color );
                  g.fillRect( left - dx(), y*rowHeight() - dy(), right - left, rowHeight());
                }
              }
            }
          }
        }


        drawColumnGrid(g, startLine, endLine, startTrack, endTrack);
        drawTrackGrid(g, startLine, endLine, startTrack, endTrack);	
        drawPattern(g, startLine, endLine, startTrack, endTrack);
        drawRestArea(g, startLine, endLine, startTrack, endTrack);

      }
    }



    void PatternView::PatternDraw::drawPattern( ngrs::Graphics& g, int startLine, int endLine, int startTrack, int endTrack )
    {
      // do we have a pattern ?
      if ( pView->pattern() ) {
        drawCellBg( g, cursor()  );

        // find start iterator
        psy::core::SinglePattern::iterator it = pView->pattern_->find_lower_nearest(startLine);
        psy::core::TimeSignature signature;

        int lastLine = -1;
        psy::core::PatternLine* line;
        psy::core::PatternLine emptyLine;

        for ( int y = startLine; y <= endLine; y++) {

          if ( it != pView->pattern_->end() )	{
            int liney = d2i (it->first * pView->pattern_->beatZoom());
            if (liney == y ) {
              line = &it->second;
              it++;
            } else line = &emptyLine;
          } else line = &emptyLine;


          if (y != lastLine) {
            ngrs::Color tColor = textColor();

            bool onBeat = false;
            bool onBar  = false;
            if ( !(y % beatZoom())) {
              if (  it != pView->pattern_->end() && pView->pattern()->barStart(it->first, signature) ) {
                tColor = barColor();
              } else {
                onBeat = true;
                tColor = beatTextColor();
              }
            }

            if ((y == pView->playPos() /*&& Player::Instance()->playing()*/ ) ) {
              int trackWidth = xEndByTrack( endTrack ) - dx();
              g.setForeground( playBarColor() );
              g.fillRect(0, y*rowHeight() - dy(), trackWidth, rowHeight());
            }

            ngrs::Color stdColor = tColor;
            ngrs::Color crColor = pView->colorInfo().cursor_text_color;

            std::map<int, psy::core::PatternEvent>::iterator eventIt = line->notes().lower_bound(startTrack);
            psy::core::PatternEvent emptyEvent;
            psy::core::PatternEvent* event;

            for ( int x = startTrack; x <= endTrack; x++ ) {

              if ( eventIt != line->notes().end() && eventIt->first <= endTrack ) {
                int trackx = eventIt->first;
                if ( x == trackx) {
                  event = &eventIt->second;
                  eventIt++;
                } else {
                  event = &emptyEvent;
                }
              } else event = &emptyEvent;


              if ( x >= selection().left() && x < selection().right() &&
                y >= selection().top() && y < selection().bottom() ) {
                  if ( !onBeat ) 
                    tColor = pView->colorInfo().sel_text_color;
                  else 
                    tColor = pView->colorInfo().sel_beat_text_color;
              }	else tColor = stdColor;

              drawData( g, x, y, 0, event->note() ,event->isSharp(), tColor );
              if (event->instrument() != 255) drawData( g, x, y, 1, event->instrument(), 1, tColor );
              if (event->machine() != 255) drawData( g, x, y, 2, event->machine(), 1, tColor );
              if (event->volume() != 255) drawData( g, x, y, 3, event->volume(), 1, tColor );
              if (event->command() != 0 || event->parameter() != 0) {
                drawData( g, x, y, 4, (event->command() << 8) | event->parameter(), 1, tColor );
              }	else {
                drawString( g, x, y, 4, "....", tColor );
              }									

              psy::core::PatternEvent::PcmListType & pcList = event->paraCmdList();
              psy::core::PatternEvent::PcmListType::iterator it = pcList.begin();
              int count = 0;
              for ( ; it < pcList.end(); it++, count++ ) {
                psy::core::PatternEvent::PcmType & pc = *it;
                int command = pc.first;
                int parameter = pc.second;
                if ( command != 0 || parameter != 0) {
                  drawData( g, x, y, 5+count, ( command << 8) | parameter , 1, tColor );
                }
              }
            }
            lastLine = y;
          }
        }
      }
    }


    void PatternView::PatternDraw::onMousePress( int x, int y, int button )
    {
      CustomPatternView::onMousePress( x, y, button );
      if (button == 6) {
        std::cout << "wheel mouse scroll left" << std::endl;
        // wheel mouse scroll left
        int startTrack  = dx() / pView->colWidth();
        int newTrack = std::max(0,startTrack-1);
        pView->hScrBar()->setPos( (newTrack) * pView->colWidth());
      } else
        if (button == 7) {
          // wheel mouse scrolling right
          std::cout << "wheel mouse scroll right" << std::endl;
          int startTrack  = dx() / pView->colWidth();
          int newTrack = std::max(0,std::min(pView->trackNumber(),startTrack+1));
          pView->hScrBar()->setPos( (newTrack) * pView->colWidth());
        } else
          if (button == 4) {
            // wheel mouse scroll up
            int startLine  = dy() / pView->rowHeight();
            int newLine = std::max(0,startLine-1);
            pView->vScrBar()->setPos( (newLine) * pView->rowHeight());
          } else
            if (button == 5) {
              // wheel mouse scrolling down
              int startLine  = dy() / pView->rowHeight();
              int newLine = std::max(0,std::min(pView->lineNumber(),startLine+1));
              pView->vScrBar()->setPos( (newLine) * pView->rowHeight());
            } else 
              if (button == 2) {
                // todo linux paste
              } else
                if (button == 3) {
                  editPopup_->setPosition(x + absoluteLeft() + window()->left(), y + absoluteTop() + window()->top(),100,100);
                  editPopup_->setVisible(true);
                }
    }

    void PatternView::PatternDraw::onMouseOver( int x, int y )
    {
      CustomPatternView::onMouseOver( x, y);
    }

    void PatternView::PatternDraw::onMousePressed( int x, int y, int button )
    {  
      CustomPatternView::onMousePressed(x,y,button);
    }

    void PatternView::PatternDraw::onKeyPress( const ngrs::KeyEvent & event )
    {
      if ( !pView->pattern() ) return;
      CustomPatternView::onKeyPress( event );

      int command = 0;
      //Global::pConfig()->inputHandler().getEnumCodeByKey( Key( event.shift(), event.scancode() ) );
      switch (command) {		
        case cdefEditToggle:
          // FIXME: needs to toggle edit mode...
/*          if (Player::Instance()->playing() ) {
            Player::Instance()->stop();
          } else {
            Player::Instance()->start(0);
          }*/
          break;
        case cdefNavPageUp:
          {
            std::cout << "nav page up" << std::endl;
            psy::core::TimeSignature signature;
            for (int y = cursor().line()-1; y >= 0; y--) {
              float position = y / (float) beatZoom();
              if ((pView->pattern()->barStart(position, signature) )) {
                moveCursor(0, y - cursor().line() );
                pView->checkUpScroll( cursor() );
                break;
              }
            }
          }
          break;
        case cdefNavPageDn:
          {
            std::cout << "nav page dn" << std::endl;
            psy::core::TimeSignature signature;
            for (int y = cursor().line()+1; y < lineNumber(); y++) {
              float position = y / (float) beatZoom();
              if ((pView->pattern()->barStart(position, signature) )) {
                moveCursor(0,y - cursor().line());
                pView->checkDownScroll( cursor() );
                break;
              }
            }
          }
          break;
        case cdefNavLeft:
          checkLeftScroll( cursor() );
          return;
          break;
        case cdefNavRight:
          checkRightScroll( cursor() );
          return;
          break;
        case cdefNavDn:
          pView->checkDownScroll( cursor() );
          return;
          break;
        case cdefNavUp:
          pView->checkUpScroll( cursor() );
          return;
          break;
        case cdefNavBottom:
          pView->checkDownScroll( cursor() );
          return;
          break;
        case cdefNavTop:
          pView->checkUpScroll( cursor() );
          return;
          break;
        case cdefColumnNext:
          checkRightScroll( cursor() );
          return;
          break;
        case cdefNavFirstTrack:
          checkLeftScroll( cursor() );
          break;
        case cdefNavLastTrack:
          checkRightScroll( cursor() );
          break;
        case cdefColumnPrev: // todo ngrs 
          checkLeftScroll( cursor() );
          return;
          break;
        case cdefSelectLeft:
          checkLeftScroll( cursor() );
          return;
          break;
        case cdefSelectRight:
          checkRightScroll( cursor() );
          return;
          break;
        case cdefSelectUp:
          pView->checkUpScroll( cursor() );
          return;
          break;
        case cdefSelectTop:
          pView->checkUpScroll( cursor() );
          return;
          break;
        case cdefSelectDn:
          pView->checkDownScroll( cursor() );
          return;
          break;
        case cdefSelectBottom:
          pView->checkDownScroll( cursor() );
          return;
          break;
        case cdefRowDelete:
          std::cout << "pattern delete row" << std::endl;
          clearCursorPos();
          moveCursor(0,1); 
          pView->checkDownScroll( cursor() );
          return;
          break;
        case cdefRowClear:
          std::cout << "pattern clear row" << std::endl;
          clearCursorPos();
          moveCursor(0,-1); 
          pView->checkUpScroll( cursor() );
          break;
        case cdefRowInsert:
          std::cout << "pattern insert row" << std::endl;
          return;
          break;
        default: ;
      }

      switch (command) {
            case cdefUndo:
              std::cout << "key: edit undo" << std::endl;
              pView->doUndo();
              return;
              break;
            case cdefSelectAll:
              std::cout << "key: block select all" << std::endl;
              selectAll(cursor());
              break;
            case cdefBlockCopy: 
              std::cout << "key: block copy" << std::endl;
              copyBlock(false);
              return;
              break;
            case cdefBlockPaste: 
              std::cout << "key: block paste" << std::endl;
              pasteBlock( cursor().track(), cursor().line(), false);

              if (pView->moveCursorWhenPaste()) {
                /*       if (pView->cursor().y()+ blockLines < pView->lineNumber() ) {
                pView->setCursor(Point3D(pView->cursor().x(),pView->cursor().y()+blockLines,pView->cursor().z()));
                } else
                pView->setCursor(Point3D(pView->cursor().x(),pView->lineNumber()-1,pView->cursor().z()));*/
              }
              pView->repaint();
              return;
              break;
            case cdefOctaveDn:
              pView->setEditOctave( std::min(pView->editOctave() + 1, 8) );
              return;
              break;
            case cdefOctaveUp:
              pView->setEditOctave( std::max(pView->editOctave() - 1, 0) );
              return;
              break;
            case cdefPatternstepDec:
              pView->setPatternStep(std::max(1, pView->patternStep()-1));
              break;
            case cdefPatternstepInc:
              pView->setPatternStep(std::min(16, pView->patternStep()+1));
              break;
      }

      if ( cursor().eventNr() == 0 ) {
        // A note event.
        if (!( event.shift() & ngrs::nsCtrl )) {
          // We don't want a note to fire if ctrl is held down.
          std::cout << "event #0 - note event" << std::endl;
          int note = 0;
          //Global::pConfig()->inputHandler().getEnumCodeByKey(Key( ngrs::nsNone,event.scancode()));
          if ( note == cdefKeyStop ) {
            pView->undoManager().addUndo( cursor() );
            pView->noteOffAny( cursor() );
          } else
            if (note >=0 && note < 120) {
              pView->undoManager().addUndo( cursor() );
              pView->enterNote( cursor(), note );
              moveCursor(0,1);
              pView->checkDownScroll( cursor() );
            }
        }
      } else
        if ( isHex(event.scancode()) ) {
          if ( cursor().eventNr() == 1 ) {
            // inst select
            // i.e. a key is pressed in the instrument column.
            std::cout << "eventnr 1 - inst select" << std::endl;

            // Add the new data...
            psy::core::PatternEvent patEvent = pView->pattern()->event( cursor().line(), cursor().track() );
            unsigned char newByte = convertDigit( 0xFF, event.scancode(), patEvent.instrument(), cursor().col() );
            patEvent.setInstrument( newByte );
            pView->pattern()->setEvent( cursor().line(), cursor().track(), patEvent );
            // ...and move the cursor.
            if (cursor().col() == 0) {
              moveCursor(1,0);			
            } else {
              moveCursor(-1,1);
              pView->checkDownScroll( cursor() );
            }
          } else 
            if ( cursor().eventNr() == 2) {
              // mac select
              // i.e. a key is pressed in the machine column.
              std::cout << "event nr 2 - mach select" << std::endl;
              psy::core::PatternEvent patEvent = pView->pattern()->event( cursor().line(), cursor().track() );
              unsigned char newByte = convertDigit( 0xFF, event.scancode(), patEvent.machine(), cursor().col() );
              patEvent.setMachine( newByte );
              pView->pattern()->setEvent( cursor().line(), cursor().track(), patEvent );
              if (cursor().col() == 0) {
                moveCursor(1,0);			
              } else {
                moveCursor(-1,1);
                pView->checkDownScroll( cursor() );
              }
            } else
              if ( cursor().eventNr() == 3) {
                // mac select
                std::cout << "event nr 3 - mach select" << std::endl;
                psy::core::PatternEvent patEvent = pView->pattern()->event( cursor().line(), cursor().track() );
                unsigned char newByte = convertDigit( 0xFF, event.scancode(), patEvent.volume(), cursor().col() );
                patEvent.setVolume( newByte );
                pView->pattern()->setEvent( cursor().line(), cursor().track(), patEvent );
                if (cursor().col() == 0) {
                  moveCursor(1,0);			
                } else {
                  moveCursor(-1,1);
                }
                pView->checkDownScroll( cursor() );
              } else
                if ( cursor().eventNr() >= 4) {
                  // comand or parameter
                  std::cout << "event nr >=4 - command or parameter" << std::endl;
                  psy::core::PatternEvent patEvent = pView->pattern()->event( cursor().line(), cursor().track() );
                  if (cursor().col() < 2 ) {
                    int cmdValue;
                    if (cursor().eventNr() == 4) {
                      cmdValue = patEvent.command();
                    } else {
                      psy::core::PatternEvent::PcmType & pc = patEvent.paraCmdList()[cursor().eventNr() - 5];
                      cmdValue = pc.first;
                    }
                    unsigned char newByte = convertDigit( 0x00, event.scancode(), cmdValue, cursor().col() );
                    if (cursor().eventNr() == 4) {
                      patEvent.setCommand( newByte );
                    } else {
                      psy::core::PatternEvent::PcmType & pc = patEvent.paraCmdList()[cursor().eventNr() - 5];
                      pc.first = newByte;					
                    }
                    pView->pattern()->setEvent( cursor().line(), cursor().track(), patEvent );
                    moveCursor(1,0);
                  }
                  else {
                    int paraValue;
                    if (cursor().eventNr() == 4) {
                      paraValue = patEvent.parameter();
                    } else {
                      psy::core::PatternEvent::PcmType & pc = patEvent.paraCmdList()[cursor().eventNr() - 5];
                      paraValue = pc.second;
                    }
                    unsigned char newByte = convertDigit( 0x00, event.scancode(), paraValue, cursor().col() - 2 );
                    if (cursor().eventNr() == 4) {
                      patEvent.setParameter( newByte );
                    } else {
                      psy::core::PatternEvent::PcmType & pc = patEvent.paraCmdList()[cursor().eventNr() - 5];
                      pc.second = newByte;					
                    }
                    pView->pattern()->setEvent( cursor().line(), cursor().track(), patEvent );
                    if (cursor().col() < 3) {
                      moveCursor(1,0);			
                    } else {
                      moveCursor(-3,1);
                      pView->checkDownScroll( cursor() );
                    }
                  }			
                }
        }
    }

    void PatternView::PatternDraw::clearCursorPos() {
      if ( !pView->pattern()->lineIsEmpty( cursor().line() ) ) {
        psy::core::PatternEvent patEvent = pView->pattern()->event( cursor().line(), cursor().track() );
        if ( cursor().eventNr() == 0) {
          pView->pattern()->clearTrack( cursor().line(), cursor().track() );
        } else
          if (cursor().eventNr() == 1) {					
            patEvent.setInstrument(255);
            pView->pattern()->setEvent( cursor().line(), cursor().track() , patEvent );
          } else
            if (cursor().eventNr() == 2) {					
              patEvent.setMachine(255);
              pView->pattern()->setEvent( cursor().line(), cursor().track() , patEvent );
            } else
              if (cursor().eventNr() == 3) {					
                patEvent.setVolume(255);
                pView->pattern()->setEvent( cursor().line(), cursor().track() , patEvent );
              } else
                if (cursor().eventNr() == 4 ) {					
                  patEvent.setCommand(0);
                  patEvent.setParameter(0);
                  pView->pattern()->setEvent( cursor().line(), cursor().track() , patEvent );
                } else {					
                  psy::core::PatternEvent::PcmType & pc = patEvent.paraCmdList()[cursor().eventNr() - 5];
                  pc.first = 0;
                  pc.second = 0;
                }			
      }	
    }

    void PatternView::checkDownScroll( const PatCursor & cursor ) {
      // check for scroll
      if ( (cursor.line()+1) * drawArea->rowHeight() - drawArea->dy() > drawArea->clientHeight() ) {
        vBar->setPos( (std::min(cursor.line(),lineNumber()-1)+1) * drawArea->rowHeight() - drawArea->clientHeight() );
      }
    }

    void PatternView::checkUpScroll( const PatCursor & cursor ) {
      // check for scroll
      if ( (cursor.line()) * drawArea->rowHeight() - drawArea->dy() < 0 ) {
        vBar->setPos( std::max(0,cursor.line()) * drawArea->rowHeight());
      }
    }

    void PatternView::PatternDraw::checkLeftScroll( const PatCursor & cursor ) {
      // check for scroll
      if ( xOffByTrack(cursor.track()) - dx() < 0) {
        pView->hBar->setPos( xOffByTrack(std::max( cursor.track(), 0)) );
      }	
    }

    void PatternView::PatternDraw::checkRightScroll( const PatCursor & cursor ) {
      //check for scroll
      if ( xOffByTrack(std::max( cursor.track()+1, 0)) - dx() > clientWidth() ) {
        pView->hBar->setPos( xEndByTrack(std::min( cursor.track()+1 , trackNumber()-1)) - clientWidth() );
      }
    }

    void PatternView::PatternDraw::selectAll(const PatCursor & cursor) {
      CustomPatternView::selectAll(cursor);
    }

    int PatternView::PatternDraw::doSel(const PatCursor & selCursor) {
      int dir = CustomPatternView::doSel( selCursor );

      pView->checkDownScroll( selCursor );
      pView->checkUpScroll( selCursor );
      checkLeftScroll( selCursor );
      checkRightScroll( selCursor );
      std::cout << "here" << std::endl;

      return dir;
    }

    void PatternView::enterNote( const PatCursor & cursor, int note ) {
      if ( pattern() ) {
        psy::core::PatternEvent event = pattern()->event( cursor.line(), cursor.track() );
        event.setNote( editOctave() * 12 + note );
        event.setSharp( drawArea->sharpMode() );        
        pattern()->setEvent( cursor.line(), cursor.track(), event );
        //if (tmac) PlayNote( editOctave() * 12 + note, 127, false, tmac);   
      }
    }

    void PatternView::repaintLineNumber( int startLine, int endLine ) {

    }

    ngrs::Rect PatternView::repaintLineNumberArea( int startLine, int endLine )
    {
      int top    = startLine    * rowHeight()  + drawArea->absoluteTop()  - lineNumber_->dy();
      int bottom = (endLine+3)  * rowHeight()  + drawArea->absoluteTop()  - lineNumber_->dy();

      return ngrs::Rect(lineNumber_->absoluteLeft(),top,lineNumber_->clientWidth(),bottom - top);
    }

    void PatternView::PatternDraw::onPopupBlockCopy( ngrs::ButtonEvent* ev )
    {
      copyBlock(false);
    }

    void PatternView::PatternDraw::onPopupBlockCut( ngrs::ButtonEvent* ev )
    {
      copyBlock(true);
    }

    void PatternView::PatternDraw::copyBlock( bool cutit )
    {  
      isBlockCopied=true;
      pasteBuffer.clear();
      psy::core::SinglePattern copyPattern = pView->pattern()->block( selection().left(), selection().right(), selection().top(), selection().bottom() );

      float start = selection().top()    / (float) pView->pattern()->beatZoom();
      float end   = selection().bottom() / (float) pView->pattern()->beatZoom();

      std::string xml = "<patsel beats='" + stringify( end - start ); 
      xml+= "' tracks='"+ stringify( selection().right() - selection().left() );
      xml+= "'>"; 
      xml+= copyPattern.toXml();
      xml+= "</patsel>";

      ngrs::App::system().clipBoard().setAsText( xml );

      if (cutit) {
        pView->undoManager().addUndo( ngrs::Size( selection().left(), selection().top(), selection().right(), selection().bottom() ), cursor() );
        pView->pattern()->deleteBlock( selection().left(), selection().right(), selection().top(), selection().bottom() );
        pView->repaint();
      }
    }

    void PatternView::PatternDraw::onPopupBlockDelete( ngrs::ButtonEvent * ev )
    {	
      deleteBlock();
      repaint();
    }

    void PatternView::PatternDraw::onPopupBlockMixPaste( ngrs::ButtonEvent * ev )
    {
      pasteBlock( cursor().track(), cursor().line(),true);

      if (pView->moveCursorWhenPaste()) {
        /*       if (pView->cursor().y()+ blockLines < pView->lineNumber() ) {
        pView->setCursor(Point3D(pView->cursor().x(),pView->cursor().y()+blockLines,pView->cursor().z()));
        } else
        pView->setCursor(Point3D(pView->cursor().x(),pView->lineNumber()-1,pView->cursor().z()));*/
      }
      pView->repaint();
    }

    void PatternView::PatternDraw::onPopupBlockPaste( ngrs::ButtonEvent * ev )
    {
      pasteBlock( cursor().track(), cursor().line(), false);

      if (pView->moveCursorWhenPaste()) {
        /*       if (pView->cursor().y()+ blockLines < pView->lineNumber() ) {
        pView->setCursor(Point3D(pView->cursor().x(),pView->cursor().y()+blockLines,pView->cursor().z()));
        } else
        pView->setCursor(Point3D(pView->cursor().x(),pView->lineNumber()-1,pView->cursor().z()));*/
      }
      pView->repaint();
    }

    void PatternView::PatternDraw::pasteBlock(int tx,int lx,bool mix,bool save)
    {
      if ( ngrs::App::system().clipBoard().asText() != "" ) {
        ngrs::XmlParser parser;
        lastXmlLineBeatPos = 0.0;
        xmlTracks = 0;
        xmlBeats = 0;
        parser.tagParse.connect(this, &PatternView::PatternDraw::onTagParse);
        parser.parseString( ngrs::App::system().clipBoard().asText() );

        if (!mix)
          pView->pattern()->copyBlock(tx,lx,pasteBuffer,xmlTracks,xmlBeats);
        else
          pView->pattern()->mixBlock(tx,lx,pasteBuffer,xmlTracks,xmlBeats);
      }
    }

    void PatternView::updatePlayBar(bool followSong)
    {
      /*  if ( ((VisualComponent*) parent())->visible() && (Global::pPlayer()->_lineChanged) && (editPosition() == Global::pPlayer()->_playPosition) && !followSong )
      {
      int trackCount  = clientWidth() / colWidth();
      int startTrack  = drawArea->dx() / colWidth();

      int oldPlayPos = playPos_;
      playPos_ = Global::pPlayer()->_lineCounter;
      if (oldPlayPos < playPos_)

      window()->repaint(drawArea,drawArea->repaintTrackArea(oldPlayPos,playPos_,startTrack,trackCount+startTrack));
      else if (oldPlayPos > playPos_) {
      window()->repaint(drawArea,drawArea->repaintTrackArea(oldPlayPos,oldPlayPos,startTrack,trackCount+startTrack));
      window()->repaint(drawArea,drawArea->repaintTrackArea(playPos_,playPos_,startTrack,trackCount+startTrack));
      }
      } else 
      if (visible() && followSong) {

      if (editPosition() != Global::pPlayer()->_playPosition) {
      setEditPosition(Global::pPlayer()->_playPosition);
      playPos_ = Global::pPlayer()->_lineCounter;
      int startLine  = playPos_;
      vScrBar()->posChange.disconnect_all();
      vScrBar()->setPos( (startLine) * rowHeight());
      vBar->posChange.connect(this,&PatternView::onVScrollBar);
      drawArea->setDy(startLine * rowHeight());
      lineNumber_->setDy(startLine * rowHeight());
      repaint();
      } else
      if ((Global::pPlayer()->_lineChanged) && editPosition() == Global::pPlayer()->_playPosition) {
      int startLine  = drawArea->dy() / rowHeight();
      int lineCount  = drawArea->clientHeight() / rowHeight();
      int oldLine = playPos_;
      playPos_ = Global::pPlayer()->_lineCounter;

      if (oldLine < playPos_) {
      int newLine = playPos_;
      if (newLine > startLine + lineCount-1) {
      vScrBar()->setPos( (startLine+1) * rowHeight());
      }
      int trackCount  = drawArea->clientWidth() / colWidth();
      int startTrack  = drawArea->dx() / colWidth();

      window()->repaint(this,drawArea->repaintTrackArea(oldLine,newLine,startTrack,trackCount+startTrack));
      } else if (playPos_ < oldLine) {
      vScrBar()->setPos(0);
      }
      }

      }*/
    }

    void PatternView::PlayNote(int note,int velocity,bool bTranspose,Machine*pMachine)
    {
    }

    void PatternView::PatternDraw::transposeBlock(int trp)
    {
      int right = selection().right();
      int left =  selection().left();
      double top = selection().top() / (double)pView->beatZoom();
      double bottom = selection().bottom() / (double)pView->beatZoom();

      pView->undoManager().addUndo( ngrs::Size( selection().left(), selection().top(), selection().right(), selection().bottom() ), cursor() );

      pView->pattern()->transposeBlock(left, right, top, bottom, trp);

      window()->repaint(this,repaintTrackArea(selection().top(),selection().bottom(),left,right));
      //   }
    }


    void PatternView::noteOffAny( const PatCursor & cursor )
    {
      if (pattern_) {
        psy::core::PatternEvent event;
        event.setNote(120);
        pattern()->setEvent( cursor.line(), cursor.track(), event );
        drawArea->repaintCursorPos( cursor );    
      }
    }

    void PatternView::PatternDraw::onPopupPattern( ngrs::ButtonEvent * ev )
    {
    }

    void PatternView::PatternDraw::onPopupTranspose1( ngrs::ButtonEvent * ev )
    {
      transposeBlock(1);
    }

    void PatternView::PatternDraw::onPopupTranspose12( ngrs::ButtonEvent * ev )
    {
      transposeBlock(12);
    }

    void PatternView::PatternDraw::onPopupTranspose_1( ngrs::ButtonEvent * ev )
    {
      transposeBlock(-1);
    }

    void PatternView::PatternDraw::onPopupTranspose_12( ngrs::ButtonEvent * ev )
    {
      transposeBlock(-12);
    }


    void PatternView::PatternDraw::onKeyRelease(const ngrs::KeyEvent & event) {
      CustomPatternView::onKeyRelease( event );
      if ( !pView->pattern() ) return;

      if ( cursor().eventNr() == 0 ) {
        int outnote = 0;
        //Global::pConfig()->inputHandler().getEnumCodeByKey(Key(ngrs::nsNone,event.scancode()));
        pView->StopNote( outnote );
      }
    }

    void PatternView::copyBlock( bool cutit )
    {
      drawArea->copyBlock(cutit);
    }

    void PatternView::pasteBlock( int tx, int lx, bool mix, bool save )
    {
      drawArea->pasteBlock(tx,lx,mix,save);
    }

    void PatternView::blockTranspose( int trp )
    {
      drawArea->transposeBlock(trp);
    }

    void PatternView::PatternDraw::deleteBlock( )
    {
      int right = selection().right();
      int left = selection().left();
      double top = selection().top() / (double) pView->beatZoom();
      double bottom = selection().bottom() / (double) pView->beatZoom();

      pView->undoManager().addUndo( ngrs::Size( selection().left(), selection().top(), selection().right(), selection().bottom() ), cursor() );

      pView->pattern()->deleteBlock(left, right, top, bottom);
      repaint();
    }

    void PatternView::deleteBlock( )
    {
      drawArea->deleteBlock();
    }

    void PatternView::setPatternStep( int step )
    {
      drawArea->setPatternStep( step );
      patternCombo_->setIndex(step-1);
      patternCombo_->repaint();
    }

    int PatternView::patternStep() const {
      return drawArea->patternStep();
    }

    void PatternView::StopNote( int note, bool bTranspose, Machine * pMachine )
    {
    }

    void PatternView::PatternDraw::scaleBlock(float factor )
    {
      int right = selection().right();
      int left =  selection().left();
      double top = selection().top() / (double)pView->beatZoom();
      double bottom = selection().bottom() / (double)pView->beatZoom();
      pView->pattern()->scaleBlock(left, right, top, bottom, factor);
      window()->repaint(this,repaintTrackArea(selection().top(),static_cast<int>( selection().bottom()*std::max(factor,1.0f) ),left,right));
      pView->pattern()->clearEmptyLines();
    }

    void PatternView::doubleLength( )
    {
      drawArea->scaleBlock(2.0f);
    }
    void PatternView::halveLength()
    {
      drawArea->scaleBlock(0.5f);
    }

    int PatternView::beatZoom( ) const
    {
      if (pattern_)
        return pattern_->beatZoom();
      else
        return 4;
    }

    void PatternView::doUndo() {

      if ( undoManager_.size() > 0 ) {
        UndoPattern & lastPattern = undoManager_.back();
        drawArea->setCursor( lastPattern.oldCursor() );
        undoManager_.doUndo();
        drawArea->repaint();
      }
    }

    void PatternView::setPattern( psy::core::SinglePattern * pattern )
    {
      pattern_ = pattern;
      undoManager_.setPattern( pattern );
      resize();
    }

    psy::core::SinglePattern * PatternView::pattern( )
    {
      return pattern_;
    }

    psy::core::Song * PatternView::pSong( )
    {
      return _pSong;
    }

    void PatternView::setBeatZoom( int tpb )
    {
      if (pattern_) {    
        pattern_->setBeatZoom( std::max(tpb, 1) );
        int count = (drawArea->clientHeight() - headerHeight()) / rowHeight();
        vBar->setRange( 0,  ( lineNumber() - 1 - count) * rowHeight());
        if ( drawArea->cursor().line() > lineNumber()-1 ) {
          PatCursor newCursor = drawArea->cursor();
          newCursor.setLine( lineNumber() - 1 );
          drawArea->setCursor( newCursor );
        }
      }
    }

    void PatternView::onOctaveChange( ngrs::ItemEvent * ev )
    {
      setEditOctave( ngrs::str<int> ( ev->item()->text() ) );
    }

    void PatternView::onTrackChange( ngrs::ItemEvent * ev )
    {
//      pSong()->setTracks( ngrs::str<int>( ev->item()->text() ) );
//      drawArea->setTrackNumber( pSong()->tracks() ); 
//      updateRange();
      //if (cursor().track() >= pSong()->tracks() )
      //{
      //  setCursor( PatCursor( pSong()->tracks() ,cursor().line(),0,0) );
      //}
      repaint();
    }

    void PatternView::setColorInfo( const PatternViewColorInfo & info ) {
      colorInfo_ = info;

      drawArea->setRestAreaColor( colorInfo_.restarea_bg_color );
      drawArea->setSelectionColor( colorInfo_.sel_bg_color );
      drawArea->setCursorColor( colorInfo_.cursor_bg_color );
      drawArea->setBarColor( colorInfo_.bar_bg_color );
      drawArea->setBeatColor( colorInfo_.beat_bg_color );
      drawArea->setBackground( colorInfo_.bg_color );
      drawArea->setPlayBarColor( colorInfo_.playbar_bg_color  );
      drawArea->setBigTrackSeparatorColor( colorInfo_.track_big_sep_color );
      drawArea->setLineSeparatorColor( colorInfo_.line_sep_color );

      drawArea->setSeparatorColor(colorInfo_.col_sep_color );
      drawArea->setSmallTrackSeparatorColor(colorInfo_.track_small_sep_color );
      drawArea->setTextColor(colorInfo_.text_color );
      drawArea->setBeatTextColor(colorInfo_.beat_text_color );

      tweakGUI->setRestAreaColor( colorInfo_.restarea_bg_color );
      tweakGUI->setSelectionColor( colorInfo_.sel_bg_color  );
      tweakGUI->setCursorColor( colorInfo_.cursor_bg_color  );
      tweakGUI->setBarColor( colorInfo_.bar_bg_color  );
      tweakGUI->setBeatColor( colorInfo_.beat_bg_color  );
      tweakGUI->setBackground( colorInfo_.bg_color  );
      tweakGUI->setPlayBarColor( colorInfo_.playbar_bg_color );
      tweakGUI->setBigTrackSeparatorColor( colorInfo_.track_big_sep_color );
      tweakGUI->setLineSeparatorColor(colorInfo_.line_sep_color );

      tweakGUI->setSeparatorColor(colorInfo_.col_sep_color );
      tweakGUI->setTextColor( colorInfo_.text_color );
      tweakGUI->setBeatTextColor(colorInfo_.beat_text_color );
      tweakGUI->setSmallTrackSeparatorColor(colorInfo_.track_small_sep_color );

      lineNumber_->setBackground ( colorInfo_.bg_color );
      lineNumber_->setTextColor( colorInfo_.text_color );

      header->setBackground( colorInfo_.bg_color   );

      lineHeaderLabel->setBackground( colorInfo_.bg_color );
      lineHeaderLabel->setTransparent( false );

    }

    const PatternViewColorInfo & PatternView::colorInfo() const {
      return colorInfo_;
    }


    void PatternView::onTick( double sequenceStart ) {
      /*if (pattern_) {
        int liney = d2i ( (Player::Instance()->playPos() - sequenceStart) * pattern_->beatZoom());
        if (liney != playPos_ ) {			
          int oldPlayPos = playPos_;
          playPos_ = liney;
          int startTrack = drawArea->findTrackByScreenX( drawArea->dx() );
          int endTrack = drawArea->findTrackByScreenX( drawArea->dx() + drawArea->clientWidth() );
          window()->repaint( drawArea , drawArea->repaintTrackArea( oldPlayPos, oldPlayPos, startTrack, endTrack ));	
          window()->repaint( drawArea , drawArea->repaintTrackArea( liney,liney, startTrack, endTrack ));
        }
      }*/
    }

    void PatternView::onStartPlayBar() {
      playPos_ = 0;
    }

    void PatternView::onEndPlayBar() {
      playPos_ = -1;
    }

    PatternUndoManager & PatternView::undoManager() {
      return undoManager_;
    }

    void PatternView::setActiveMachineIdx( int idx )
    {
      selectedMacIdx_ = idx;
    }

    int PatternView::selectedMachineIndex( ) const
    {
      return selectedMacIdx_;
    }

    void psy::host::PatternView::onPatternStepChange( ngrs::ItemEvent * ev )
    {
      if (patternCombo_->selIndex()!=-1) {
        setPatternStep(patternCombo_->selIndex()+1);
      }
    }

    void PatternView::PatternDraw::onTagParse( const ngrs::XmlParser & parser, const std::string & tagName  )
    {
      if (tagName == "patsel") {
        xmlTracks = ngrs::str<int>   (parser.getAttribValue("tracks"));
        xmlBeats = ngrs::str<float> (parser.getAttribValue("beats"));
        std::cout << "tracks:" << xmlTracks << std::endl;
        std::cout << "beats:" << xmlBeats << std::endl;
      } else
        if (tagName == "patline") {
          lastXmlLineBeatPos = ngrs::str<float> (parser.getAttribValue("pos"));     
        } else
          if ( tagName == "patevent" && pView->pattern() ) {
            int trackNumber = str_hex<int> (parser.getAttribValue("track"));

            psy::core::PatternEvent data;
            data.setMachine( str_hex<int> (parser.getAttribValue("mac")) );
            data.setInstrument( str_hex<int> (parser.getAttribValue("inst")) );
            data.setNote( str_hex<int> (parser.getAttribValue("note")) );
            data.setParameter( str_hex<int> (parser.getAttribValue("param")) );
            data.setParameter( str_hex<int> (parser.getAttribValue("cmd")) );

            pasteBuffer[lastXmlLineBeatPos].notes()[trackNumber]=data;
          }
    }

  }
}