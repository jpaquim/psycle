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
#include "nfontmetrics.h"
#include "configuration.h"
#include "global.h"
#include "song.h"
#include "inputhandler.h"
#include "napp.h"
#include "nalignlayout.h"
#include "nwindow.h"
#include "nmenuitem.h"
#include "nmenuseperator.h"
#include "player.h"
#include "machine.h"
#include "nframeborder.h"


/// The pattern Main Class , a container for the inner classes LineNumber, Header, and PatternDraw
PatternView::PatternView()
 : NPage()
{
  setLayout(new NAlignLayout());

  hBar = new NScrollBar();
    hBar->setOrientation(nHorizontal);
    hBar->setHeight(15);
    hBar->setAlign(nAlBottom);
    hBar->posChange.connect(this,&PatternView::onHScrollBar);
  add(hBar);

  vBar = new NScrollBar();
    vBar->setAlign(nAlRight);
    vBar->setWidth(15);
    vBar->posChange.connect(this,&PatternView::onVScrollBar);
  add(vBar);

  lNumber_ = 255;

  lineNumber_ = new LineNumber(this);
    setBorder(new NFrameBorder());
    lineNumber_->setAlign(nAlLeft);
  add(lineNumber_);

  header = new Header(this);
    header->setAlign(nAlTop);
  add(header);

  drawArea = new PatternDraw(this);
    drawArea->setAlign(nAlClient);
  add(drawArea);

  setFont(NFont("System",8,nMedium | nStraight ));

  addEvent(1); // add one byte event to trackerline
  addEvent(1); // add one byte event to trackerline
  addEvent(2); // add two byte event to trackerline

  editPosition_ = 0;
  prevEditPosition_ = 0;
  playPos_ = 0;
  editOctave_ = 0;
  outtrack = 0;
  for(int i=0;i<MAX_TRACKS;i++) notetrack[i]=120;
  setTransparent(false);
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
      window()->repaint(rect);
    } else {
      drawArea->setDx(newPos);
      drawArea->repaint();
    }
  }
}

