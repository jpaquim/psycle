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
#include "sequencergui.h"
#include "sequencerbeatchangelineal.h"
#include "singlepattern.h"
#include "zoombar.h"
#include "global.h"
#include "configuration.h"
#include "defaultbitmaps.h"
#include <ngrs/nscrollbar.h>
#include <ngrs/nalignlayout.h>
#include <ngrs/nlabel.h>
#include <ngrs/nframeborder.h>
#include <ngrs/ntoolbar.h>
#include <ngrs/ntoolbarseparator.h>
#include <ngrs/nlistbox.h>
#include <ngrs/nitem.h>
#include <ngrs/nwindow.h>
#include <ngrs/nbevelborder.h>
#include <ngrs/nproperty.h>


namespace psycle {
	namespace host {


// this is the bpm lineal

SequencerGUI::SequencerBeatLineal::SequencerBeatLineal(SequencerGUI* seqGui )
{
  sView = seqGui;
  setBackground(NColor(255,255,255));
  setTransparent(false);
}

SequencerGUI::SequencerBeatLineal::~ SequencerBeatLineal( )
{
}

void SequencerGUI::SequencerBeatLineal::paint( NGraphics * g )
{
  drawLineal(g, 0);
}

void SequencerGUI::SequencerBeatLineal::drawLineal( NGraphics* g, int dx )
{
  NRect area = g->repaintArea().rectClipBox();

  int cw = clientWidth();
  int ch = clientHeight();

  g->setForeground(NColor(220,220,220));

  g->drawLine(scrollDx(), ch - 10 , cw + scrollDx(), ch - 10);

  int start = (area.left() - absoluteLeft() + scrollDx()) / sView->beatPxLength();
  int end   = (area.left() + area.width() - absoluteLeft() + scrollDx() ) / sView->beatPxLength();

  for (int i = start ; i < end ; i++) {
     if (! (i % 16)) {
        g->setForeground(NColor(180,180,180));
        g->drawLine(i* sView->beatPxLength(),ch-10,d2i(i*sView->beatPxLength()), ch);
        std::string beatLabel = stringify(i/4);
        int textWidth = g->textWidth(beatLabel);
        g->drawText(i* sView->beatPxLength() - textWidth / 2, g->textAscent(), beatLabel);
     }
     else {
        if (sView->beatPxLength() > 3) {
          g->setForeground(NColor(220,220,220));
          g->drawLine(i* sView->beatPxLength(),ch-10,d2i(i*sView->beatPxLength()), ch-5);
        }
    }
  }
}

int SequencerGUI::SequencerBeatLineal::preferredHeight( ) const
{
  NFontMetrics metrics(font());

  return metrics.textHeight() + 10;
}



// this is the sequencer Area

SequencerGUI::Area::Area( SequencerGUI* seqGui )
{
  setBackground(Global::pConfig()->pvc_row);
  setTransparent(false);


  sView = seqGui;
}

SequencerGUI::Area::~ Area( )
{
}

void SequencerGUI::Area::paint( NGraphics * g )
{
  drawTimeGrid(g);
}


void SequencerGUI::Area::drawTimeGrid( NGraphics * g )
{
   NRect area = g->repaintArea().rectClipBox();

  int start = (area.left() - absoluteLeft() + scrollDx()) / sView->beatPxLength();
  int end   = (area.left() + area.width() - absoluteLeft() + scrollDx() ) / sView->beatPxLength();

  for (int i = start ; i <= end ; i++) {
     if ( sView->beatPxLength() > 3 || (sView->beatPxLength() <= 3 && (!( i %16)))  ) {
       g->setForeground(NColor(220,220,220));
       g->drawLine(i* sView->beatPxLength(),-scrollDy(),d2i(i*sView->beatPxLength()),clientHeight()+scrollDy());
     }
  }
}

int SequencerGUI::Area::preferredHeight( ) const
{
  std::vector<NVisualComponent*>::const_iterator itr = visualComponents().begin();

  int yp = 0;

  for (;itr < visualComponents().end(); itr++) {
    NVisualComponent* visualChild = *itr;
    yp += visualChild->preferredHeight();
  }

  return yp;
}

int SequencerGUI::Area::preferredWidth( ) const
{
  std::vector<NVisualComponent*>::const_iterator itr = visualComponents().begin();

  int xp = 100;

  for (;itr < visualComponents().end(); itr++) {
    NVisualComponent* visualChild = *itr;
    xp = std::max(visualChild->preferredWidth(), xp);
  }

  return xp;
}

void SequencerGUI::Area::resize( )
{
  std::vector<NVisualComponent*>::const_iterator itr = visualComponents().begin();

  int xp = clientWidth();

  for (;itr < visualComponents().end(); itr++) {
    NVisualComponent* visualChild = *itr;
    xp = std::max(visualChild->preferredWidth(), xp);
  }



  itr = visualComponents().begin();

  int yp = 0;

  for (;itr < visualComponents().end(); itr++) {
    NVisualComponent* visualChild = *itr;
    visualChild->setHeight( visualChild->preferredHeight() );
    visualChild->setWidth(xp);
    visualChild->setTop(yp);
    yp+= visualChild->preferredHeight();
  }

}

// end of Area class


// this is the gui class of one pattern entry
SequencerItem::SequencerItem( SequencerGUI* seqGui )
{
  setMoveable(NMoveable(nMvHorizontal | nMvNoneRepaint | nMvLeftLimit));

  setTransparent(false);
  sequenceEntry_ = 0;
  sView = seqGui;
  selected_ = false;
  if (properties()) {
     properties()->bind("startOffset", *this, &SequencerItem::start, &SequencerItem::setStart);
     properties()->publish("startOffset");
     properties()->bind("endOffset", *this, &SequencerItem::endOffset, &SequencerItem::setEndOffset);
     properties()->publish("endOffset");
  }

}

SequencerItem::~ SequencerItem( )
{
}

void SequencerItem::setSequenceEntry( SequenceEntry * sequenceEntry )
{
  sequenceEntry_ = sequenceEntry;
}

void SequencerItem::paint( NGraphics * g )
{
  int cw = clientWidth();
  int ch = clientHeight();

  int tw = g->textWidth(sequenceEntry_->pattern()->name());

  int xp = (cw - tw) / 2;
  int yp = (ch + g->textHeight() /2 ) / 2;

  g->setForeground( NColor( sequenceEntry_->pattern()->category()->color() ));
  g->fillRect(0,0, clientWidth(), clientHeight() );
  g->drawText( xp, yp, sequenceEntry_->pattern()->name());

  if (selected_) {
    g->setForeground( NColor( 0,0,255) );
    g->drawRect(0,0, clientWidth()-1, clientHeight()-1 );
  }  else {
    g->setForeground( NColor( 180,180,180) );
    g->drawRect(0,0, clientWidth()-1, clientHeight()-1 );
  }
}

SequenceEntry * SequencerItem::sequenceEntry( )
{
  return sequenceEntry_;
}


void SequencerItem::resize( )
{

}


NRegion SequencerItem::entriesInRegion( )
{
  const std::vector<SequencerItem*> & selItems = sView->selectedItems();

  NRegion region;

  for (std::vector<SequencerItem*>::const_iterator it = selItems.begin(); it < selItems.end(); it++) {
    SequencerItem* item = *it;
    NRegion itemregion = item->geometry()->region();
    int parentAbsLeft = ((NVisualComponent*) item->parent())->absoluteLeft() - ((NVisualComponent*) item->parent())->scrollDx();
    int parentAbsTop  = ((NVisualComponent*) item->parent())->absoluteTop() - ((NVisualComponent*) item->parent())->scrollDy();;
    itemregion.move(parentAbsLeft, parentAbsTop);
    region = region | itemregion;
  }

  return region;
}

void SequencerItem::onMoveStart( const NMoveEvent & moveEvent )
{
  oldDrag = entriesInRegion();
  oldLeft = left();
}

void SequencerItem::onMove( const NMoveEvent & moveEvent )
{
  const std::vector<SequencerItem*> & selItems = sView->selectedItems();
  int moveDx = left() - oldLeft;
  oldLeft = left();


  for (std::vector<SequencerItem*>::const_iterator it = selItems.begin(); it < selItems.end(); it++) {
    SequencerItem* item = *it;
    int newLeft = 0;
    if (item != this) newLeft = item->left() + moveDx; else newLeft = item->left();
    item->sequenceEntry()->track()->MoveEntry(item->sequenceEntry(), newLeft / (double) sView->beatPxLength() );
  }
  sView->resize();

  NRegion newDrag = entriesInRegion();
  NRegion repaintArea = newDrag | oldDrag;

  window()->repaint(sView->scrollArea(),repaintArea);

  oldDrag = newDrag;
}

void SequencerItem::onMoveEnd( const NMoveEvent & moveEvent )
{
  sView->repaint();
}

void SequencerItem::onMousePress( int x, int y, int button )
{
  selected_ = true;
  click.emit(this);
  sView->entryClick.emit(this);
}

void SequencerItem::setSelected( bool on )
{
  selected_ = on;
}

bool SequencerItem::selected( )
{
  return selected_;
}

void SequencerItem::setStart( float start )
{
  sequenceEntry_->setStartPos(start);
  sView->resize();
  sView->repaint();
}

float SequencerItem::start( ) const
{
  return sequenceEntry_->startPos();
}

void SequencerItem::setEndOffset( float pos )
{
  sequenceEntry_->setEndPos( pos );
  sView->resize();
  sView->repaint();
}

float SequencerItem::endOffset( ) const
{
  return sequenceEntry_->endPos();
}

// end of SequencerItem class



// this is the gui class that represents one SequenceLine

SequencerGUI::SequencerLine::SequencerLine( SequencerGUI* seqGui )
{
  sView = seqGui;
  setBorder( NBevelBorder( nNone, nLowered) );
}

SequencerGUI::SequencerLine::~ SequencerLine( )
{
}

void SequencerGUI::SequencerLine::paint( NGraphics * g )
{
  if (sView->selectedLine_ && sView->selectedLine_ == this) {
    g->setForeground(NColor(0,0,255));
    g->drawRect(0,0, clientWidth()-1, clientHeight()-1);
  }
}

void SequencerGUI::SequencerLine::addItem( SinglePattern* pattern )
{
  double endTick = sequenceLine()->tickLength();

  SequencerItem* item = new SequencerItem(sView);
    item->click.connect(this,&SequencerGUI::SequencerLine::onSequencerItemClick);
    item->setPosition(d2i(sView->beatPxLength() * endTick),5,pattern->beats() * sView->beatPxLength() ,20);
    item->setSequenceEntry(sequenceLine()->createEntry(pattern, endTick));
    items.push_back(item);
  add(item);

}

void SequencerGUI::SequencerLine::removeItems( SinglePattern * pattern )
{
  std::list<SequencerItem*>::iterator it = items.begin();
  while ( it != items.end()) {
    SequencerItem* item = *it;
    if (item->sequenceEntry()->pattern() == pattern) {
      items.erase(it++);
      removeChild(item);
    } else it++;
	}
}

void SequencerGUI::SequencerLine::removeChild( NVisualComponent * item )
{
  std::list<SequencerItem*>::iterator it = find( items.begin(), items.end(), item );
  if (it != items.end() ) items.erase(it);
  NPanel::removeChild(item);
}

void SequencerGUI::SequencerLine::onMousePress( int x, int y, int button )
{
  click.emit(this);
}

void SequencerGUI::SequencerLine::setSequenceLine( SequenceLine * line )
{
  seqLine_ = line;
}

SequenceLine * SequencerGUI::SequencerLine::sequenceLine( )
{
  return seqLine_;
}


void SequencerGUI::SequencerLine::resize( )
{
  std::list<SequencerItem*>::iterator it = items.begin();
  for ( ; it != items.end(); it++) {
    SequencerItem* item = *it;
    double tickPosition = item->sequenceEntry()->tickPosition();
    SinglePattern* pattern = item->sequenceEntry()->pattern();

    item->setPosition(d2i(sView->beatPxLength() * tickPosition),5, (item->endOffset()-item->start()) * sView->beatPxLength(),20);
  }
}

std::vector<SequencerItem*> SequencerGUI::SequencerLine::itemsByPattern( SinglePattern * pattern )
{
  std::vector<SequencerItem*> list;
  std::list<SequencerItem*>::iterator it = items.begin();
  for ( ; it != items.end(); it++) {
    SequencerItem* item = *it;
    if (item->sequenceEntry()->pattern() == pattern) {
      list.push_back(item);
    }
  }
  return list;
}

int SequencerGUI::SequencerLine::preferredWidth( ) const
{
  if (items.size() > 0) {
     SequencerItem* last = items.back();
     return std::max(100, last->left() + last->width());
  } else
  return 100;
}

int SequencerGUI::SequencerLine::preferredHeight( ) const
{
  return 30;
}

void SequencerGUI::SequencerLine::onSequencerItemClick( SequencerItem * item )
{
  itemClick.emit(item);
}

// end of SequencerLine class


// main class

SequencerGUI::SequencerGUI()
 : NPanel()
{
  setLayout( NAlignLayout() );

  counter = 0;
  beatPxLength_ = 5; // default value for one beat

  scrollArea_ = new Area( this );

  DefaultBitmaps & icons = Global::pConfig()->icons();

  NImage* img;

  toolBar_ = new NToolBar();
    img = new NImage();
    img->setSharedBitmap(&icons.addTrack());
    img->setPreferredSize(25,25);
    NButton* btn;
    btn = new NButton(img);
    btn->setHint("Insert Track ");
    toolBar_->add( btn )->clicked.connect(this,&SequencerGUI::onInsertTrack);

    img = new NImage();
    img->setSharedBitmap(&icons.deleteTrack());
    img->setPreferredSize(25,25);
    btn = new NButton(img);
    btn->setHint("Delete Track ");
    toolBar_->add( btn)->clicked.connect(this,&SequencerGUI::onDeleteTrack);

    img = new NImage();
    img->setSharedBitmap(&icons.moveDownTrack());
    img->setPreferredSize(25,25);
    btn = new NButton(img);
    btn->setHint("Move Down Track ");
    toolBar_->add( btn)->clicked.connect(this,&SequencerGUI::onMoveDownTrack);

    img = new NImage();
    img->setSharedBitmap(&icons.moveUpTrack());
    img->setPreferredSize(25,25);
    btn = new NButton(img);
    btn->setHint("Move Up Track ");
    toolBar_->add( btn)->clicked.connect(this,&SequencerGUI::onMoveUpTrack);

    toolBar_->add(new NToolBarSeparator());

    toolBar_->add( new NButton("Delete Entry"))->clicked.connect(this,&SequencerGUI::onDeleteEntry);
  add(toolBar_, nAlTop);

  beatLineal_ = new SequencerBeatLineal(this);
  add(beatLineal_, nAlTop);

  // create scrollBars
  NPanel* hBarPanel = new NPanel();
    hBarPanel->setLayout( NAlignLayout() );
    zoomHBar = new ZoomBar();
    zoomHBar->setRange(2,50);
    zoomHBar->setPos(5);
    zoomHBar->posChanged.connect(this, &SequencerGUI::onZoomHBarPosChanged);
    hBarPanel->add(zoomHBar, nAlRight);
    hBar = new NScrollBar();
      hBar->setOrientation(nHorizontal);
      hBar->setPreferredSize(100,15);
      hBar->posChange.connect(this,&SequencerGUI::onHScrollBar);
    hBarPanel->add(hBar, nAlClient);
  add(hBarPanel, nAlBottom);

  beatChangeLineal_ = new SequencerBeatChangeLineal(this);
  add( beatChangeLineal_, nAlBottom );

  vBar = new NScrollBar();
    vBar->setWidth(15);
    vBar->setOrientation(nVertical);
    vBar->posChange.connect(this,&SequencerGUI::onVScrollBar);
  add(vBar, nAlRight);

  // the main Sequencer gui
  add(scrollArea_, nAlClient);

  lastLine = 0;
  selectedLine_ = 0;

  patternSequence_ = 0;
}


SequencerGUI::~SequencerGUI()
{
}

int SequencerGUI::beatPxLength( ) const
{
  return beatPxLength_;
}

void SequencerGUI::setPatternSequence( PatternSequence * sequence )
{
  patternSequence_ = sequence;
}

PatternSequence * SequencerGUI::patternSequence( )
{
  return patternSequence_;
}

void SequencerGUI::addSequencerLine( )
{
  SequencerLine* line = new SequencerLine( this );
  line->itemClick.connect(this, &SequencerGUI::onSequencerItemClick);
  lines.push_back(line);
  line->setSequenceLine( patternSequence_->createNewLine() );
  line->click.connect(this, &SequencerGUI::onSequencerLineClick);

  scrollArea_->add(line);
  scrollArea_->resize();
  lastLine = line;
  selectedLine_ = line;
}

// track operations

void SequencerGUI::onInsertTrack( NButtonEvent * ev )
{
  if (lines.size() == 0) {
   addSequencerLine();
   resize();
   repaint();
  } else
  if (selectedLine_) {
    SequencerLine* line = new SequencerLine( this );
    line->itemClick.connect(this, &SequencerGUI::onSequencerItemClick);
    lines.push_back(line);
    line->setSequenceLine( patternSequence_->createNewLine() );
    line->click.connect(this, &SequencerGUI::onSequencerLineClick);

    int index = selectedLine_->zOrder();
    scrollArea_->insert(line, index);
    scrollArea_->resize();
    lastLine = line;
    selectedLine_ = line;
    resize();
    scrollArea_->repaint();
  }
}

void SequencerGUI::onDeleteTrack( NButtonEvent * ev )
{
  if (selectedLine_) {

    SequenceLine* line = selectedLine_->sequenceLine();

    std::vector<SequencerLine*>::iterator it = find(lines.begin(), lines.end(), selectedLine_);
    if (it != lines.end() ) lines.erase(it);

    scrollArea_->removeChild(selectedLine_);
    scrollArea_->resize();

    patternSequence_->removeLine(line);
    scrollArea_->repaint();
    selectedLine_ = 0;
  }
}

void SequencerGUI::onMoveDownTrack( NButtonEvent * ev )
{
  if (selectedLine_) {
     int i = selectedLine_->zOrder();
     if ( i < componentZOrderSize() - 1 ) {
       scrollArea_->erase(selectedLine_);
       scrollArea_->insert(selectedLine_,i+1);
       patternSequence_->moveDownLine(selectedLine_->sequenceLine());
       scrollArea_->resize();
       scrollArea_->repaint();
     }
  }
}

void SequencerGUI::onMoveUpTrack( NButtonEvent * ev )
{
  if (selectedLine_) {
     int i = selectedLine_->zOrder();
     if ( i > 0 ) {
       scrollArea_->erase(selectedLine_);
       scrollArea_->insert(selectedLine_,i-1);
       patternSequence_->moveUpLine(selectedLine_->sequenceLine());
       scrollArea_->resize();
       scrollArea_->repaint();
     }
  }
}

void SequencerGUI::onNewPattern( NButtonEvent * ev )
{
  patternBox_->add(new NItem("Pattern" + stringify(counter) ));
  patternBox_->repaint();
  counter++;
}


void SequencerGUI::onSequencerLineClick( SequencerLine * line )
{
  selectedLine_ = line;
  deselectAll();
  repaint();
}

void SequencerGUI::onSequencerItemClick( SequencerItem * item )
{
  std::vector<SequencerItem*>::iterator it;
  it = find( selectedItems_.begin(), selectedItems_.end(), item);

  if (it == selectedItems_.end()) {
    if (NApp::system().keyState() & ControlMask) {
      selectedItems_.push_back(item);
      item->setSelected(true);
      item->repaint();
    } else {
      deselectAll();
      selectedItems_.push_back(item);
      item->setSelected(true);
      item->repaint();
    }
  }
}

// entry operations

void SequencerGUI::addPattern( SinglePattern * pattern )
{
  if ( selectedLine_ ) {
    selectedLine_->addItem( pattern );
    resize();
    repaint();
 }
}

void SequencerGUI::onDeleteEntry( NButtonEvent * ev )
{
  std::vector<SequencerItem*>::iterator it = selectedItems_.begin();

  // deselect all
  for ( ; it < selectedItems_.end(); it++) {
    SequencerItem* selectedItem = *it;
    SequenceEntry* entry = selectedItem->sequenceEntry();
    NVisualComponent* parentContainer = (NVisualComponent*) (selectedItem->parent());
    parentContainer->removeChild(selectedItem);
    entry->track()->removeEntry(entry);
  }
  selectedItems_.clear();

  resize();
  repaint();
}


void SequencerGUI::onZoomHBarPosChanged( ZoomBar * zoomBar, double newPos )
{
  if (newPos < 2) newPos = 2;
  beatPxLength_ = (int) newPos;
  resize();
  repaint();
}

void SequencerGUI::removePattern( SinglePattern * pattern )
{
  std::vector<SequencerLine*>::iterator it = lines.begin();
  for (; it < lines.end(); it++) {
    SequencerLine* line = *it;
    line->removeItems(pattern);
  }
}

std::vector<SequencerItem*> SequencerGUI::guiItemsByPattern( SinglePattern * pattern )
{
  std::vector<SequencerItem*> list;

  std::vector<SequencerLine*>::iterator it = lines.begin();
  for (; it < lines.end(); it++) {
    SequencerLine* line = *it;
    std::vector<SequencerItem*> temp = line->itemsByPattern(pattern);
    list.insert( list.end() , temp.begin(), temp.end());
  }
  return list;
}

void SequencerGUI::onVScrollBar( NObject * sender, int pos )
{
  int newPos = pos;
  if (newPos != scrollArea_->scrollDy() && newPos >= 0) {
      int diffY  = newPos - scrollArea_->scrollDy();
      if (diffY < scrollArea_->clientHeight()) {
        NRect rect = scrollArea_->blitMove(0,diffY, scrollArea_->absoluteSpacingGeometry());
        scrollArea_->setScrollDy(newPos);
        window()->repaint(scrollArea_,rect);
      } else {
        scrollArea_->setScrollDy(newPos);
        scrollArea_->repaint();
    }
  }
}

void SequencerGUI::onHScrollBar( NObject * sender, int pos )
{
  int newPos = pos;
  if (newPos != scrollArea_->scrollDx() && newPos >= 0) {

    int diffX  = newPos - scrollArea_->scrollDx();
    if (diffX < scrollArea_->clientWidth()) {
      // scrolls the area
      NRect rect = scrollArea_->blitMove(diffX,0, scrollArea_->absoluteSpacingGeometry());
      scrollArea_->setScrollDx(newPos);
      window()->repaint(scrollArea_,rect);

      // scrolls the beat lineal
      rect = beatLineal_->blitMove(diffX,0, beatLineal_->absoluteSpacingGeometry());
      beatLineal_->setScrollDx(newPos);
      window()->repaint(beatLineal_,rect);
      beatLineal_->repaint();

      // scrolls the beatchangelineal
      rect = beatChangeLineal_->blitMove(diffX,0, beatChangeLineal_->absoluteSpacingGeometry());
      beatChangeLineal_->setScrollDx(newPos);
      window()->repaint(beatChangeLineal_,rect);
      beatChangeLineal_->repaint();
    } else {
      scrollArea_->setScrollDx(newPos);
      scrollArea_->repaint(scrollArea_);
      beatLineal_->setScrollDx(newPos);
      beatLineal_->repaint();
      beatChangeLineal_->setScrollDx(newPos);
      beatChangeLineal_->repaint();
    }
  }
}

void SequencerGUI::resize( )
{
  // calls the AlignLayout
  NPanel::resize();
  // set the ScrollBar`s range new
  hBar->setRange( scrollArea_->preferredWidth()  - scrollArea_->clientWidth()  );
  vBar->setRange( scrollArea_->preferredHeight() - scrollArea_->clientHeight() );
}

void SequencerGUI::update( )
{
  lines.clear();
  scrollArea_->removeChilds();
  std::vector<SequenceLine*>::iterator it = patternSequence()->begin();
  for ( ; it < patternSequence()->end(); it++) {
    SequenceLine* seqLine = *it;
    SequencerLine* line = new SequencerLine( this );
    line->itemClick.connect(this, &SequencerGUI::onSequencerItemClick);
    lines.push_back(line);
    line->setSequenceLine( seqLine );
    line->click.connect(this, &SequencerGUI::onSequencerLineClick);
    scrollArea_->add(line);
    scrollArea_->resize();
    lastLine = line;
    selectedLine_ = line;
    // now iterate the sequence entries
    SequenceLine::iterator iter = seqLine->begin();
    for(; iter!= seqLine->end(); ++iter)
    {
      SequenceEntry* entry = iter->second;
      SequencerItem* item = new SequencerItem( this );
      item->click.connect(line,&SequencerGUI::SequencerLine::onSequencerItemClick);
      item->setSequenceEntry( entry );
      line->items.push_back(item);
      line->add(item);

    }
  }
  resize();
}

const std::vector< SequencerItem * > & SequencerGUI::selectedItems( )
{
  return selectedItems_;
}

NPanel * SequencerGUI::scrollArea( )
{
  return scrollArea_;
}

void SequencerGUI::deselectAll( )
{
  std::vector<SequencerItem*>::iterator it = selectedItems_.begin();
  for ( ; it < selectedItems_.end(); it++) {
    SequencerItem* selectedItem = *it;
    selectedItem->setSelected(false);
    selectedItem->repaint();
   }
   selectedItems_.clear();
}

}}










