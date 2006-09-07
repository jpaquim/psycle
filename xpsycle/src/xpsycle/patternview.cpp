/***************************************************************************
  *   Copyright (C) 2006 by Stefan   *
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
#include "global.h"
#include "song.h"
#include "inputhandler.h"
#include "player.h"
#include "machine.h"
#include "defaultbitmaps.h"
#include "zoombar.h"
#include <ngrs/napp.h>
#include <ngrs/nalignlayout.h>
#include <ngrs/nwindow.h>
#include <ngrs/nmenuitem.h>
#include <ngrs/nmenuseperator.h>
#include <ngrs/nframeborder.h>
#include <ngrs/nfontmetrics.h>
#include <ngrs/ntoolbar.h>
#include <ngrs/ncombobox.h>
#include <ngrs/nitem.h>
#include <ngrs/nitemevent.h>
#include <ngrs/nlabel.h>
#include <ngrs/nsystem.h>
#include <ngrs/nsplitbar.h>
#include <ngrs/ncheckbox.h>

namespace psycle { namespace host {


template<class T> inline T str_hex(const std::string &  value) {
   T result;

   std::stringstream str;
   str << value;
   str >> std::hex >> result;

   return result;
}


/// The pattern Main Class , a container for the inner classes LineNumber, Header, and PatternDraw
PatternView::PatternView( Song* song )
  : NPanel()
{
  _pSong = song;
  setLayout(NAlignLayout());

  hBar = 0;
  vBar = 0;
  tweakHBar = 0;
  tweakGUI = 0;
  header = 0;
  drawArea = 0;
	tweakHeader = 0;
  pattern_ = 0;

  vBar = new NScrollBar();
    vBar->setWidth(15);
    vBar->setOrientation(nVertical);
    vBar->change.connect(this,&PatternView::onVScrollBar);
  add(vBar, nAlRight);

  // create the patternview toolbar
  add(toolBar = new NToolBar() ,nAlTop);
  initToolBar();

  // create the left linenumber panel
	NPanel* linePanel = new NPanel();
    linePanel->setLayout( NAlignLayout() );
    NPanel* lineHeaderLabel = new NPanel();
		linePanel->add( lineHeaderLabel, nAlTop );
  	linePanel->add( lineNumber_ = new LineNumber(this), nAlClient);
  add( linePanel, nAlLeft );
	
	// sub group tweaker and patterndraw
	NPanel* drawGroup = new NPanel();
		drawGroup->setLayout( NAlignLayout() );
		// create tweak gui
    tweakGroup = new NPanel();
      tweakGroup->setLayout( NAlignLayout() );
      tweakGroup->add( tweakHeader = new TweakHeader(this), nAlTop);
      tweakHBar = new NScrollBar();
        tweakHBar->setOrientation(nHorizontal);
        tweakHBar->setPreferredSize(100,15);
        tweakHBar->change.connect(this,&PatternView::onHTweakScrollBar);
      tweakGroup->add( tweakHBar, nAlBottom );
      tweakGroup->add( tweakGUI = new TweakGUI(this), nAlClient) ;      
      tweakGroup->setMinimumWidth(1);
		drawGroup->add ( tweakGroup, nAlLeft );
    drawGroup->setPreferredSize( 2*tweakGUI->colWidth() , 100);
		// splitbar
    splitBar = new NSplitBar();
		drawGroup->add( splitBar, nAlLeft );
  	// create the pattern panel
		NPanel* subGroup = new NPanel();
			subGroup->setLayout( NAlignLayout() );
      // create the headertrack panel
      subGroup->add(header      = new Header(this), nAlTop);
			lineHeaderLabel->setPreferredSize( 20, header->preferredHeight() );
      tweakHeader->setPreferredSize( 20, header->preferredHeight() );
			// hbar with beat zoom
  		NPanel* hBarPanel = new NPanel();
    		hBarPanel->setLayout( NAlignLayout() );
    		zoomHBar = new ZoomBar();
    		zoomHBar->setRange(1,100);
    		zoomHBar->setPos(4);
    		zoomHBar->posChanged.connect(this, &PatternView::onZoomHBarPosChanged);
    		hBarPanel->add(zoomHBar, nAlRight);
    		hBar = new NScrollBar();
      		hBar->setOrientation(nHorizontal);
      		hBar->setPreferredSize(100,15);
      		hBar->change.connect(this,&PatternView::onHScrollBar);
    		hBarPanel->add(hBar, nAlClient);
  		subGroup->add(hBarPanel, nAlBottom);
      // create the drawArea
      subGroup->add(drawArea    = new PatternDraw(this), nAlClient);
    drawGroup->add( subGroup, nAlClient );
	add(drawGroup, nAlClient);
	
  setFont(NFont("System",8,nMedium | nStraight ));
  editPosition_ = prevEditPosition_ = playPos_  = outtrack = 0;
  editOctave_ = 4;
  for(int i=0;i<MAX_TRACKS;i++) notetrack[i]=120;

  moveCursorWhenPaste_ = false;  
  selectedMacIdx_ = 255;
}

PatternView::~PatternView()
{
}

void PatternView::setFocus() {
  drawArea->setFocus();
}

void PatternView::onZoomHBarPosChanged(ZoomBar* zoomBar, double newPos) {
  setBeatZoom( (int) newPos );
  repaint();
}

void PatternView::onHScrollBar( NScrollBar * sender )
{
  double pos = sender->pos();
  int newPos = (pos / drawArea->colWidth()) * drawArea->colWidth();

  if (newPos != drawArea->dx()) {
    header->setScrollDx(newPos);
    header->repaint();

    int diffX  = newPos - drawArea->dx();
    if (diffX < drawArea->clientWidth()) {
      NRect rect = drawArea->blitMove(diffX,0, drawArea->absoluteSpacingGeometry());
      drawArea->setDx(newPos);
      window()->repaint(drawArea,rect);
    } else {
      drawArea->setDx(newPos);
      drawArea->repaint(drawArea);
    }
  }
}

void PatternView::onHTweakScrollBar( NScrollBar * sender )
{
  double pos = sender->pos();
  int newPos = (pos / tweakGUI->colWidth()) * tweakGUI->colWidth();

  if (newPos != tweakGUI->dx()) {
    tweakHeader->setScrollDx(newPos);
    tweakHeader->repaint();

    int diffX  = newPos - tweakGUI->dx();
    if (diffX < tweakGUI->clientWidth()) {
      NRect rect = tweakGUI->blitMove(diffX,0, tweakGUI->absoluteSpacingGeometry());
      tweakGUI->setDx(newPos);
      window()->repaint(tweakGUI,rect);
    } else {
      tweakGUI->setDx(newPos);
      tweakGUI->repaint();
    }
  }
}


void PatternView::onVScrollBar( NScrollBar * sender )
{
  double pos = sender->pos();
  if (pos >= 0) {
  int newPos = (pos / rowHeight()) * rowHeight();

  if (newPos != drawArea->dy()) {
    int diffY  = newPos - lineNumber_->dy();
      if (diffY < drawArea->clientHeight()) {
        NRect rect = lineNumber_->blitMove(0,diffY,lineNumber_->absoluteSpacingGeometry());
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
  meterCbx = new NComboBox();
    meterCbx->add(new NItem("4/4"));
    meterCbx->add(new NItem("3/4"));
    meterCbx->setPreferredSize(50,15);
    meterCbx->setIndex(0);
		meterCbx->enableFocus(false);
  toolBar->add(meterCbx);
  NButton* btn = toolBar->add(new NButton("add Bar"));
  btn->clicked.connect(this,&PatternView::onAddBar);
  btn->enableFocus( false);
  btn = toolBar->add(new NButton("delete Bar"));
  btn->clicked.connect(this,&PatternView::onDeleteBar);
  btn->enableFocus(false);
  toolBar->add(new NLabel("Pattern Step"));
  patternCombo_ = new NComboBox();
    for (int i = 1; i <=16; i++) 
      patternCombo_->add(new NItem(stringify(i)));
    patternCombo_->setIndex(0);
    patternCombo_->itemSelected.connect(this,&PatternView::onPatternStepChange);
    patternCombo_->setWidth(40);
    patternCombo_->setHeight(20);
    patternCombo_->enableFocus(false);
  toolBar->add(patternCombo_);

  toolBar->add(new NLabel("Octave"));
  octaveCombo_ = new NComboBox();
    for (int i=0; i<9; i++) octaveCombo_->add(new NItem(stringify(i)));
    octaveCombo_->itemSelected.connect(this,&PatternView::onOctaveChange);
    octaveCombo_->setWidth(40);
    octaveCombo_->setHeight(20);
    octaveCombo_->setIndex(4);
    octaveCombo_->enableFocus(false);
    setEditOctave(4);
  toolBar->add(octaveCombo_);

  toolBar->add(new NLabel("Tracks"));
  trackCombo_ = new NComboBox();
    trackCombo_->setWidth(40);
    trackCombo_->setHeight(20);
    trackCombo_->itemSelected.connect(this,&PatternView::onTrackChange);
    for(int i=4;i<=MAX_TRACKS;i++) {
      trackCombo_->add(new NItem(stringify(i)));
    }
    trackCombo_->setIndex( _pSong->tracks() - 4 );  // starts at 4 .. so 16 - 4 = 12 ^= 16
  toolBar->add(trackCombo_);

  sideBox = new NCheckBox("Tweak left/right");
     sideBox->clicked.connect(this,&PatternView::onSideChange);
  toolBar->add(sideBox);
}

void PatternView::onAddBar( NButtonEvent * ev )
{
  if ( pattern_ ) {
    std::string meter = meterCbx->text();
    int pos = meter.find("/");
    std::string num   = meter.substr( 0, pos);
    std::string denom = meter.substr( pos+1 );

    pattern_->addBar( TimeSignature(str<int>(num), str<int>(denom)) );
    resize();
    repaint();
  }
}

void PatternView::onDeleteBar(NButtonEvent* ev)
{
  if ( pattern_ ) {
    float position = drawArea->cursor().line() / (float) pattern_->beatZoom();
    pattern_->removeBar(position);
    resize();
    repaint();
  }
}

void PatternView::onSideChange( NButtonEvent* ev ) {
   if (sideBox->checked() ) {
     tweakGroup->setAlign(nAlRight);
	   splitBar->setAlign( nAlRight );
   } else {
     tweakGroup->setAlign(nAlLeft);
	   splitBar->setAlign( nAlLeft );
   }
   resize();
   repaint();
}

void PatternView::resize( )
{
  // calls the AlignLayout to reorder scroll-,-header,-linenumber- and patternpanels
  NPanel::resize();
  updateRange();
  
}

void PatternView::setSeparatorColor( const NColor & separatorColor )
{
  separatorColor_ = separatorColor;
}

const NColor & PatternView::separatorColor( )
{
  return separatorColor_;
}

int PatternView::trackNumber( ) const
{
  return _pSong->tracks();
}

int PatternView::rowHeight( ) const
{
  return 12;
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
  return ( pattern_) ? pattern_->beatZoom() * pattern_->beats() : 0;  
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

NScrollBar * PatternView::vScrBar( )
{
  return vBar;
}

NScrollBar * PatternView::hScrBar( )
{
  return hBar;
}

void PatternView::setEditOctave( int octave )
{
  editOctave_ = octave;
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

/// End of PatternView main Class


/// The Header Panel Class
PatternView::Header::Header( PatternView * pPatternView ) : pView(pPatternView) , NPanel()
{
  setSkin();
  setHeight(bgCoords.height());
  skinColWidth_ = bgCoords.width();
  setBackground(Global::pConfig()->pvc_row);
  setTransparent(false);
}

PatternView::Header::~ Header( )
{
}

void PatternView::Header::setSkin( )
{
  bgCoords.setPosition(0,0,109,18);
  noCoords.setPosition(0,18,7,12);
  sRecCoords.setPosition(70,18,11,11);
  dRecCoords.setXY(52,3);
  sMuteCoords.setPosition(81,18,11,11);
  dMuteCoords.setXY(75,3);
  sSoloCoords.setPosition(92,18,11,11);
  dSoloCoords.setXY(97,3);
  dgX0Coords.setXY(24,3);
  dg0XCoords.setXY(31,3);
}

void PatternView::Header::paint( NGraphics * g )
{
  NBitmap & bitmap = Global::pConfig()->icons().pattern_header_skin();

  int startTrack = scrollDx() / pView->colWidth();
  int trackCount = spacingWidth() / pView->colWidth();

  g->setForeground(pView->separatorColor());
  for (int i = startTrack; i <= std::min(startTrack + trackCount ,pView->trackNumber() - 1); i++) {
    const int trackX0 = i/10;
    const int track0X = i%10;
    int xOff = i* pView->colWidth();
    int center = (pView->colWidth() - skinColWidth()) / 2;
    xOff += center;
    g->putBitmap(xOff,0,bgCoords.width(),bgCoords.height(), bitmap, 
                  bgCoords.left(), bgCoords.top());
    g->putBitmap(xOff+dgX0Coords.x(),0+dgX0Coords.y(),noCoords.width(),noCoords.height(), bitmap,
                  trackX0*noCoords.width(), noCoords.top());
    g->putBitmap(xOff+dg0XCoords.x(),0+dg0XCoords.y(),noCoords.width(),noCoords.height(), bitmap,
                  track0X*noCoords.width(), noCoords.top());

    // blit the mute LED
    if (  pView->pSong()->_trackMuted[i]) {
        g->putBitmap(xOff+dMuteCoords.x(),0+dMuteCoords.y(),sMuteCoords.width(),sMuteCoords.height(), bitmap,
                  sMuteCoords.left(), sMuteCoords.top());
    }

    // blit the solo LED
    if ( pView->pSong()->_trackSoloed == i) {
        g->putBitmap(xOff+dSoloCoords.x(),0+dSoloCoords.y(),sSoloCoords.width(),sSoloCoords.height(), bitmap,
                  sSoloCoords.left(), sSoloCoords.top());
    }

    // blit the record LED
    if ( pView->pSong()->_trackArmed[i]) {
        g->putBitmap(xOff+dRecCoords.x(),0+dRecCoords.y(),sRecCoords.width(),sRecCoords.height(), bitmap,
                  sRecCoords.left(), sRecCoords.top());
    }

    if (i!=0) g->drawLine(i*pView->colWidth(),0,i*pView->colWidth(),clientWidth()); // col seperator
  }
}

void PatternView::Header::onMousePress( int x, int y, int button )
{
    // check if left mouse button pressed
  if (button == 1)
  {
      // determine the track column, the mousepress occured on
      int track = x / pView->colWidth();
      // find out the start offset of the header bitmap
      NPoint off(track * pView->colWidth() + (pView->colWidth() - skinColWidth()) / 2,0);
      // the rect area of the solo led
      NRect solo(off.x() + dSoloCoords.x(), off.y() + dSoloCoords.y(), sSoloCoords.width(), sSoloCoords.height());
      // now check point intersection for solo
      if (solo.intersects(x,y)) {
          onSoloLedClick(track);
      } else {
        // the rect area of the solo led
        NRect mute(off.x() + dMuteCoords.x(), off.y() + dMuteCoords.y(), sMuteCoords.width(), sMuteCoords.height());
        // now check point intersection for solo
        if (mute.intersects(x,y)) {
            onMuteLedClick(track);
        } else
        {
            // the rect area of the record led
            NRect record(off.x() + dRecCoords.x(), off.y() + dRecCoords.y(), sRecCoords.width(), sRecCoords.height());
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
  if ( pView->pSong()->_trackSoloed != track )
  {
    for ( int i=0;i<MAX_TRACKS;i++ ) {
      pView->pSong()->_trackMuted[i] = true;
    }
    pView->pSong()->_trackMuted[track] = false;
    pView->pSong()->_trackSoloed = track;
  }
  else
  {
    for ( int i=0;i<MAX_TRACKS;i++ )
    {
      pView->pSong()->_trackMuted[i] = false;
    }
    pView->pSong()->_trackSoloed = -1;
  }
  repaint();
}

void PatternView::Header::onMuteLedClick( int track )
{
  pView->pSong()->_trackMuted[track] = !(pView->pSong()->_trackMuted[track]);
  repaint();
}

void PatternView::Header::onRecLedClick(int track) {
  pView->pSong()->_trackArmed[track] = ! pView->pSong()->_trackArmed[track];
  pView->pSong()->_trackArmedCount = 0;
  for ( int i=0;i<MAX_TRACKS;i++ ) {
      if ( pView->pSong()->_trackArmed[i] )
      {
        pView->pSong()->_trackArmedCount++;
      }
  }
  repaint();
}

int PatternView::Header::preferredWidth( )
{
  return (pView->trackNumber())*pView->colWidth();
}

int PatternView::Header::skinColWidth( )
{
  return skinColWidth_;
}

/// End of Header Class


///
/// The Line Number Panel Class
///
  PatternView::LineNumber::LineNumber( PatternView * pPatternView ) : NPanel(), dy_(0)
  {
    setBorder( NFrameBorder() );
    pView = pPatternView;
    setWidth(60);
    setBackground(Global::pConfig()->pvc_row);
    setTransparent(false);
  }

  PatternView::LineNumber::~ LineNumber( )
  {
  }

  void PatternView::LineNumber::paint( NGraphics * g )
  {
    TimeSignature signature;

		NRect repaintRect = g->repaintArea().rectClipBox();
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
      g->drawLine(0,(i+1)*pView->rowHeight() - dy() -1,
                  clientWidth(),(i+1)*pView->rowHeight() -1 - dy() );

    g->setForeground(pView->separatorColor());
    g->drawLine(0,0,0,clientHeight() );

      for (int i = startLine; i <= endLine; i++) {
        //if (i+startLine == pView->cursor().line()) {
        //  g->setForeground(Global::pConfig()->pvc_cursor);
        //  g->fillRect(0,i*pView->rowHeight()+pView->headerHeight(),clientWidth()-1,pView->rowHeight());
        //}
        std::string text = stringify( i );
        if ( pView->pattern() ) {
          float position = i  / (float) pView->pattern()->beatZoom();
          SinglePattern::iterator it = pView->pattern()->find_nearest(i);
          if (it != pView->pattern()->end()) {
            // check out how many hidden lines there are
            int lastLine = d2i (it->first * pView->pattern_->beatZoom());
            int y = lastLine;
            SinglePattern::iterator it2 = it;
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
              int xOff = clientWidth()-g->textWidth(text)- 10 ;
              int yOff = i*pView->rowHeight()+pView->rowHeight() -1 - 3 -dy();
              g->drawLine( xOff , yOff+1, xOff, yOff - pView->rowHeight() + 5);
              if (it->first < position) {
                g->drawLine( xOff , yOff - pView->rowHeight() + 5, xOff-3, yOff - pView->rowHeight() + 8);
                g->drawLine( xOff , yOff - pView->rowHeight() + 5, xOff+4, yOff - pView->rowHeight() + 9);
              } else {
                 g->drawLine( xOff , yOff +1, xOff-3, yOff - 2);
                g->drawLine(  xOff , yOff +1, xOff+4, yOff - 3);
              }
            }
          }
          if ( pView->pattern()->barStart(position, signature) ) {
            std::string caption = stringify(signature.numerator())+"/"+stringify(signature.denominator());
            g->drawText(0,i*pView->rowHeight()+pView->rowHeight()-1 - dy(),caption);
          }
        }
        g->drawText(clientWidth()-g->textWidth(text)-3,i*pView->rowHeight()+pView->rowHeight()-1-dy(),text);
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

void PatternView::TweakHeader::paint( NGraphics* g ) {

  NBitmap & bitmap = Global::pConfig()->icons().tweakHeader();

  int startTrack = scrollDx() / pView->tweakColWidth();
  int trackCount = spacingWidth() / pView->tweakColWidth();

  g->setForeground(pView->separatorColor());
  for (int i = startTrack; i <= std::min(startTrack + trackCount ,pView->trackNumber() - 1); i++) {
    int parameterIndex = 0;
    int machineIndex   = 0xFF;    
		if ( pView->pattern() ) {
      TweakTrackInfo info = pView->pattern()->tweakInfo( i );
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
		g->putBitmap(xOff,0,bgCoords.width(),bgCoords.height(), bitmap, 
                  bgCoords.left(), bgCoords.top());

    g->putBitmap(xOff+14,3,noCoords.width(),noCoords.height(), bitmap,
                  digit1_X0*noCoords.width(), noCoords.top());
    g->putBitmap(xOff+22,3,noCoords.width(),noCoords.height(), bitmap,
                  digit1_0X*noCoords.width(), noCoords.top());  

    g->putBitmap(xOff+23 + 14,3,noCoords.width(),noCoords.height(), bitmap,
                  digit2_X0*noCoords.width(), noCoords.top());
    g->putBitmap(xOff+23 + 22,3,noCoords.width(),noCoords.height(), bitmap,
                  digit2_0X*noCoords.width(), noCoords.top());  

    if (i!=0) g->drawLine(i*pView->tweakColWidth(),0,i*pView->tweakColWidth(),clientWidth()); // col seperator
  }
  
}

int PatternView::TweakHeader::preferredWidth() {
  return (16)*pView->tweakColWidth();  
}


PatternView::TweakGUI::TweakGUI( PatternView* pPatternView)
{
  pView = pPatternView;
  addEvent( ColumnEvent::hex4 );
  setMinimumWidth(1);
  setTransparent(false);
  setBackground(Global::pConfig()->pvc_row);
}

PatternView::TweakGUI::~TweakGUI() {

}

void PatternView::TweakGUI::customPaint(NGraphics* g, int startLine, int endLine, int startTrack, int endTrack) {
  if (pView->pattern()) {
    TimeSignature signature;

    g->setForeground(Global::pConfig()->pvc_rowbeat);

    int trackWidth = ((endTrack+1) * colWidth()) - dx();
    int lineHeight = ((endLine +1) * rowHeight()) - dy();

    for (int y = startLine; y <= endLine; y++) {
      float position = y / (float) beatZoom();
      if (!(y == pView->playPos()) /*|| pView->editPosition() != Global::pPlayer()->_playPosition*/) {
      if ( !(y % beatZoom())) {
          if ((pView->pattern()->barStart(position, signature) )) {
              g->setForeground(Global::pConfig()->pvc_row4beat);
              g->fillRect(0, y*rowHeight() - dy(),trackWidth, rowHeight());
              g->setForeground(Global::pConfig()->pvc_rowbeat);
          } else {
            g->fillRect(0, y* rowHeight() - dy(),trackWidth, rowHeight());
          }
        }
      } else  {
        g->setForeground(Global::pConfig()->pvc_playbar);
        g->fillRect(0, y*rowHeight() - dy(), trackWidth, rowHeight());
        g->setForeground(Global::pConfig()->pvc_rowbeat);
      }
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