void PatternView::onVScrollBar( NObject * sender, int pos )
{
  if (pos > 0) {
  int newPos = (pos / rowHeight()) * rowHeight();

  if (newPos != drawArea->dy()) {
    int diffY  = newPos - lineNumber_->dy();
     if (diffY < drawArea->clientHeight()) {
       NRect rect = lineNumber_->blitMove(0,diffY,NRect(lineNumber_->absoluteSpacingLeft(),lineNumber_->absoluteSpacingTop()+headerHeight(),lineNumber_->spacingWidth(),lineNumber_->spacingHeight()-headerHeight()));
       lineNumber_->setDy(newPos);
       if (diffY < 0) rect.setHeight(rect.height()+headerHeight());
       window()->repaint(rect);
       rect = drawArea->blitMove(0,diffY,drawArea->absoluteSpacingGeometry());
       drawArea->setDy(newPos);
       window()->repaint(rect);
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
  NPanel::resize(); // we use a layout manager!
  hBar->setRange(header->preferredWidth() - clientWidth());
  int count = (drawArea->clientHeight()-headerHeight()) / rowHeight();
  vBar->setRange((lineNumber()-count)*rowHeight());
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
  return header->height();
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
  int newX = std::min(std::max(cursor_.x()+dx,0),trackNumber());
  int newY = std::min(std::max(0,cursor_.y()+dy),lineNumber());
  int newZ = cursor_.z()+dz;

  if (newZ >= cellCount() ) {
     newX++;
     newZ=0;
  } else

  if (newZ < 0 ) {
     newX--;
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
  vBar->setRange((lineNumber()-count)*rowHeight());
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
  bitmap.loadFromFile("/home/natti/xpsycle/icons/pattern_header_skin.xpm");
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

    if (i!=0) g->drawLine(i*pView->colWidth(),0,i*pView->colWidth(),clientWidth()); // col seperator
  }
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
  PatternView::LineNumber::LineNumber( PatternView * pPatternView ) : dy_(0), NPanel()
  {
    pView = pPatternView;
    setTransparent(true);
    setWidth(40);
  }

  PatternView::LineNumber::~ LineNumber( )
  {
  }

  void PatternView::LineNumber::paint( NGraphics * g )
  {
    int startLine = dy_ / pView->rowHeight();
    int rDiff   = g->repaintArea().top() - absoluteTop() + pView->headerHeight();
    int offT = rDiff / pView->rowHeight();
    if (offT < 0) offT = 0;
    offT = 0;
    int offB = (rDiff+g->repaintArea().height()) / pView->rowHeight();
    if (offB < 0) offB = 0;
    int count = std::min(clientHeight() / pView->rowHeight(),offB);

    for (int i = offT; i < count; i++)
      g->drawLine(0,i*pView->rowHeight()+pView->headerHeight(),
                  clientWidth(),i*pView->rowHeight()+pView->headerHeight());

    g->setForeground(pView->separatorColor());
    g->drawLine(0,0,0,clientHeight());
    g->drawLine(clientWidth()-1,0,clientWidth()-1,clientHeight());

      for (int i = offT; i < count; i++)
        g->drawText(clientWidth()-g->textWidth(stringify(i+startLine))-3,i*pView->rowHeight()+pView->rowHeight()+pView->headerHeight()-1,stringify(i+startLine));

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
    editPopup_->add(new NMenuItem("Block mix paste"));
    editPopup_->add(new NMenuItem("Block delete"));
    editPopup_->add(new NMenuSeperator());
    editPopup_->add(new NMenuItem("Interpolate Effect"));
    editPopup_->add(new NMenuItem("Change Generator"));
    editPopup_->add(new NMenuItem("Change Instrument"));
    editPopup_->add(new NMenuSeperator());
    editPopup_->add(new NMenuItem("Transpose+1"));
    editPopup_->add(new NMenuItem("Transpose-1"));
    editPopup_->add(new NMenuItem("Transpose+12"));
    editPopup_->add(new NMenuItem("Transpose-12"));
    editPopup_->add(new NMenuSeperator());
    editPopup_->add(new NMenuItem("Block Swing Fill"));
    editPopup_->add(new NMenuItem("Block Track Fill"));
    editPopup_->add(new NMenuSeperator());
    NMenuItem* blockPatPropItem_ = new NMenuItem("Pattern properties");
      blockPatPropItem_->click.connect(this,&PatternView::PatternDraw::onPopupPattern);
    editPopup_->add(blockPatPropItem_);
    initKeyMap();
    patDlg = new PatDlg();
}

PatternView::PatternDraw::~ PatternDraw( )
{
}

void PatternView::PatternDraw::paint( NGraphics * g )
{
  int startLine  = dy_ / pView->rowHeight();
  int startTrack = dx_ / pView->colWidth();
  int lineCount  = clientHeight() / pView->rowHeight();
  int trackCount = clientWidth()  / pView->colWidth();

  // check for repaintArea

  int startLineOffset = std::max(((g->repaintArea().top() - absoluteTop()) / pView->rowHeight()),(long)0);
  int endLineOffset   = std::max((clientHeight()-(g->repaintArea().top() + g->repaintArea().height() - absoluteTop())) / pView->rowHeight(),(long)0);

  int startTrackOffset = std::max((g->repaintArea().left() - absoluteLeft()) / pView->colWidth(),(long)0);
  int endTrackOffset   = std::max((clientWidth()-(g->repaintArea().left() + g->repaintArea().width() - absoluteLeft())) / pView->colWidth() -1,(long)0);

  startLine  = startLine  + startLineOffset;
  startTrack = startTrack + startTrackOffset;
  int endLine  =  std::min(startLine  + (lineCount  - startLineOffset - endLineOffset)+1 , pView->lineNumber());
  int endTrack = std::min(startTrack + (trackCount - startTrackOffset - endTrackOffset) , pView->trackNumber()-1 );


  g->setForeground(Global::pConfig()->pvc_rowbeat);

  int trackWidth = ((endTrack+1) * pView->colWidth()) - dx();

  for (int y = startLine; y < endLine; y++) {
    if (!(y == pView->playPos())) {
      if ( !(y % Global::pSong()->LinesPerBeat())) {
        if (!(y%(Global::pSong()->LinesPerBeat()*Global::pConfig()->pv_timesig))) {
           g->setForeground(Global::pConfig()->pvc_row4beat);
           g->fillRect(0,y*pView->rowHeight() - dy_,trackWidth,pView->rowHeight());
           g->setForeground(Global::pConfig()->pvc_rowbeat);
        } else {
          g->fillRect(0,y*pView->rowHeight() - dy_,trackWidth,pView->rowHeight());
        }
      }
    } else {
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
     g->fillRect(i*pView->colWidth()-dx_,0,3,clientHeight());

  g->setForeground(pView->separatorColor());
  for (int i = startTrack; i <= endTrack; i++)  // col separators
      g->drawLine(i*pView->colWidth()-dx_,0,i*pView->colWidth()-dx_,clientHeight());

  g->setForeground(pView->foreground());

  for (int x = startTrack; x <= endTrack; x++) {
     int COL = pView->noteCellWidth();
     for (std::vector<int>::iterator it = pView->eventSize.begin(); it < pView->eventSize.end(); it++) {
      switch (*it) {
      case 1:
           g->drawLine(x*pView->colWidth()+COL-dx_,0,x*pView->colWidth()+COL-dx_,clientHeight());
           COL+=2 * pView->cellWidth();
      break;
      case 2:
           g->drawLine(x*pView->colWidth()+COL-dx_,0,x*pView->colWidth()+COL-dx_,clientHeight());
           COL+=4 * pView->cellWidth();
      break;
     }
   }
  }

  drawPattern(g,std::min(startLine,pView->lineNumber()),std::min(endLine,pView->lineNumber()),startTrack,endTrack);
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
  for (int y = startLine; y < endLine; y++) {
     unsigned char *patOffset = Global::pSong()->_ppattern(Global::pSong()->playOrder[pView->editPosition_]) + (y*MULTIPLY) + (startTrack)*5;
    for (int x = startTrack; x < endTrack; x++) {
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
  if (button == 3) {
     editPopup_->setPosition(x + absoluteLeft(), y + 10 + absoluteTop(),100,100);
     editPopup_->setVisible(true);
  } else {
    clearOldSelection();
    NPoint3D p = intersectCell(x,y);
     selStartPoint_ = p;
     selection_.setLeft(p.x());
     selection_.setTop(p.y());
     selection_.setRight(p.x());
     selection_.setBottom(p.y());
     oldSelection_ = selection_;
     doDrag_ = true;
     doSelect_ = false;
  }
}

void PatternView::PatternDraw::onMouseOver( int x, int y )
{
  if (doDrag_) {
    doSelect_=true;
    NPoint3D p = intersectCell(x,y);
    if (p.x() < selStartPoint_.x()) selection_.setLeft(p.x()); else
    if (p.x() == selStartPoint_.x()) {
      selection_.setLeft (p.x());
      selection_.setRight(p.x()+1);
    } else
    if (p.x() > selStartPoint_.x()) {
        selection_.setRight(p.x()+1);
        int startTrack  = dx_ / pView->colWidth();
        int trackCount  = clientWidth() / pView->colWidth();
        if (selection_.right() > startTrack + trackCount) {
           pView->hScrBar()->setPos( (startTrack+2) * pView->colWidth());
        }
    }
    if (p.y() < selStartPoint_.y()) selection_.setTop(p.y()); else
    if (p.y() == selStartPoint_.y()) {
      selection_.setTop (p.y());
      selection_.setBottom(p.y()+1);
    } else
    if (p.y() > selStartPoint_.y()) {
         selection_.setBottom(p.y()+1);
         int startLine  = dy_ / pView->rowHeight();
         int lineCount  = clientHeight() / pView->rowHeight();
         if (selection_.bottom() > startLine + lineCount) {
           pView->vScrBar()->setPos( (startLine+2) * pView->rowHeight());
        }
    }

    if (oldSelection_ != selection_) {
      // these is totally unoptimized todo repaint only new area
      NSize clipBox = selection_.clipBox(oldSelection_);
      NRect r = repaintTrackArea(clipBox.top(),clipBox.bottom(),clipBox.left(),clipBox.right());
      window()->repaint(r);
      oldSelection_ = selection_;
    }
  }
}

char inline hex_value(char c) { if(c >= 'A') return 10 + c - 'A'; else return c - '0'; }

void PatternView::PatternDraw::onMousePressed( int x, int y, int button )
{
  if (!doSelect_) pView->setCursor(intersectCell(x,y));
  repaint();
  doDrag_ = false;
  doSelect_ = false;
}

void PatternView::PatternDraw::onKeyPress( const NKeyEvent & event )
{
  switch (event.scancode()) {
    case XK_BackSpace :
       pView->clearCursorPos();
    break;
    case XK_Left: {
        int oldTrack = pView->cursor().x();
        pView->moveCursor(0,0,-1);
        int newTrack = pView->cursor().x();
        int startTrack  = dx_ / pView->colWidth();
        if (newTrack < startTrack) {
           pView->hScrBar()->setPos( (newTrack) * pView->colWidth());
        }
        window()->repaint(repaintTrackArea(pView->cursor().y(),pView->cursor().y(),newTrack,oldTrack));
    }
    break;
    case XK_Right: {
        int oldTrack = pView->cursor().x();
        pView->moveCursor(0,0,1);
        int newTrack    = pView->cursor().x();
        int trackCount  = clientWidth() / pView->colWidth();
        int startTrack  = dx_ / pView->colWidth();
        if (newTrack > startTrack + trackCount -1) {
           pView->hScrBar()->setPos( (startTrack+2) * pView->colWidth());
        }
        window()->repaint(repaintTrackArea(pView->cursor().y(),pView->cursor().y(),oldTrack,newTrack));
     }
     break;
     case XK_Down : {
        int startLine  = dy_ / pView->rowHeight();
        int lineCount  = clientHeight() / pView->rowHeight();
        int oldLine = pView->cursor().y();
        pView->moveCursor(0,1,0);
        int newLine = pView->cursor().y();
        if (newLine > startLine + lineCount-1) {
           pView->vScrBar()->setPos( (startLine+2) * pView->rowHeight());
        }
        window()->repaint(repaintTrackArea(oldLine,newLine,pView->cursor().x(),pView->cursor().x()));
     }
     break;
     case XK_Up: {
        int oldLine = pView->cursor().y();
        pView->moveCursor(0,-1,0);
        int startLine  = dy_ / pView->rowHeight();
        int newLine = pView->cursor().y();
        if (newLine <= startLine) {
           pView->vScrBar()->setPos( (newLine) * pView->rowHeight());
        }
        window()->repaint(repaintTrackArea(newLine,oldLine,pView->cursor().x(),pView->cursor().x()));
     }
     break;
     default:
       int note = charToNote(event.scancode());
       if (note == cdefKeyStop && pView->cursor().z()==0) pView->noteOffAny(); else {
       {
         unsigned char *patOffset = Global::pSong()->_ppattern(Global::pSong()->playOrder[pView->editPosition_]) + (pView->cursor().y() *MULTIPLY) + (pView->cursor().x())*5;

         if (pView->cursor().z()==0) {
           note += pView->editOctave()*12;
           *patOffset = note;
           repaint();
           Machine* tmac = Global::pSong()->_pMachine[0];
           pView->PlayNote(24,127,false,tmac);
         } else  {
           int off = (pView->cursor().z()+1) / 2;
           patOffset +=off;
           char newByte;
           if (pView->cursor().z() % 2 == 1) newByte = (*patOffset & 0x0F) | (0xF0 & (hex_value(event.scancode()) << 4));
                                        else newByte = (*patOffset & 0xF0) | (0x0F & (hex_value(event.scancode())));
           *patOffset = newByte;
         }
         repaint();
       }
     }
     break;
  }

}

void PatternView::PatternDraw::initKeyMap( )
{
  // octave 0

  keyMap_['z'] = 0;
  keyMap_['s'] = 1;
  keyMap_['x'] = 2;
  keyMap_['d'] = 3;
  keyMap_['c'] = 4;
  keyMap_['v'] = 5;
  keyMap_['g'] = 6;
  keyMap_['b'] = 7;
  keyMap_['h'] = 8;
  keyMap_['n'] = 9;
  keyMap_['j'] = 10;
  keyMap_['m'] = 11;

  // octave 1

  keyMap_['q'] = 0  + 12;
  keyMap_['2'] = 1  + 12;
  keyMap_['w'] = 2  + 12;
  keyMap_['3'] = 3  + 12;
  keyMap_['e'] = 4  + 12;
  keyMap_['r'] = 5  + 12;
  keyMap_['5'] = 6  + 12;
  keyMap_['t'] = 7  + 12;
  keyMap_['6'] = 8  + 12;
  keyMap_['y'] = 9  + 12;
  keyMap_['7'] = 10 + 12;
  keyMap_['u'] = 11 + 12;

  // octave 2

  keyMap_['i'] = 0 + 24;
  keyMap_['9'] = 1 + 24;
  keyMap_['o'] = 2 + 24;
  keyMap_['0'] = 3 + 24;
  keyMap_['p'] = 4 + 24;

  // special

  keyMap_['1'] = cdefKeyStop;
}

int PatternView::PatternDraw::charToNote( char c )
{
  std::map<char,int>::iterator itr = keyMap_.begin();
  if ( (itr = keyMap_.find(c)) == keyMap_.end() )
       {
          // no mapped Key found
       } else {
          return itr->second;
       }
  return 255;
}


NRect PatternView::PatternDraw::repaintTrackArea( int startLine, int endLine, int startTrack, int endTrack )
{
  int top    = startLine    * pView->rowHeight()  + absoluteTop()  - dy_;
  int bottom = (endLine+1)  * pView->rowHeight()  + absoluteTop()  - dy_;
  int left   = startTrack   * pView->colWidth()   + absoluteLeft() - dx_;
  int right  = (endTrack+1) * pView->colWidth()   + absoluteLeft() - dx_;

  return NRect(left,top,right - left,bottom - top);
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
  window()->repaint(r);
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

void PatternView::updatePlayBar( )
{
  if (Global::pPlayer()->_lineChanged)
  {
     int trackCount  = clientWidth() / colWidth();
     int startTrack  = drawArea->dx() / colWidth();

     int oldPlayPos = playPos_;
     playPos_ = Global::pPlayer()->_lineCounter;
     window()->repaint(drawArea->repaintTrackArea(oldPlayPos,playPos_,startTrack,trackCount+startTrack));
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

  window()->repaint(drawArea->repaintTrackArea(cursor().y(),cursor().y(),cursor().x(),cursor().x()));
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

  window()->repaint(drawArea->repaintTrackArea(cursor().y(),cursor().y(),cursor().x(),cursor().x()));
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


int PatternView::eventLength(int event) {
  if (event >= 0 && event < eventSize.size()) return eventSize.at(event); else return -1;
}

void PatternView::PatternDraw::onPopupPattern( NButtonEvent * ev )
{
  int patNum = Global::pSong()->playOrder[pView->editPosition()];
  int nlines = Global::pSong()->patternLines[patNum];

  patDlg->setLineNumber(nlines);
  patDlg->execute();

  if ( nlines != patDlg->lineNumber() ) {
   //AddUndo(patNum,0,0,MAX_TRACKS,nlines,editcur.track,editcur.line,editcur.col,editPosition);
   //AddUndoLength(patNum,nlines,editcur.track,editcur.line,editcur.col,editPosition);
   Global::pSong()->AllocNewPattern(patNum,"",patDlg->lineNumber(),patDlg->adaptSize()?true:false);
   repaint();
  }
}



