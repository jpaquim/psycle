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
#include <ngrs/napp.h>
#include <ngrs/nalignlayout.h>
#include <ngrs/nwindow.h>
#include <ngrs/nmenuitem.h>
#include <ngrs/nmenuseperator.h>
#include <ngrs/nframeborder.h>
#include <ngrs/nfontmetrics.h>


/// The pattern Main Class , a container for the inner classes LineNumber, Header, and PatternDraw
PatternView::PatternView()
  : NPanel()
{
  setLayout(NAlignLayout());

  // create scrollBars
  hBar = new NScrollBar();
    hBar->setOrientation(nHorizontal);
    hBar->setHeight(15);
    hBar->posChange.connect(this,&PatternView::onHScrollBar);
  add(hBar, nAlBottom);

  vBar = new NScrollBar();
    vBar->setWidth(15);
    vBar->setOrientation(nVertical);
    vBar->posChange.connect(this,&PatternView::onVScrollBar);
  add(vBar, nAlRight);


  // create the left linenumber panel
  add(lineNumber_ = new LineNumber(this), nAlLeft);
  // create the headertrack panel
  add(header      = new Header(this), nAlTop);
  // create the pattern panel
  add(drawArea    = new PatternDraw(this), nAlClient);

  setFont(NFont("System",8,nMedium | nStraight ));

  addEvent(1); // add one byte event to trackerline
  addEvent(1); // add one byte event to trackerline
  addEvent(2); // add two byte event to trackerline

  editPosition_ = prevEditPosition_ = playPos_ = editOctave_ = outtrack = 0;

  for(int i=0;i<MAX_TRACKS;i++) notetrack[i]=120;

  setTransparent(false);

  patternStep_ = 1;
}


PatternView::~PatternView()
{
}