void PatternView::TweakGUI::onKeyPress(const NKeyEvent & event) {
   CustomPatternView::onKeyPress( event );

		switch ( event.scancode() ) {		
		case ' ':
			if (Player::Instance()->_playing) {
				Player::Instance()->Stop();
			} else {
				Player::Instance()->Start(0);
			}
		break;
		case XK_Page_Up:
		{
			TimeSignature signature;
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
		case XK_Page_Down:
		{
			TimeSignature signature;
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
		case XK_Left :
			checkLeftScroll( cursor() );
      return;
		break;
		case XK_Right:
			checkRightScroll( cursor() );
			return;
		break;
    case XK_Down:
			pView->checkDownScroll( cursor() );
      return;
    break;
    case XK_Up:
			pView->checkUpScroll( cursor() );
      return;
    break;
		case XK_End:
      pView->checkDownScroll( cursor() );
      return;
    break;
    case XK_Home:
      pView->checkUpScroll( cursor() );
      return;
    break;
		case XK_Tab:
			checkRightScroll( cursor() );
			return;
		break;
		case XK_ISO_Left_Tab:
			checkLeftScroll( cursor() );
      return;
		break;
		case XK_BackSpace:
			if ( !pView->pattern()->lineIsEmpty( cursor().line() ) ) {
					pView->pattern()->clearTweakTrack( cursor().line(), cursor().track() );
			}
			moveCursor(0,-1); 
			pView->checkUpScroll( cursor() );
			return;
		break;
    case XK_Delete:
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
			PatternEvent patEvent = pView->pattern()->tweakEvent( cursor().line(), cursor().track() );			
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

void PatternView::TweakGUI::drawPattern(NGraphics* g, int startLine, int endLine, int startTrack, int endTrack) {
	if ( pView->pattern() ) {
		drawCellBg(g, cursor(), Global::pConfig()->pvc_cursor );

		SinglePattern::iterator it = pView->pattern_->find_lower_nearest(startLine);

		int lastLine = -1;
		for ( ; it != pView->pattern_->end(); it++ ) {
			PatternLine & line = it->second;
			int y = d2i (it->first * pView->pattern_->beatZoom());
			if (y > endLine) break;
			if (y != lastLine) {
				PatternLine::iterator eventIt = line.tweaks().lower_bound(startTrack);
				for(; eventIt != line.tweaks().end() && eventIt->first <= endTrack; ++eventIt) {
					PatternEvent event = eventIt->second;
					int x = eventIt->first;
					if (event.command() != 0 || event.parameter() != 0) {
						drawData( g, x, y, 0, (event.command() << 8) | event.parameter() );
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
	if ( (cursor.track()) * colWidth() - dx() < 0) {
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

  setTransparent(false);
  setBackground(Global::pConfig()->pvc_row);

  pView = pPatternView;
  editPopup_ = new NPopupMenu();
  add(editPopup_);
    editPopup_->add(new NMenuItem("Undo"));
    editPopup_->add(new NMenuItem("Redo"));
    editPopup_->add(new NMenuSeperator());
    NMenuItem* blockCutItem_ = new NMenuItem("Block cut");
      blockCutItem_->click.connect(this,&PatternView::PatternDraw::onPopupBlockCut);
    editPopup_->add(blockCutItem_);
    NMenuItem* blockCopyItem_ = new NMenuItem("Block copy");
      blockCopyItem_->click.connect(this,&PatternView::PatternDraw::onPopupBlockCopy);
    editPopup_->add(blockCopyItem_);
    NMenuItem* blockPasteItem_ = new NMenuItem("Block paste");
      blockPasteItem_->click.connect(this,&PatternView::PatternDraw::onPopupBlockPaste);
    editPopup_->add(blockPasteItem_);
    NMenuItem* blockPasteMixItem_ = new NMenuItem("Block mix paste");
      blockPasteMixItem_->click.connect(this,&PatternView::PatternDraw::onPopupBlockMixPaste);
    editPopup_->add(blockPasteMixItem_);

    NMenuItem* blockDelItem = new NMenuItem("Block delete");
        blockDelItem->click.connect(this,&PatternView::PatternDraw::onPopupBlockDelete);
    editPopup_->add(blockDelItem);

    editPopup_->add(new NMenuSeperator());
    editPopup_->add(new NMenuItem("Interpolate Effect"));
    editPopup_->add(new NMenuItem("Change Generator"));
    editPopup_->add(new NMenuItem("Change Instrument"));
    editPopup_->add(new NMenuSeperator());

    NMenuItem* blockT1Item = new NMenuItem("Transpose+1");
        blockT1Item->click.connect(this,&PatternView::PatternDraw::onPopupTranspose1);
    editPopup_->add(blockT1Item);

    NMenuItem* blockT_1Item = new NMenuItem("Transpose-1");
        blockT_1Item->click.connect(this,&PatternView::PatternDraw::onPopupTranspose_1);
    editPopup_->add(blockT_1Item);

    NMenuItem* blockT12Item = new NMenuItem("Transpose12");
        blockT12Item->click.connect(this,&PatternView::PatternDraw::onPopupTranspose12);
    editPopup_->add(blockT12Item);

    NMenuItem* blockT_12Item = new NMenuItem("Transpose-12");
        blockT_12Item->click.connect(this,&PatternView::PatternDraw::onPopupTranspose_12);
    editPopup_->add(blockT_12Item);

    editPopup_->add(new NMenuSeperator());

    editPopup_->add(new NMenuItem("Block Swing Fill"));
    editPopup_->add(new NMenuItem("Block Track Fill"));
    editPopup_->add(new NMenuSeperator());
    NMenuItem* blockPatPropItem_ = new NMenuItem("Pattern properties");
      blockPatPropItem_->click.connect(this,&PatternView::PatternDraw::onPopupPattern);
    editPopup_->add(blockPatPropItem_);

    patDlg = new PatDlg();
}

PatternView::PatternDraw::~ PatternDraw( )
{
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

int PatternView::PatternDraw::trackNumber() const {
	return pView->trackNumber();
}

int PatternView::PatternDraw::beatZoom() const {
	return pView->beatZoom();
}

void PatternView::PatternDraw::customPaint(NGraphics* g, int startLine, int endLine, int startTrack, int endTrack)
{
  if (pView->pattern()) {
    TimeSignature signature;

    g->setForeground(Global::pConfig()->pvc_rowbeat);

    int trackWidth = ((endTrack+1) * colWidth()) - dx();
    int lineHeight = ((endLine +1) * rowHeight()) - dy();

    for (int y = startLine; y <= endLine; y++) {
      float position = y / (float) beatZoom();
      if (!(y == pView->playPos()) /*|| pView->editPosition() != Global::pPlayer()->_playPosition*/) {
      if ( !(y % beatZoom())) {
          if ((pView->pattern()->barStart(position, signature) )) {
              g->setForeground(Global::pConfig()->pvc_row4beat);
              g->fillRect(0, y*rowHeight() - dy(),trackWidth, rowHeight());
              g->setForeground(Global::pConfig()->pvc_rowbeat);
          } else {
            g->fillRect(0, y* rowHeight() - dy(),trackWidth, rowHeight());
          }
        }
      } else  {
        g->setForeground(Global::pConfig()->pvc_playbar);
        g->fillRect(0, y*rowHeight() - dy(), trackWidth, rowHeight());
        g->setForeground(Global::pConfig()->pvc_rowbeat);
      }
    }

		drawSelBg(g,selection());		
		drawColumnGrid(g, startLine, endLine, startTrack, endTrack);
    drawTrackGrid(g, startLine, endLine, startTrack, endTrack);	
		drawPattern(g, startLine, endLine, startTrack, endTrack);
		drawRestArea(g, startLine, endLine, startTrack, endTrack);
		
  }
}

void PatternView::PatternDraw::drawPattern( NGraphics * g, int startLine, int endLine, int startTrack, int endTrack )
{
	if ( pView->pattern() ) {
		drawCellBg(g, cursor(), Global::pConfig()->pvc_cursor );

		SinglePattern::iterator it = pView->pattern_->find_lower_nearest(startLine);

		int lastLine = -1;
		for ( ; it != pView->pattern_->end(); it++ ) {
			PatternLine & line = it->second;
			int y = d2i (it->first * pView->pattern_->beatZoom());
			if (y > endLine) break;
			if (y != lastLine) {
				PatternLine::iterator eventIt = line.lower_bound(startTrack);
				for(; eventIt != line.end() && eventIt->first <= endTrack; ++eventIt) {
					PatternEvent event = eventIt->second;
					int x = eventIt->first;
					drawData( g, x, y, 0, event.note() );
					if (event.instrument() != 255) drawData( g, x, y, 1, event.instrument() );
					if (event.machine() != 255) drawData( g, x, y, 2, event.machine() );
					if (event.volume() != 255) drawData( g, x, y, 3, event.volume() );
					if (event.command() != 0 || event.parameter() != 0) {
						drawData( g, x, y, 4, (event.command() << 8) | event.parameter() );
					}
					lastLine = y;
				}
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

void PatternView::PatternDraw::onKeyPress( const NKeyEvent & event )
{
  if ( !pView->pattern() ) return;
	CustomPatternView::onKeyPress( event );

	switch ( event.scancode() ) {		
		case ' ':
			if (Player::Instance()->_playing) {
				Player::Instance()->Stop();
			} else {
				Player::Instance()->Start(0);
			}
		break;
		case XK_Page_Up:
		{
			TimeSignature signature;
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
		case XK_Page_Down:
		{
			TimeSignature signature;
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
		case XK_Left :
			checkLeftScroll( cursor() );
      return;
		break;
		case XK_Right:
			checkRightScroll( cursor() );
			return;
		break;
    case XK_Down:
			pView->checkDownScroll( cursor() );
      return;
    break;
    case XK_Up:
			pView->checkUpScroll( cursor() );
      return;
    break;
		case XK_End:
      pView->checkDownScroll( cursor() );
      return;
    break;
    case XK_Home:
      pView->checkUpScroll( cursor() );
      return;
    break;
		case XK_Tab:
			checkRightScroll( cursor() );
			return;
		break;
		case XK_ISO_Left_Tab:
			checkLeftScroll( cursor() );
      return;
		break;
		case XK_BackSpace:
			clearCursorPos();
			moveCursor(0,-1); 
			pView->checkUpScroll( cursor() );
			return;
		break;
    case XK_Delete:
      clearCursorPos();
      moveCursor(0,1); 
			pView->checkDownScroll( cursor() );
		default: ;
	}
				

	if ( cursor().eventNr() == 0 ) {
		// a note event
		int note = Global::pConfig()->inputHandler.getEnumCodeByKey(Key(0,event.scancode()));
		if ( note == cdefKeyStop ) {
			pView->noteOffAny( cursor() );
		} else
		if (note >=0 && note < 120) {
			pView->enterNote( cursor(), note );
			moveCursor(0,1);
			pView->checkDownScroll( cursor() );
		}
	} else
	if ( isHex(event.scancode()) ) {
		if ( cursor().eventNr() == 1 ) {
			// inst select
			PatternEvent patEvent = pView->pattern()->event( cursor().line(), cursor().track() );
			unsigned char newByte = convertDigit( 0xFF, event.scancode(), patEvent.instrument(), cursor().col() );
			patEvent.setInstrument( newByte );
			pView->pattern()->setEvent( cursor().line(), cursor().track(), patEvent );
      if (cursor().col() == 0)
			 moveCursor(1,0);			
      else
       moveCursor(-1,1);
       pView->checkDownScroll( cursor() );
		} else 
		if ( cursor().eventNr() == 2) {
			// mac select
			PatternEvent patEvent = pView->pattern()->event( cursor().line(), cursor().track() );
			unsigned char newByte = convertDigit( 0xFF, event.scancode(), patEvent.machine(), cursor().col() );
			patEvent.setMachine( newByte );
			pView->pattern()->setEvent( cursor().line(), cursor().track(), patEvent );
      if (cursor().col() == 0)
			 moveCursor(1,0);			
      else
       moveCursor(-1,1);
			 pView->checkDownScroll( cursor() );
		} else
		
		if ( cursor().eventNr() == 3) {
			// mac select
			PatternEvent patEvent = pView->pattern()->event( cursor().line(), cursor().track() );
			unsigned char newByte = convertDigit( 0xFF, event.scancode(), patEvent.volume(), cursor().col() );
			patEvent.setVolume( newByte );
			pView->pattern()->setEvent( cursor().line(), cursor().track(), patEvent );
      if (cursor().col() == 0)
			 moveCursor(1,0);			
      else
       moveCursor(-1,1);
			 pView->checkDownScroll( cursor() );
		} else
		if ( cursor().eventNr() == 4) {
			// comand or parameter
			PatternEvent patEvent = pView->pattern()->event( cursor().line(), cursor().track() );
			if (cursor().col() < 2 ) {
				unsigned char newByte = convertDigit( 0x00, event.scancode(), patEvent.command(), cursor().col() );
				patEvent.setCommand( newByte );
				pView->pattern()->setEvent( cursor().line(), cursor().track(), patEvent );
        moveCursor(1,0);
			}
			else {
				unsigned char newByte = convertDigit( 0x00, event.scancode(), patEvent.parameter(), cursor().col() - 2 );
				patEvent.setParameter( newByte );
				pView->pattern()->setEvent( cursor().line(), cursor().track(), patEvent );
        if (cursor().col() < 3)
					moveCursor(1,0);			
				else
					moveCursor(-3,1);
				pView->checkDownScroll( cursor() );
			}			
		}
	}
}

void PatternView::PatternDraw::clearCursorPos() {
		if ( !pView->pattern()->lineIsEmpty( cursor().line() ) ) {
				PatternEvent patEvent = pView->pattern()->event( cursor().line(), cursor().track() );
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
	if ( (cursor.track()) * colWidth() - dx() < 0) {
		pView->hBar->setPos( std::max( cursor.track(), 0) * colWidth() );
	}	
}

void PatternView::PatternDraw::checkRightScroll( const PatCursor & cursor ) {
	//check for scroll
	if ( (cursor.track()+1) * colWidth() - dx() > clientWidth() ) {
		pView->hBar->setPos( std::min( cursor.track()+1 , trackNumber()) * colWidth() - clientWidth() );
	}
}

int PatternView::PatternDraw::doSel(const PatCursor & selCursor) {
  int dir = CustomPatternView::doSel( selCursor );

	pView->checkDownScroll( selCursor );
	pView->checkUpScroll( selCursor );
	checkLeftScroll( selCursor );
  checkRightScroll( selCursor );
	
  return dir;
}

void PatternView::enterNote( const PatCursor & cursor, int note ) {
 if ( pattern() ) {
   PatternEvent event = pattern()->event( cursor.line(), cursor.track() );
   Machine* tmac = pSong()->_pMachine[ pSong()->seqBus ];
   event.setNote( editOctave() * 12 + note );
   if (tmac) event.setMachine( tmac->_macIndex );
   if (tmac && tmac->_type == MACH_SAMPLER ) {
     event.setInstrument( pSong()->instSelected );
	 }
   pattern()->setEvent( cursor.line(), cursor.track(), event );
   if (tmac) PlayNote( editOctave() * 12 + note, 127, false, tmac);   
 }
}

void PatternView::repaintLineNumber( int startLine, int endLine ) {

}

NRect PatternView::repaintLineNumberArea(int startLine, int endLine)
{
  int top    = startLine    * rowHeight()  + drawArea->absoluteTop()  - lineNumber_->dy();
  int bottom = (endLine+3)  * rowHeight()  + drawArea->absoluteTop()  - lineNumber_->dy();

  return NRect(lineNumber_->absoluteLeft(),top,lineNumber_->clientWidth(),bottom - top);
}

void PatternView::PatternDraw::onPopupBlockCopy( NButtonEvent * ev )
{
  copyBlock(false);
}

void PatternView::PatternDraw::onPopupBlockCut( NButtonEvent * ev )
{
  copyBlock(true);
}

void PatternView::PatternDraw::copyBlock( bool cutit )
{  
	isBlockCopied=true;
	pasteBuffer.clear();
	SinglePattern copyPattern = pView->pattern()->block( selection().left(), selection().right(), selection().top(), selection().bottom() );
	
	float start = selection().top()    / (float) pView->pattern()->beatZoom();
	float end   = selection().bottom() / (float) pView->pattern()->beatZoom();

  std::string xml = "<patsel beats='" + stringify( end - start ); 
	xml+= "' tracks='"+ stringify( selection().right() - selection().left() );
	xml+= "'>"; 
	xml+= copyPattern.toXml();
  xml+= "</patsel>";

	NApp::system().clipBoard().setAsText( xml );

	if (cutit) {
		pView->pattern()->deleteBlock( selection().left(), selection().right(), selection().top(), selection().bottom() );
		pView->repaint();
	}
}

void PatternView::PatternDraw::onPopupBlockDelete( NButtonEvent * ev )
{
  deleteBlock();
  repaint();
}

void PatternView::PatternDraw::onPopupBlockMixPaste( NButtonEvent * ev )
{
  pasteBlock( cursor().track(), cursor().line(),true);

  if (pView->moveCursorWhenPaste()) {
/*       if (pView->cursor().y()+ blockNLines < pView->lineNumber() ) {
          pView->setCursor(NPoint3D(pView->cursor().x(),pView->cursor().y()+blockNLines,pView->cursor().z()));
       } else
       pView->setCursor(NPoint3D(pView->cursor().x(),pView->lineNumber()-1,pView->cursor().z()));*/
  }
  pView->repaint();
}

void PatternView::PatternDraw::onPopupBlockPaste( NButtonEvent * ev )
{
  pasteBlock( cursor().track(), cursor().line(), false);

  if (pView->moveCursorWhenPaste()) {
/*       if (pView->cursor().y()+ blockNLines < pView->lineNumber() ) {
          pView->setCursor(NPoint3D(pView->cursor().x(),pView->cursor().y()+blockNLines,pView->cursor().z()));
       } else
       pView->setCursor(NPoint3D(pView->cursor().x(),pView->lineNumber()-1,pView->cursor().z()));*/
  }
  pView->repaint();
}

void PatternView::PatternDraw::pasteBlock(int tx,int lx,bool mix,bool save)
{
  if ( NApp::system().clipBoard().asText() != "" ) {
    NXmlParser parser;
		lastXmlLineBeatPos = 0.0;
		xmlTracks = 0;
		xmlBeats = 0;
    parser.tagParse.connect(this, &PatternView::PatternDraw::onTagParse);
    parser.parseString( NApp::system().clipBoard().asText() );
	
		if (!mix)
			pView->pattern()->copyBlock(tx,lx,pasteBuffer,xmlTracks,xmlBeats);
		else
			pView->pattern()->mixBlock(tx,lx,pasteBuffer,xmlTracks,xmlBeats);
  }
}

void PatternView::updatePlayBar(bool followSong)
{
/*  if ( ((NVisualComponent*) parent())->visible() && (Global::pPlayer()->_lineChanged) && (editPosition() == Global::pPlayer()->_playPosition) && !followSong )
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
    // stop any notes with the same value
    //StopNote(note,bTranspose,pMachine);

    if(note<0) return;

    // octave offset
    if(note<120) {
        if(bTranspose)
        note+=pSong()->currentOctave*12;
        if (note > 119)
        note = 119;
      }

      // build entry
      PatternEvent entry;
      entry.setNote( note );
      entry.setInstrument( pSong()->auxcolSelected );
      entry.setMachine( pSong()->seqBus );	// Not really needed.

      if(velocity != 127 && Global::pConfig()->midi().velocity().record())
      {
          int par = Global::pConfig()->midi().velocity().from() + (Global::pConfig()->midi().velocity().to() - Global::pConfig()->midi().velocity().from()) * velocity / 127;
          if (par > 255) par = 255; else if (par < 0) par = 0;
          switch(Global::pConfig()->midi().velocity().type())
          {
            case 0:
              entry.setCommand( Global::pConfig()->midi().velocity().command() );
              entry.setParameter( par );
            break;
            case 3:
              entry.setInstrument( par );
            break;
          }
      } else
      {
          entry.setCommand( 0 );
          entry.setParameter( 0 );
      }

      // play it
      if(pMachine==NULL)
      {
        int mgn = pSong()->seqBus;

        if (mgn < MAX_MACHINES) {
            pMachine = pSong()->_pMachine[mgn];
        }
      }

      if (pMachine) {
        // pick a track to play it on	
//        if(bMultiKey)
        {
          int i;
          for (i = outtrack+1; i < pSong()->tracks(); i++)
          {
            if (notetrack[i] == 120) {
              break;
            }
          }
          if (i >= pSong()->tracks()) {
            for (i = 0; i <= outtrack; i++) {
                if (notetrack[i] == 120) {
                  break;
                }
            }
          }
          outtrack = i;
        }// else  {
          // outtrack=0;
        // }
        // this should check to see if a note is playing on that track
        if (notetrack[outtrack] < 120) {
            //StopNote(notetrack[outtrack], bTranspose, pMachine);
        }

        // play
        notetrack[outtrack]=note;
        pMachine->Tick(outtrack, entry );
      }
}

void PatternView::PatternDraw::transposeBlock(int trp)
{
	int right = selection().right();
	int left =  selection().left();
	double top = selection().top() / (double)pView->beatZoom();
	double bottom = selection().bottom() / (double)pView->beatZoom();

	pView->pattern()->transposeBlock(left, right, top, bottom, trp);

	window()->repaint(this,repaintTrackArea(selection().top(),selection().bottom(),left,right));
//   }
}


void PatternView::noteOffAny( const PatCursor & cursor )
{
  if (pattern_) {
    PatternEvent event;
    event.setNote(120);
    pattern()->setEvent( cursor.line(), cursor.track(), event );
		drawArea->repaintCursorPos( cursor );    
  }
}

void PatternView::PatternDraw::onPopupPattern( NButtonEvent * ev )
{
}

void PatternView::PatternDraw::onPopupTranspose1( NButtonEvent * ev )
{
	transposeBlock(1);
}

void PatternView::PatternDraw::onPopupTranspose12( NButtonEvent * ev )
{
	transposeBlock(12);
}

void PatternView::PatternDraw::onPopupTranspose_1( NButtonEvent * ev )
{
	transposeBlock(-1);
}

void PatternView::PatternDraw::onPopupTranspose_12( NButtonEvent * ev )
{
	transposeBlock(-12);
}


void PatternView::PatternDraw::onKeyRelease(const NKeyEvent & event) {
  CustomPatternView::onKeyRelease( event );
	if ( !pView->pattern() ) return;

  if ( cursor().eventNr() == 0 ) {
    int outnote = Global::pConfig()->inputHandler.getEnumCodeByKey(Key(0,event.scancode()));
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
}

int PatternView::patternStep() const {
  return drawArea->patternStep();
}

void PatternView::StopNote( int note, bool bTranspose, Machine * pMachine )
{
  if (!(note >=0 && note < 128)) return;

  // octave offset
  if(note<120) {
      if(bTranspose) note+=pSong()->currentOctave*12;
      if (note > 119) note = 119;
  }

  if(pMachine==NULL) {
      int mgn = pSong()->seqBus;

      if (mgn < MAX_MACHINES) {
          pMachine = pSong()->_pMachine[mgn];
      }

  for(int i=0; i<pSong()->tracks(); i++) {
      if(notetrack[i]==note) {
        notetrack[i]=120;
        // build entry
        PatternEvent entry;
        entry.setNote( 120+0 );
        entry.setInstrument( pSong()->auxcolSelected );
        entry.setMachine( pSong()->seqBus );
        entry.setCommand( 0 );
        entry.setParameter( 0 );

        // play it

        if (pMachine) {
          pMachine->Tick( i, entry );
        }
      }
    }

  }
}

void PatternView::PatternDraw::scaleBlock(float factor )
{
	int right = selection().right();
	int left =  selection().left();
	double top = selection().top() / (double)pView->beatZoom();
	double bottom = selection().bottom() / (double)pView->beatZoom();
	pView->pattern()->scaleBlock(left, right, top, bottom, factor);
	window()->repaint(this,repaintTrackArea(selection().top(),selection().bottom()*std::max(factor,1.0f),left,right));
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

void PatternView::setPattern( SinglePattern * pattern )
{
  pattern_ = pattern;
  resize();
}

SinglePattern * PatternView::pattern( )
{
  return pattern_;
}

Song * PatternView::pSong( )
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

void PatternView::onOctaveChange( NItemEvent * ev )
{
  setEditOctave( str<int> ( ev->item()->text() ) );
}

void PatternView::onTrackChange( NItemEvent * ev )
{
  pSong()->setTracks( str<int>( ev->item()->text() ) );
  //if (cursor().track() >= pSong()->tracks() )
  //{
  //  setCursor( PatCursor( pSong()->tracks() ,cursor().line(),0,0) );
  //}
  repaint();
}
}}

void psycle::host::PatternView::setActiveMachineIdx( int idx )
{
  selectedMacIdx_ = idx;
}

int psycle::host::PatternView::selectedMachineIndex( ) const
{
  return selectedMacIdx_;
}

void psycle::host::PatternView::onPatternStepChange( NItemEvent * ev )
{
  if (patternCombo_->selIndex()!=-1) {
      setPatternStep(patternCombo_->selIndex()+1);
  }
}

void psycle::host::PatternView::PatternDraw::onTagParse( const NXmlParser & parser, const std::string & tagName  )
{
	if (tagName == "patsel") {
		xmlTracks = str<int>   (parser.getAttribValue("tracks"));
		xmlBeats = str<float> (parser.getAttribValue("beats"));
		std::cout << "tracks:" << xmlTracks << std::endl;
		std::cout << "beats:" << xmlBeats << std::endl;
	} else
  if (tagName == "patline") {
			lastXmlLineBeatPos = str<float> (parser.getAttribValue("pos"));     
	} else
	if ( tagName == "patevent" && pView->pattern() ) {
		int trackNumber = str_hex<int> (parser.getAttribValue("track"));

		PatternEvent data;
		data.setMachine( str_hex<int> (parser.getAttribValue("mac")) );
		data.setInstrument( str_hex<int> (parser.getAttribValue("inst")) );
		data.setNote( str_hex<int> (parser.getAttribValue("note")) );
		data.setParameter( str_hex<int> (parser.getAttribValue("param")) );
		data.setParameter( str_hex<int> (parser.getAttribValue("cmd")) );

		pasteBuffer[lastXmlLineBeatPos][trackNumber]=data;
	}
}