void PatternView::onHScrollBar( NObject * sender, int pos )
{
  int newPos = (pos / colWidth()) * colWidth();

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

void PatternView::onVScrollBar( NObject * sender, int pos )
{
  if (pos >= 0) {
  int newPos = (pos / rowHeight()) * rowHeight();

  if (newPos != drawArea->dy()) {
    int diffY  = newPos - lineNumber_->dy();
      if (diffY < drawArea->clientHeight()) {
        NRect rect = lineNumber_->blitMove(0,diffY,NRect(lineNumber_->absoluteSpacingLeft(),lineNumber_->absoluteSpacingTop()+headerHeight(),lineNumber_->spacingWidth(),lineNumber_->spacingHeight()-headerHeight()));
        lineNumber_->setDy(newPos);
        if (diffY < 0) rect.setHeight(rect.height()+headerHeight());
        window()->repaint(lineNumber_,rect);
        rect = drawArea->blitMove(0,diffY,drawArea->absoluteSpacingGeometry());
        drawArea->setDy(newPos);
        window()->repaint(drawArea,rect);
    } else {
        lineNumber_->setDy(newPos);
        drawArea->setDy(newPos);
        lineNumber_->repaint();
        drawArea->repaint();
    }
  }
  }
}

void PatternView::resize( )
{
  // calls the AlignLayout to reorder scroll-,-header,-linenumber- and patternpanels
  NPanel::resize();
  // set the Range to the new headerwidth
  hBar->setRange(header->preferredWidth() - clientWidth());
  int count = (drawArea->clientHeight()-headerHeight()) / rowHeight();
  vBar->setRange((lineNumber()-1-count)*rowHeight());
}

void PatternView::setSeparatorColor( const NColor & separatorColor )
{
  separatorColor_ = separatorColor;
}

const NColor & PatternView::separatorColor( )
{
  return separatorColor_;
}

int PatternView::trackNumber( )
{
  return Global::pSong()->SONGTRACKS;
}

int PatternView::rowHeight( )
{
  return 12;
}

int PatternView::headerHeight( )
{
  return header->height()-1;
}

int PatternView::colWidth( )
{
  int col = noteCellWidth();
  for (std::vector<int>::iterator it = eventSize.begin(); it < eventSize.end(); it++) {
    switch (*it) {
    case 1: col+=2*cellWidth();
    break;
    case 2: col+=4*cellWidth();
    break;
    }
  }

  return std::max(header->skinColWidth(),col);
}

int PatternView::noteCellWidth( )
{
  NFontMetrics metrics(font());
  int width = metrics.textWidth("C#-10");
  return width;
}

int PatternView::cellWidth( )
{
  //NFontMetrics metrics(font());
  //int width = metrics.textWidth("_");
  return 12;
}

int PatternView::lineNumber( )
{
  return Global::pSong()->patternLines[Global::pSong()->playOrder[editPosition()]];

}

int PatternView::playPos( )
{
  return playPos_;
}

void PatternView::setPrevEditPosition( int pos )
{
  prevEditPosition_ = pos;
}

int PatternView::prevEditPosition( )
{
  return prevEditPosition_;
}

const NPoint3D & PatternView::cursor( ) const
{
  return cursor_;
}

void PatternView::setCursor( const NPoint3D & cursor )
{
  cursor_ = cursor;
}

void PatternView::moveCursor( int dx, int dy, int dz )
{
  int newX = std::min(std::max(cursor_.x()+dx,0),trackNumber()-1);
  int newY = std::min(std::max(0,cursor_.y()+dy),lineNumber()-1);

  int newZ = cursor_.z()+dz;

  if (newZ >= cellCount() ) {
      newX++;
      if (newX > trackNumber()-1) {
        newX = trackNumber()-1;
        newZ = cursor_.z();
      }
      else 
        newZ=0;
  } else

  if (newZ < 0 ) {
      newX--;
      if (newX < 0) {
        newX = 0;
        newZ = 0;
      }
      else
        newZ= cellCount()-1;
  }

  cursor_.setXYZ(newX,newY,newZ);
}

void PatternView::addEvent( int byteLength )
{
  eventSize.push_back(byteLength);
}

void PatternView::setEditPosition( int pos )
{
  editPosition_ = pos;
  int count = (drawArea->clientHeight()-headerHeight()) / rowHeight();
  vBar->setRange((lineNumber()-1-count)*rowHeight());
}

int PatternView::editPosition( )
{
  return editPosition_;
}

std::string PatternView::noteToString( int value )
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

int PatternView::editOctave( )
{
  return editOctave_;
}


/// End of PatternView main Class













/// The Header Panel Class
PatternView::Header::Header( PatternView * pPatternView ) : pView(pPatternView) , NPanel()
{
  bitmap = Global::pConfig()->icons().pattern_header_skin();
  setSkin();
  setHeight(bgCoords.height());
  skinColWidth_ = bgCoords.width();
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
    if (Global::pSong()->_trackMuted[i]) {
        g->putBitmap(xOff+dMuteCoords.x(),0+dMuteCoords.y(),sMuteCoords.width(),sMuteCoords.height(), bitmap,
                  sMuteCoords.left(), sMuteCoords.top());
    }

    // blit the solo LED
    if (Global::pSong()->_trackSoloed == i) {
        g->putBitmap(xOff+dSoloCoords.x(),0+dSoloCoords.y(),sSoloCoords.width(),sSoloCoords.height(), bitmap,
                  sSoloCoords.left(), sSoloCoords.top());
    }

    // blit the record LED
    if (Global::pSong()->_trackArmed[i]) {
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
  if (Global::pSong()->_trackSoloed != track )
  {
    for ( int i=0;i<MAX_TRACKS;i++ ) {
      Global::pSong()->_trackMuted[i] = true;
    }
    Global::pSong()->_trackMuted[track] = false;
    Global::pSong()->_trackSoloed = track;
  }
  else
  {
    for ( int i=0;i<MAX_TRACKS;i++ )
    {
      Global::pSong()->_trackMuted[i] = false;
    }
    Global::pSong()->_trackSoloed = -1;
  }
  repaint();
}

void PatternView::Header::onMuteLedClick( int track )
{
  Global::pSong()->_trackMuted[track] = !(Global::pSong()->_trackMuted[track]);
  repaint();
}

void PatternView::Header::onRecLedClick(int track) {
  Global::pSong()->_trackArmed[track] = ! Global::pSong()->_trackArmed[track];
  Global::pSong()->_trackArmedCount = 0;
  for ( int i=0;i<MAX_TRACKS;i++ ) {
      if (Global::pSong()->_trackArmed[i])
      {
        Global::pSong()->_trackArmedCount++;
      }
  }
  repaint();
}

int PatternView::Header::preferredWidth( )
{
  return (pView->trackNumber()+2)*pView->colWidth();
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
    setBorder(NFrameBorder());
    pView = pPatternView;
    setWidth(40);
  }

  PatternView::LineNumber::~ LineNumber( )
  {
  }

  void PatternView::LineNumber::paint( NGraphics * g )
  {
    int startLine = dy_ / pView->rowHeight();
    int rDiff   = g->repaintArea().rectClipBox().top() - absoluteTop() + pView->headerHeight();
    int offT = rDiff / pView->rowHeight();
    if (offT < 0) offT = 0;
    offT = 0;
    int offB = (rDiff+g->repaintArea().rectClipBox().height()) / pView->rowHeight();
    if (offB < 0) offB = 0;
    int count = std::min(clientHeight() / pView->rowHeight(),offB);

    for (int i = offT; i < count; i++)
      g->drawLine(0,i*pView->rowHeight()+pView->headerHeight(),
                  clientWidth(),i*pView->rowHeight()+pView->headerHeight());

    g->setForeground(pView->separatorColor());
    g->drawLine(0,0,0,clientHeight());
    g->drawLine(clientWidth()-1,0,clientWidth()-1,clientHeight());

      for (int i = offT; i < count; i++) {
        if (i+startLine == pView->cursor().y()) {
          g->setForeground(Global::pConfig()->pvc_cursor);
          g->fillRect(0,i*pView->rowHeight()+pView->headerHeight(),clientWidth()-1,pView->rowHeight());
        }
        g->drawText(clientWidth()-g->textWidth(stringify(i+startLine))-3,i*pView->rowHeight()+pView->rowHeight()+pView->headerHeight()-1,stringify(i+startLine));
      }

    g->drawText(1,pView->headerHeight()-1,"Line");
  }

  void PatternView::LineNumber::setDy( int dy )
  {
    dy_ = dy;
  }

  int PatternView::LineNumber::dy( )
  {
    return dy_;
  }

/// End of Line Number Panel














///
///
/// The PatternDraw class, that displays the data
///
///


PatternView::PatternDraw::PatternDraw( PatternView * pPatternView ) : dx_(0),dy_(0),doDrag_(0),doSelect_(0),NPanel()
{
  setName("debug::patternDraw");
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

void PatternView::PatternDraw::paint( NGraphics * g )
{
  NPoint lineArea = linesFromRepaint(g->repaintArea());
  int startLine = lineArea.x();
  int endLine   = lineArea.y();

  NPoint trackArea = tracksFromRepaint(g->repaintArea());
  int startTrack = trackArea.x();
  int endTrack   = trackArea.y();

  g->setForeground(Global::pConfig()->pvc_rowbeat);

  int trackWidth = ((endTrack+1) * pView->colWidth()) - dx();
  int lineHeight = ((endLine +1) * pView->rowHeight()) - dy();

  for (int y = startLine; y <= endLine; y++) {
    if (!(y == pView->playPos()) || pView->editPosition() != Global::pPlayer()->_playPosition) {
      if ( !(y % Global::pSong()->LinesPerBeat())) {
        if (!(y%(Global::pSong()->LinesPerBeat()*Global::pConfig()->pv_timesig))) {
            g->setForeground(Global::pConfig()->pvc_row4beat);
            g->fillRect(0,y*pView->rowHeight() - dy_,trackWidth,pView->rowHeight());
            g->setForeground(Global::pConfig()->pvc_rowbeat);
        } else {
          g->fillRect(0,y*pView->rowHeight() - dy_,trackWidth,pView->rowHeight());
        }
      }
    } else  {
      g->setForeground(Global::pConfig()->pvc_playbar);
      g->fillRect(0,y*pView->rowHeight() - dy_,trackWidth,pView->rowHeight());
      g->setForeground(Global::pConfig()->pvc_rowbeat);
    }
  }

  drawSelBg(g,selection_);

  g->setForeground(pView->foreground());

  for (int y = startLine; y <= endLine; y++)
    g->drawLine(0,y*pView->rowHeight() - dy_,trackWidth,y*pView->rowHeight()-dy_);

  for (int i = startTrack; i <= endTrack; i++) // 3px space at begin of trackCol
      g->fillRect(i*pView->colWidth()-dx_,0,3,lineHeight);

  g->setForeground(pView->separatorColor());
  for (int i = startTrack; i <= endTrack; i++)  // col separators
      g->drawLine(i*pView->colWidth()-dx_,0,i*pView->colWidth()-dx_,lineHeight);

  g->setForeground(pView->foreground());

  for (int x = startTrack; x <= endTrack; x++) {
      int COL = pView->noteCellWidth();
      for (std::vector<int>::iterator it = pView->eventSize.begin(); it < pView->eventSize.end(); it++) {
      switch (*it) {
      case 1:
            g->drawLine(x*pView->colWidth()+COL-dx_,0,x*pView->colWidth()+COL-dx_,lineHeight);
            COL+=2 * pView->cellWidth();
      break;
      case 2:
            g->drawLine(x*pView->colWidth()+COL-dx_,0,x*pView->colWidth()+COL-dx_,lineHeight);
            COL+=4 * pView->cellWidth();
      break;
      }
    }
  }

  drawPattern(g,startLine,endLine,startTrack,endTrack);

  g->setForeground(NColor(0,0,80));
  int endTop     = pView->lineNumber() * pView->rowHeight() - dy();
  int endHeight  = std::max(0, clientHeight() - endTop);
  g->fillRect(0,endTop,clientWidth(),endHeight);

  int endLeft     = pView->trackNumber() * pView->colWidth() - dx();
  int endWidth    = std::max(0, clientWidth() - endLeft);
  g->fillRect(endLeft,0,endWidth,clientHeight());
}

void PatternView::PatternDraw::setDy( int dy )
{
  dy_ = dy;
}

int PatternView::PatternDraw::dy( )
{
  return dy_;
}

void PatternView::PatternDraw::setDx( int dx )
{
  dx_ = dx;
}

int PatternView::PatternDraw::dx( )
{
  return dx_;
}


void PatternView::PatternDraw::drawText(NGraphics* g, int track, int line, int eventOffset, const std::string & text )
{
  int xOff = track * pView->colWidth()+3 + eventOffset        - dx_;
  int yOff = line  * pView->rowHeight() + pView->rowHeight()  - dy_;

  g->drawText(xOff,yOff,text);
}

void PatternView::PatternDraw::drawData( NGraphics * g, int track, int line, int eventOffset, const std::string & text )
{
  int xOff = track * pView->colWidth()+3 + eventOffset        - dx_;
  int yOff = line  * pView->rowHeight() + pView->rowHeight()  - dy_;

  int col = 0;
  for (int i = 0; i < text.length(); i++) {
      g->drawText(xOff + col,yOff,text.substr(i,1));
      col += pView->cellWidth();
  }
}

void PatternView::PatternDraw::drawCellBg( NGraphics * g, int track, int line, int col, const NColor & bgColor )
{
  int xOff = track * pView->colWidth()+3 - dx_;
  int yOff = line  * pView->rowHeight()  - dy_;

  int cellWidth = pView->noteCellWidth()-3;
  int colOffset = 0;

  if (col>0) {
    cellWidth = pView->cellWidth();
    colOffset = pView->noteCellWidth()-3 + (col-1)*pView->cellWidth();
  }

  g->setForeground(bgColor);
  g->fillRect(xOff + colOffset,yOff, cellWidth,pView->rowHeight());
}


void PatternView::PatternDraw::drawPattern( NGraphics * g, int startLine, int endLine, int startTrack, int endTrack )
{
  // draw Cursor bg;

  drawCellBg(g,pView->cursor().x(),pView->cursor().y(),pView->cursor().z(),Global::pConfig()->pvc_cursor );

  char tbuf[16];
  for (int y = startLine; y <= endLine; y++) {
      unsigned char *patOffset = Global::pSong()->_ppattern(Global::pSong()->playOrder[pView->editPosition_]) + (y*MULTIPLY) + (startTrack)*5;
    for (int x = startTrack; x <= endTrack; x++) {
      drawText(g, x,y,0,pView->noteToString(*patOffset));
      patOffset++;
      int COL = pView->noteCellWidth();
      for (std::vector<int>::iterator it = pView->eventSize.begin(); it < pView->eventSize.end(); it++) {
      int value = *patOffset;
      switch (*it) {
        case 1  :{
                  sprintf(tbuf,"%.2X",value); 
                  if (value!=255) {
                    drawData(g,x,y,COL,tbuf);
                  }
                  COL+=2 * pView->cellWidth();
                  patOffset++;
                  } break;
        case 2  :{
                  if (*patOffset == 0 && *(patOffset+1) == 0 && (*(patOffset-3) <= 120 || *(patOffset-3) == 255 )) 
                  { patOffset+=2;} else {
                    sprintf(tbuf,"%.2X",value);
                    patOffset++;
                    int value = *patOffset;
                    sprintf(&tbuf[2],"%.2X",value);
                    if (value!=255) drawData(g,x,y,COL,tbuf);
                    patOffset++;
                  }
                  COL+=4 * pView->cellWidth();
                  } break;
      }
      }
    }
  }
}


void PatternView::PatternDraw::onMousePress( int x, int y, int button )
{
  if (button == 6) {
      std::cout << "wheel mouse scroll left" << std::endl;
      // wheel mouse scroll left
      int startTrack  = dx_ / pView->colWidth();
      int newTrack = std::max(0,startTrack-1);
      pView->hScrBar()->setPos( (newTrack) * pView->colWidth());
  } else
  if (button == 7) {
    // wheel mouse scrolling right
      std::cout << "wheel mouse scroll right" << std::endl;
      int startTrack  = dx_ / pView->colWidth();
      int newTrack = std::max(0,std::min(pView->trackNumber(),startTrack+1));
      pView->hScrBar()->setPos( (newTrack) * pView->colWidth());
  } else
  if (button == 4) {
      // wheel mouse scroll up
      int startLine  = dy_ / pView->rowHeight();
      int newLine = std::max(0,startLine-1);
      pView->vScrBar()->setPos( (newLine) * pView->rowHeight());
  } else
  if (button == 5) {
    // wheel mouse scrolling down
      int startLine  = dy_ / pView->rowHeight();
      int newLine = std::max(0,std::min(pView->lineNumber(),startLine+1));
      pView->vScrBar()->setPos( (newLine) * pView->rowHeight());
  } else 
  if (button == 2) {
    // todo linux paste
  } else
  if (button == 3) {
      editPopup_->setPosition(x + absoluteLeft(), y + 10 + absoluteTop(),100,100);
      editPopup_->setVisible(true);
  } else {
    clearOldSelection();
    NPoint3D p = intersectCell(x,y);
    startSel(p);
  }
}

void PatternView::PatternDraw::onMouseOver( int x, int y )
{
  if (doDrag_) {
    NPoint3D p = intersectCell(x,y);
    doSel(p);
  }
}

char inline hex_value(char c) { if(c >= 'A') return 10 + c - 'A'; else return c - '0'; }

void PatternView::PatternDraw::onMousePressed( int x, int y, int button )
{
  if (button==1) {
    if (!doSelect_) pView->setCursor(intersectCell(x,y));
    pView->repaint();
    endSel();
  }
}

void PatternView::PatternDraw::onKeyPress( const NKeyEvent & event )
{
  if (doDrag_ != (NApp::system().keyState() & ShiftMask) && 
                  !(NApp::system().keyState() & ControlMask)) {
      if (!doDrag_) {
        clearOldSelection();
        startSel(pView->cursor());
        selCursor = pView->cursor();
        selCursor.setZ(0);
      }
  }

  switch (event.scancode()) {
    case XK_BackSpace :
        pView->clearCursorPos();
    break;
    case XK_Left: {
        if (NApp::system().keyState() & ShiftMask) {
          // selMode
          selCursor.setX(std::max(selCursor.x()-1,0));
          doSel(selCursor);
        } else {
          int oldTrack = pView->cursor().x();
          pView->moveCursor(0,0,-1);
          int newTrack = pView->cursor().x();
          int startTrack  = dx_ / pView->colWidth();
          if (newTrack < startTrack) {
              pView->hScrBar()->setPos( (newTrack) * pView->colWidth());
          }
        window()->repaint(this,repaintTrackArea(pView->cursor().y(),pView->cursor().y(),newTrack,oldTrack));
        }
    }
    break;
    case XK_Right: {
        if (NApp::system().keyState() & ShiftMask) {
          // selMode
          selCursor.setX(std::min(selCursor.x()+1,pView->trackNumber()-1));
          doSel(selCursor);
        } else {
          int oldTrack = pView->cursor().x();
          pView->moveCursor(0,0,1);
          int newTrack    = pView->cursor().x();
          int trackCount  = clientWidth() / pView->colWidth();
          int startTrack  = dx_ / pView->colWidth();
          if (newTrack > startTrack + trackCount -1) {
            pView->hScrBar()->setPos( (startTrack+2) * pView->colWidth());
          }
          window()->repaint(this,repaintTrackArea(pView->cursor().y(),pView->cursor().y(),oldTrack,newTrack));
        }
      }
      break;
      case XK_Down : {
        if (NApp::system().keyState() & ShiftMask) {
          // selMode
          selCursor.setY(std::min(selCursor.y()+1,pView->lineNumber()-1));
          doSel(selCursor);
        } else {
          int startLine  = dy_ / pView->rowHeight();
          int lineCount  = clientHeight() / pView->rowHeight();
          int oldLine = pView->cursor().y();
          pView->moveCursor(0,pView->patternStep(),0);
          int newLine = pView->cursor().y();
          if (newLine > startLine + lineCount-1) {
            pView->vScrBar()->setPos( (startLine+2) * pView->rowHeight());
          }
            window()->repaint(this,repaintTrackArea(oldLine,newLine,pView->cursor().x(),pView->cursor().x()));
            window()->repaint(pView,pView->repaintLineNumberArea(oldLine,newLine));

        }
      }
      break;
      case XK_Up: {
        if (NApp::system().keyState() & ShiftMask) {
          // selMode
          selCursor.setY(std::max(selCursor.y()-1,0));
          doSel(selCursor);
        } else {
          int oldLine = pView->cursor().y();
          pView->moveCursor(0,-pView->patternStep(),0);
          int startLine  = dy_ / pView->rowHeight();
          int newLine = pView->cursor().y();
          if (newLine < startLine) {
            pView->vScrBar()->setPos( (newLine) * pView->rowHeight());
          }
          window()->repaint(this,repaintTrackArea(newLine,oldLine,pView->cursor().x(),pView->cursor().x()));
          window()->repaint(pView,pView->repaintLineNumberArea(newLine,oldLine));
        }
      }
      break;
      case XK_Page_Up: {
        int lines = (Global::pSong()->LinesPerBeat()*Global::pConfig()->pv_timesig);

        int oldLine = pView->cursor().y();
        pView->moveCursor(0,-lines,0);
        int startLine  = dy_ / pView->rowHeight();
        int newLine = pView->cursor().y();
        if (newLine <= startLine) {
            pView->vScrBar()->setPos( (newLine) * pView->rowHeight());
        }
        window()->repaint(this,repaintTrackArea(newLine,oldLine,pView->cursor().x(),pView->cursor().x()));
        window()->repaint(pView,pView->repaintLineNumberArea(newLine,oldLine));
      }
      break;
      case XK_Page_Down:{
        int lines = (Global::pSong()->LinesPerBeat()*Global::pConfig()->pv_timesig);

        int startLine  = dy_ / pView->rowHeight();
        int lineCount  = clientHeight() / pView->rowHeight();
        int oldLine = pView->cursor().y();
        pView->moveCursor(0,lines,0);
        int newLine = pView->cursor().y();
        if (newLine > startLine + lineCount-1) {
            pView->vScrBar()->setPos( (startLine+2) * pView->rowHeight());
        }
        window()->repaint(this,repaintTrackArea(oldLine,newLine,pView->cursor().x(),pView->cursor().x()));
        window()->repaint(pView,pView->repaintLineNumberArea(oldLine,newLine));
      }
      break;
      default: {
          switch
            (Global::pConfig()->inputHandler.getEnumCodeByKey(Key(NApp::system().keyState(),event.scancode()))) 
          {
            case cdefBlockCopy :
                copyBlock(false);
            break;
            case cdefBlockDelete :
                deleteBlock();
                repaint();
            break;
            case cdefBlockPaste :
                pasteBlock(pView->cursor().x(),pView->cursor().y(),false);
            break;
            case cdefBlockMix :
                pasteBlock(pView->cursor().x(),pView->cursor().y(),true);
            break;
            case cdefTransposeBlockInc:
                blockTranspose(1);
            break;
            case cdefTransposeBlockInc12:
                blockTranspose(12);
            break;
            case cdefTransposeBlockDec:
                blockTranspose(-1);
            break;
            case cdefTransposeBlockDec12:
                blockTranspose(-12);
            break;
            case cdefBlockDouble:
                doubleLength();
                repaint();
            break;

          default: {
              if (event.buffer()!="") {
              int note = Global::pConfig()->inputHandler.getEnumCodeByKey(Key(0,event.scancode()));

              if (note == cdefKeyStop && pView->cursor().z()==0) pView->noteOffAny(); else
              {
                  unsigned char *patOffset = Global::pSong()->_ppattern(Global::pSong()->playOrder[pView->editPosition_]) + (pView->cursor().y() *MULTIPLY) + (pView->cursor().x())*5;

                  if (pView->cursor().z()==0) {
                    note += pView->editOctave()*12;
                    *patOffset = note;

                    Machine *tmac = Global::pSong()->_pMachine[Global::pSong()->seqBus];
                    if (tmac) {
                      //pView->PlayNote(note,127,false,tmac); 
                    }

                    int startLine  = dy_ / pView->rowHeight();
                    int lineCount  = clientHeight() / pView->rowHeight();
                    int oldLine = pView->cursor().y();
                    pView->moveCursor(0,1,0);
                    int newLine = pView->cursor().y();
                    if (newLine > startLine + lineCount-1) {
                    pView->vScrBar()->setPos( (startLine+2) * pView->rowHeight());
                  }
                  window()->repaint(this,repaintTrackArea(oldLine,newLine,pView->cursor().x(),pView->cursor().x()));
                  window()->repaint(pView,pView->repaintLineNumberArea(oldLine,newLine));
                } else  {
                  int off = (pView->cursor().z()+1) / 2;
                  patOffset +=off;
                  unsigned char newByte;
                  if (pView->cursor().z() % 2 == 1) 
                    newByte = (*patOffset & 0x0F) | (0xF0 & (hex_value(event.scancode()) << 4));
                  else
                    newByte = (*patOffset & 0xF0) | (0x0F & (hex_value(event.scancode())));

                  if (*patOffset == 255) {
                    // set to 0
                      newByte = (*patOffset & 0x00) | (0xF0 & (hex_value(event.scancode()) << 4));
                  }
                  *patOffset = newByte;

                    int eventIdx      =  pView->eventFromCol(pView->cursor().z());
                    int eventLen      =  pView->eventLength(eventIdx);
                    int eventColStart =  pView->colStartFromEvent(eventIdx);

                    int startLine  = dy_ / pView->rowHeight();
                    int lineCount  = clientHeight() / pView->rowHeight();
                    int oldLine = pView->cursor().y();

                    if (pView->cursor().z() - eventColStart >= 2*eventLen - 1) {
                      pView->moveCursor(0,1,0);
                      pView->setCursor(NPoint3D(pView->cursor().x(),pView->cursor().y(),eventColStart));
                    } else
                        pView->moveCursor(0,0,1);

                    int newLine = pView->cursor().y();
                    if (newLine > startLine + lineCount-1) {
                      pView->vScrBar()->setPos( (startLine+2) * pView->rowHeight());
                    }
                    window()->repaint(this,repaintTrackArea(oldLine,newLine,pView->cursor().x(),pView->cursor().x()));
                    window()->repaint(pView,pView->repaintLineNumberArea(oldLine,newLine));
                }
            }
          }
      }}}
      break;
  }
}


NRect PatternView::PatternDraw::repaintTrackArea( int startLine, int endLine, int startTrack, int endTrack )
{
  int top    = startLine    * pView->rowHeight()  + absoluteTop()  - dy_;
  int bottom = (endLine+1)  * pView->rowHeight()  + absoluteTop()  - dy_;
  int left   = startTrack   * pView->colWidth()   + absoluteLeft() - dx_;
  int right  = (endTrack+1) * pView->colWidth()   + absoluteLeft() - dx_;

  return NRect(left,top,right - left,bottom - top);
}

NRect PatternView::repaintLineNumberArea(int startLine, int endLine)
{
  int top    = startLine    * rowHeight()  + drawArea->absoluteTop()  - drawArea->dy();
  int bottom = (endLine+1)  * rowHeight()  + drawArea->absoluteTop()  - drawArea->dy();

  return NRect(lineNumber_->absoluteLeft(),top,lineNumber_->clientWidth(),bottom - top);
}

NPoint PatternView::PatternDraw::linesFromRepaint( const NRegion & repaintArea )
{
  // p.x = startLine,  // p.y = endLine (including endline number)

  NRect repaintRect = repaintArea.rectClipBox();

  int absTop  = absoluteTop();
  int ch      = clientHeight();

  // the start for whole repaint
  int start    = dy_ / pView->rowHeight();
  // the offset for the repaint expose request
  int startOff = std::max((repaintRect.top() - absTop) / pView->rowHeight(),(long)0);
  // the start
  start        = std::min(start + startOff, pView->lineNumber()-1);


  // the endline for whole repaint
  int end     = (dy_ + ch) / pView->rowHeight();
  // the offset for the repaint expose request
  int endOff  = std::max((ch-(repaintRect.top()-absTop + repaintRect.height())) / pView->rowHeight()
                        ,(long)0);
  // the end
  end         = std::min(end - endOff, pView->lineNumber()-1);

  return NPoint(start,end);
}

NPoint PatternView::PatternDraw::tracksFromRepaint( const NRegion & repaintArea )
{
  // p.x = startTrack,  // p.y = endTrack (including endline number)

  NRect repaintRect = repaintArea.rectClipBox();

  int absLeft = absoluteLeft();
  int cw      = clientWidth();

  // the start for whole repaint
  int start    = dx_ / pView->colWidth();
  // the offset for the repaint expose request
  int startOff = std::max((repaintRect.left() - absLeft) / pView->colWidth(),(long)0);
  // the start
  start        = std::min(start + startOff, pView->trackNumber()-1);


  // the endtrack for whole repaint
  int end     = (dx_ + cw) / pView->colWidth();
  // the offset for the repaint expose request
  int endOff  = std::max((cw-(repaintRect.left()-absLeft + repaintRect.width())) / pView->colWidth()
                        ,(long)0);
  // the end
  end         = std::min(end - endOff, pView->trackNumber()-1);

  return NPoint(start,end);
}

int PatternView::cellCount( )
{
  int count = 1;
  for (std::vector<int>::iterator it = eventSize.begin(); it < eventSize.end(); it++)
    switch (*it) {
        case 1  : count+=2; break;
        case 2  : count+=4; break;
    }
  return count;
}


void PatternView::PatternDraw::drawSelBg( NGraphics * g, const NSize & selArea )
{
  int x1Off = selArea.left() * pView->colWidth()  ;
  int y1Off = selArea.top()  * pView->rowHeight() ;

  int x2Off = selArea.right()  * pView->colWidth() ;
  int y2Off = selArea.bottom() * pView->rowHeight();

  g->setForeground(Global::pConfig()->pvc_selection);
  g->fillRect(x1Off - dx_,y1Off -dy_,x2Off-x1Off,y2Off-y1Off);

}

NPoint3D PatternView::PatternDraw::intersectCell( int x, int y )
{
  int track = (x + dx_) / pView->colWidth();
  int line  = (y + dy_) / pView->rowHeight();

  int colOff   = (x + dx_) -  (track*pView->colWidth() + pView->noteCellWidth() );
  int cell = 0;
  if (colOff >= 0) cell = colOff / pView->cellWidth() + 1;

  return NPoint3D(track,line,cell);
}

void PatternView::PatternDraw::clearOldSelection( )
{
  NRect r = repaintTrackArea(selection_.top(),selection_.bottom(),selection_.left(),selection_.right());
  selection_.setSize(0,0,0,0);
  window()->repaint(this,r);
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
  // UNDO CODE HERE CUT
  //if(blockSelected)
  {
    isBlockCopied=true;
    blockNTracks= (selection_.right()  - selection_.left());
    blockNLines = (selection_.bottom() - selection_.top());
    blockLastOrigin = selection_;

    int ps = Global::pSong()->playOrder[pView->editPosition()];

    int ls=0;
    int ts=0;

    PatternEntry blank;

    if (cutit) {
      //AddUndo(ps,blockSel.start.track,blockSel.start.line,blockNTracks,blockNLines,editcur.track,editcur.line,editcur.col,editPosition);
    }
    for (int t = selection_.left(); t< selection_.right(); t++)
    {
      ls=0;
      for (int l= selection_.top(); l < selection_.bottom(); l++) {
        unsigned char *offset_target=blockBufferData+(ts*EVENT_SIZE+ls*MULTIPLY);
        unsigned char *offset_source=Global::pSong()->_ptrackline(ps,t,l);
        memcpy(offset_target,offset_source,EVENT_SIZE);
        if(cutit)
            memcpy(offset_source,&blank,EVENT_SIZE);
            ++ls;
        }
        ++ts;
    }
    if (cutit) repaint();
  }
}

void PatternView::PatternDraw::onPopupBlockDelete( NButtonEvent * ev )
{
  deleteBlock();
  repaint();
}

void PatternView::PatternDraw::onPopupBlockMixPaste( NButtonEvent * ev )
{
  pasteBlock(pView->cursor().x(),pView->cursor().y(),true);
}

void PatternView::PatternDraw::onPopupBlockPaste( NButtonEvent * ev )
{
  pasteBlock(pView->cursor().x(),pView->cursor().y(),false);
}

void PatternView::PatternDraw::pasteBlock(int tx,int lx,bool mix,bool save)
{
  //if(isBlockCopied)
  {
    int ps = Global::pSong()->playOrder[pView->editPosition()];
    int nl = Global::pSong()->patternLines[ps];

    // UNDO CODE PASTE AND MIX PASTE
    //if (save) AddUndo(ps,tx,lx,blockNTracks,nl,editcur.track,editcur.line,editcur.col,editPosition);

    int ls=0;
    int ts=0;

    //added by sampler. There is a problem. The paste action can be undo but the lines are not reverted back.
    if (blockNLines > nl) 
        //if (MessageBox("Do you want to autoincrease this pattern lines?","Block doesn't fit in current pattern",MB_YESNO) == IDYES)
        {
          //pSong()->patternLines[ps] = blockNLines;
          //nl = blockNLines;
        }
        //end of added by sampler

      for (int t=tx;t<tx+blockNTracks && t< Global::pSong()->SONGTRACKS;t++)
      {
          ls=0;
          for (int l=lx;l<lx+blockNLines && l<nl;l++)
          {
            unsigned char* offset_source=blockBufferData+(ts*EVENT_SIZE+ls*MULTIPLY);
            unsigned char* offset_target=Global::pSong()->_ptrackline(ps,t,l);
            if ( mix ) {
              if (*offset_target == 0xFF) *(offset_target)=*offset_source;
              if (*(offset_target+1)== 0xFF) *(offset_target+1)=*(offset_source+1);
              if (*(offset_target+2)== 0xFF) *(offset_target+2)=*(offset_source+2);
              if (*(offset_target+3)== 0) *(offset_target+3)=*(offset_source+3);
              if (*(offset_target+4)== 0) *(offset_target+4)=*(offset_source+4);
            } else {
              memcpy(offset_target,offset_source,EVENT_SIZE);
            }
            ++ls;
          }
          ++ts;
        }
      //if (Global::pInputHandler->bMoveCursorPaste)
      //{
        //if (lx+blockNLines < nl ) editcur.line = lx+blockNLines;
        //else editcur.line = nl-1;
        //}

        //bScrollDetatch=false;
        //NewPatternDraw(tx,tx+blockNTracks-1,lx,lx+blockNLines-1);
        //Repaint(DMData);
        repaint();
    }
}

void PatternView::updatePlayBar(bool followSong)
{
  if ( ((NVisualComponent*) parent())->visible() && (Global::pPlayer()->_lineChanged) && (editPosition() == Global::pPlayer()->_playPosition) && !followSong )
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

  }
}


void PatternView::PlayNote(int note,int velocity,bool bTranspose,Machine*pMachine)
{
    // stop any notes with the same value
    //StopNote(note,bTranspose,pMachine);

    if(note<0) return;

    // octave offset
    if(note<120) {
        if(bTranspose)
        note+=Global::pSong()->currentOctave*12;
        if (note > 119)
        note = 119;
      }

      // build entry
      PatternEntry entry;
      entry._note = note;
      entry._inst = Global::pSong()->auxcolSelected;
      entry._mach = Global::pSong()->seqBus;	// Not really needed.

      if(velocity != 127 && Global::pConfig()->midi().velocity().record())
      {
          int par = Global::pConfig()->midi().velocity().from() + (Global::pConfig()->midi().velocity().to() - Global::pConfig()->midi().velocity().from()) * velocity / 127;
          if (par > 255) par = 255; else if (par < 0) par = 0;
          switch(Global::pConfig()->midi().velocity().type())
          {
            case 0:
              entry._cmd = Global::pConfig()->midi().velocity().command();
              entry._parameter = par;
            break;
            case 3:
              entry._inst = par;
            break;
          }
      } else
      {
          entry._cmd=0;
          entry._parameter=0;
      }

      // play it
      if(pMachine==NULL)
      {
        int mgn = Global::pSong()->seqBus;

        if (mgn < MAX_MACHINES) {
            pMachine = Global::pSong()->_pMachine[mgn];
        }
      }

      if (pMachine) {
        // pick a track to play it on	
//        if(bMultiKey)
        {
          int i;
          for (i = outtrack+1; i < Global::pSong()->SONGTRACKS; i++)
          {
            if (notetrack[i] == 120) {
              break;
            }
          }
          if (i >= Global::pSong()->SONGTRACKS) {
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
        pMachine->Tick(outtrack,&entry);
      }
}



void PatternView::PatternDraw::blockTranspose(int trp)
{
    // UNDO CODE TRANSPOSE
//   if ( blockSelected == true ) 
    {
        int ps = Global::pSong()->playOrder[pView->editPosition()];

        //AddUndo(ps,blockSel.start.track,blockSel.start.line,blockSel.end.track-blockSel.start.track+1,blockSel.end.line-blockSel.start.line+1,editcur.track,editcur.line,editcur.col,editPosition);

      for (int t=selection_.left(); t < selection_.right() ; t++) {
        for (int l=selection_.top(); l < selection_.bottom() ; l++) {
          unsigned char *toffset= Global::pSong()->_ptrackline(ps,t,l);

          int note =*(toffset);

          if(note < 120){
            note+=trp;
            if(note < 0)  note = 0;
            if(note > 119)note = 119;
            *toffset=static_cast<unsigned char>(note);
          }
        }
      }

      window()->repaint(this,repaintTrackArea(selection_.top(),selection_.bottom(),selection_.left(),selection_.right()));
    }
}


void PatternView::noteOffAny()
{
  const int ps = Global::pSong()->playOrder[editPosition()];
  unsigned char * offset = Global::pSong()->_ptrack(ps,cursor_.x());
  unsigned char * toffset = Global::pSong()->_ptrackline(ps,cursor_.x(),cursor_.y());

  // realtime note entering
  if (Global::pPlayer()->_playing&&Global::pConfig()->_followSong)
  {
      //toffset = offset+(Global::pPlayer()->_lineCounter*MULTIPLY);
  } else
  {
      toffset = Global::pSong()->_ptrackline(ps,cursor_.x(),cursor_.y());
  }

  // build entry
  PatternEntry *entry = (PatternEntry*) toffset;
  entry->_note = 120;
  notetrack[cursor_.x()]=120;

  window()->repaint(drawArea,drawArea->repaintTrackArea(cursor().y(),cursor().y(),cursor().x(),cursor().x()));
}


void PatternView::clearCursorPos( )
{
  // delete content at Cursor pos.

  // &&&&& hardcoded # of bytes per event

  const int ps = Global::pSong()->playOrder[editPosition()];
  unsigned char * offset = Global::pSong()->_ptrack(ps,cursor_.x());
  unsigned char * toffset = Global::pSong()->_ptrackline(ps,cursor_.x(),cursor_.y());

  if ( cursor_.z() == 0 ) {
      memset(offset+(cursor_.y()*MULTIPLY),255,3*sizeof(char));
      memset(offset+(cursor_.y()*MULTIPLY)+3,0,2*sizeof(char));
  }
  else if (cursor_.z() < 5 ) { 
    *(toffset+(cursor().z()+1)/2)= 255;
  } else
  {
    *(toffset+(cursor().z()+1)/2)= 0; 
  }

  window()->repaint(drawArea,drawArea->repaintTrackArea(cursor().y(),cursor().y(),cursor().x(),cursor().x()));
}


int PatternView::eventFromCol(int col) {
  int count = 0; int index = 0;
  for (std::vector<int>::iterator it = eventSize.begin(); it < eventSize.end(); it++) {
    switch (*it) {
        case 1  : count+=2; break;
        case 2  : count+=4; break;
    }
    if (count >= col) return index;
    index++;
  }
  return -1;  // out of range
}


int PatternView::colStartFromEvent( int event )
{
  int col = 1; 
  int count = 0;
  for (std::vector<int>::iterator it = eventSize.begin(); it < eventSize.end(); it++) {
    if (count >= event) return col;
    switch (*it) {
        case 1  : col+=2; break;
        case 2  : col+=4; break;
    }
    count++;
  }
  return -1;  // out of range
}


int PatternView::eventLength(int event) {
  if (event >= 0 && event < eventSize.size()) return eventSize.at(event); else return -1;
}

void PatternView::PatternDraw::onPopupPattern( NButtonEvent * ev )
{
  int patNum = Global::pSong()->playOrder[pView->editPosition()];
  int nlines = Global::pSong()->patternLines[patNum];

  patDlg->setLineNumber(nlines);
  if (patDlg->execute()) {

    if ( nlines != patDlg->lineNumber() ) {
      //AddUndo(patNum,0,0,MAX_TRACKS,nlines,editcur.track,editcur.line,editcur.col,editPosition);
      //AddUndoLength(patNum,nlines,editcur.track,editcur.line,editcur.col,editPosition);
      Global::pSong()->AllocNewPattern(patNum,"",patDlg->lineNumber(),patDlg->adaptSize()?true:false);
      if (pView->cursor().y() > patDlg->lineNumber()) pView->setCursor(NPoint3D(pView->cursor().x(),patDlg->lineNumber()-1,pView->cursor().z()));
      int count = (clientHeight()-pView->headerHeight()) / pView->rowHeight();
      pView->vBar->setRange((pView->lineNumber()-1-count)*pView->rowHeight());
      repaint();
    }
  }
}

void PatternView::PatternDraw::onPopupTranspose1( NButtonEvent * ev )
{
  blockTranspose(1);
}

void PatternView::PatternDraw::onPopupTranspose12( NButtonEvent * ev )
{
  blockTranspose(12);
}

void PatternView::PatternDraw::onPopupTranspose_1( NButtonEvent * ev )
{
  blockTranspose(-1);
}

void PatternView::PatternDraw::onPopupTranspose_12( NButtonEvent * ev )
{
  blockTranspose(-12);
}



void PatternView::PatternDraw::startSel(const NPoint3D & p)
{
  selStartPoint_ = p;
  selection_.setLeft(p.x());
  selection_.setTop(p.y());
  selection_.setRight(p.x());
  selection_.setBottom(p.y());
  oldSelection_ = selection_;
  doDrag_ = true;
  doSelect_ = false;
}

void PatternView::PatternDraw::endSel( )
{
  doDrag_ = false;
  doSelect_ = false;
}

void PatternView::PatternDraw::doSel(const NPoint3D & p )
{
  doSelect_=true;
    if (p.x() < selStartPoint_.x()) {
        selection_.setLeft(std::max(p.x(),0)); 
        int startTrack  = dx_ / pView->colWidth();
        if (selection_.left() < startTrack && startTrack > 0) {
            pView->hScrBar()->setPos( (startTrack-1)* pView->colWidth());
        }
    }
    else
    if (p.x() == selStartPoint_.x()) {
      selection_.setLeft (std::max(p.x(),0));
      selection_.setRight(std::min(p.x()+1,pView->trackNumber()));
    } else
    if (p.x() > selStartPoint_.x()) {
        selection_.setRight(std::min(p.x()+1,pView->trackNumber()));
        int startTrack  = dx_ / pView->colWidth();
        int trackCount  = clientWidth() / pView->colWidth();
        if (selection_.right() > startTrack + trackCount) {
            pView->hScrBar()->setPos( (startTrack+2) * pView->colWidth());
        } else
        if (selection_.right() < startTrack && startTrack > 0) {
            pView->hScrBar()->setPos( (startTrack-1)* pView->colWidth());
        }
    }
    if (p.y() < selStartPoint_.y()) {
        selection_.setTop(std::max(p.y(),0));
        int startLine  = dy_ / pView->rowHeight();
        if (selection_.top() < startLine && startLine >0) {
            pView->vScrBar()->setPos( (startLine-1) * pView->rowHeight());
          }
    } else
    if (p.y() == selStartPoint_.y()) {
      selection_.setTop (p.y());
      selection_.setBottom(p.y()+1);
    } else
    if (p.y() > selStartPoint_.y()) {
          selection_.setBottom(std::min(p.y()+1,pView->lineNumber()));
          int startLine  = dy_ / pView->rowHeight();
          int lineCount  = clientHeight() / pView->rowHeight();
          if (selection_.bottom() > startLine + lineCount) {
            pView->vScrBar()->setPos( (startLine+1) * pView->rowHeight());
          } else
          if (selection_.bottom() < startLine && startLine >0) {
            pView->vScrBar()->setPos( (startLine-1) * pView->rowHeight());
          }
    }

    if (oldSelection_ != selection_) {
      // these is totally unoptimized todo repaint only new area
      NSize clipBox = selection_.clipBox(oldSelection_);
      NRect r = repaintTrackArea(clipBox.top(),clipBox.bottom(),clipBox.left(),clipBox.right());
      window()->repaint(this,r);
      oldSelection_ = selection_;
    }
}

void PatternView::PatternDraw::onKeyRelease(const NKeyEvent & event) {
  if ( event.scancode() == XK_Shift_L || event.scancode() == XK_Shift_R ) endSel();

  if (pView->cursor().z()==0) {
    int outnote = Global::pConfig()->inputHandler.getEnumCodeByKey(Key(0,event.scancode()));
    //pView->StopNote(outnote);
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
  drawArea->blockTranspose(trp);
}

void PatternView::PatternDraw::deleteBlock( )
{
  PatternEntry blank;
  // UNDO CODE HERE CUT
  //AddUndo(ps,blockSel.start.track,blockSel.start.line,blockNTracks,blockNLines,editcur.track,editcur.line,editcur.col,editPosition);

  int ps=Global::pSong()->playOrder[pView->editPosition()];

  for (int t=selection_.left(); t < selection_.right();t++)
    for (int l=selection_.top(); l< selection_.bottom();l++)
            memcpy(Global::pSong()->_ptrackline(ps,t,l),&blank,EVENT_SIZE);

}

void PatternView::deleteBlock( )
{
  drawArea->deleteBlock();
}

void PatternView::setPatternStep( int step )
{
  patternStep_ = step;
}

int PatternView::patternStep() const {
  return patternStep_;
}

void PatternView::StopNote( int note, bool bTranspose, Machine * pMachine )
{
  if (!(note>=0 && note < 128)) return;

  // octave offset
  if(note<120) {
      if(bTranspose) note+=Global::pSong()->currentOctave*12;
      if (note > 119) note = 119;
  }

  if(pMachine==NULL) {
      int mgn = Global::pSong()->seqBus;

      if (mgn < MAX_MACHINES) {
          pMachine = Global::pSong()->_pMachine[mgn];
      }

  for(int i=0;i<Global::pSong()->SONGTRACKS;i++) {
      if(notetrack[i]==note) {
        notetrack[i]=120;
        // build entry
        PatternEntry entry;
        entry._note = 120+0;
        entry._inst = Global::pSong()->auxcolSelected;
        entry._mach = Global::pSong()->seqBus;;
        entry._cmd = 0;
        entry._parameter = 0;

        // play it

        if (pMachine) {
          pMachine->Tick(i,&entry);
        }
      }
    }

  }
}

/*const int ps = Global::pSong()->playOrder[editPosition()];
  unsigned char * offset = Global::pSong()->_ptrack(ps,cursor_.x());
  unsigned char * toffset = Global::pSong()->_ptrackline(ps,cursor_.x(),cursor_.y());

  if ( cursor_.z() == 0 ) {
      memset(offset+(cursor_.y()*MULTIPLY),255,3*sizeof(char));
      memset(offset+(cursor_.y()*MULTIPLY)+3,0,2*sizeof(char));
  }
  else if (cursor_.z() < 5 ) { 
    *(toffset+(cursor().z()+1)/2)= 255;
  } else
  {
    *(toffset+(cursor().z()+1)/2)= 0; 
  }

int ps=Global::pSong()->playOrder[pView->editPosition()];

  for (int t=selection_.left(); t < selection_.right();t++)
    for (int l=selection_.top(); l< selection_.bottom();l++)
            memcpy(Global::pSong()->_ptrackline(ps,t,l),&blank,EVENT_SIZE);

*/

void PatternView::PatternDraw::doubleLength( )
{
  unsigned char *toffset;
  PatternEntry blank;
  int st, et, sl, el,nl;

  st = selection_.left();
  et = selection_.right();
  sl = selection_.left();
  el = selection_.bottom();
  nl=((selection_.bottom()-selection_.top())/2);

  if (st==et && sl==el) {
    st=0;		
    et= pView->trackNumber()-1;
    sl=0;
    nl= (pView->lineNumber()-1)/2;
    el= pView->lineNumber()-1;
  }

  int ps = Global::pSong()->playOrder[pView->editPosition()];

  for (int t= st ; t <= et ;t++) {
    toffset= Global::pSong()->_ptrack(ps,t);
    memcpy(toffset+el*MULTIPLY,&blank,EVENT_SIZE);
    for (int l=nl;l>=0;l--) {
      memcpy(toffset+(sl*2)*MULTIPLY,toffset+(sl)*MULTIPLY,EVENT_SIZE);
      memcpy(toffset+(sl+(l*2))*MULTIPLY,&blank,EVENT_SIZE);
    }
  }
}


void PatternView::doubleLength( )
{
  drawArea->doubleLength();
}


// next todo

/*


void CChildView::
    {
      // UNDO CODE DOUBLE LENGTH
      unsigned char *toffset;
      PatternEntry blank;
      int st, et, sl, el,nl;

      int ps = _ps();
      if ( blockSelected )
      {
    ///////////////////////////////////////////////////////// Add ROW
        st=blockSel.start.track;		
        et=blockSel.end.track+1;
        sl=blockSel.start.line;			
        nl=((blockSel.end.line-sl)/2)+1;
        el=blockSel.end.line;
        AddUndo(ps,blockSel.start.track,blockSel.start.line,blockSel.end.track-blockSel.start.track+1,nl*2,editcur.track,editcur.line,editcur.col,editPosition);
      }
      else 
      {
        st=0;		
        et=_pSong->SONGTRACKS;		
        sl=0;
        nl= _pSong->patternLines[ps]/2;	
        el=_pSong->patternLines[ps]-1;
        AddUndo(ps,0,0,MAX_TRACKS,el+1,editcur.track,editcur.line,editcur.col,editPosition);
      }

      for (int t=st;t<et;t++)
      {
        toffset=_ptrack(ps,t);
        memcpy(toffset+el*MULTIPLY,&blank,EVENT_SIZE);
        for (int l=nl-1;l>0;l--)
        {
          memcpy(toffset+(sl+l*2)*MULTIPLY,toffset+(sl+l)*MULTIPLY,EVENT_SIZE);
          memcpy(toffset+(sl+(l*2)-1)*MULTIPLY,&blank,EVENT_SIZE);
        }
      }

      NewPatternDraw(st,et,sl,el);
      Repaint(DMData);
    }

    void CChildView::HalveLength()
    {
      // UNDO CODE HALF LENGTH
      unsigned char *toffset;
      int st, et, sl, el,nl;
      int ps = _ps();
      PatternEntry blank;

      if ( blockSelected )
      {
    ///////////////////////////////////////////////////////// Add ROW
        st=blockSel.start.track;	
        et=blockSel.end.track+1;
        sl=blockSel.start.line;		
        nl=blockSel.end.line-sl+1;
        el=nl/2;
        AddUndo(ps,blockSel.start.track,blockSel.start.line,blockSel.end.track-blockSel.start.track+1,nl,editcur.track,editcur.line,editcur.col,editPosition);
      }
      else 
      {
        st=0;	
        et=_pSong->SONGTRACKS;		
        sl=0;
        nl=_pSong->patternLines[ps];	
        el=_pSong->patternLines[ps]/2;
        AddUndo(ps,0,0,MAX_TRACKS,nl,editcur.track,editcur.line,editcur.col,editPosition) ;
      }
      
      for (int t=st;t<et;t++)
      {
        toffset=_ptrack(ps,t);
        int l;
        for (l=1;l<el;l++)
        {
          memcpy(toffset+(l+sl)*MULTIPLY,toffset+((l*2)+sl)*MULTIPLY,EVENT_SIZE);
        }
        while (l < nl)
        {
          memcpy(toffset+((l+sl)*MULTIPLY),&blank,EVENT_SIZE);
          l++;
        }
      }

      NewPatternDraw(st,et,sl,nl+sl);
      Repaint(DMData);
    }
*/
