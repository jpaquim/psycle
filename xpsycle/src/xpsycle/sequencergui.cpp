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
#include "skinreader.h"
#include "sequencergui.h"
#include "sequencerbeatchangelineal.h"
#include "singlepattern.h"
#include "zoombar.h"
#include "global.h"
#include "player.h"
#include "wavefileout.h"
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
#include <ngrs/ncheckbox.h>
#include <ngrs/nedit.h>
#include <ngrs/nsplitbar.h>


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
	///\ todo end seems not valid 
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
  lockPlayLine_ = false;

  sView = seqGui;
  
  vLine_ = new NLine();
		vLine_->setForeground( SkinReader::Instance()->sequencerview_info().pane_move_line_color );
    vLine_->setVisible( false );
  add( vLine_ );

	pLine_ = new NLine();
		pLine_->setForeground( SkinReader::Instance()->sequencerview_info().pane_play_line_color );
		pLine_->setMoveable( nMvHorizontal | nMvParentLimit);
    pLine_->setClippingDistance( 3 );
		pLine_->moveStart.connect( this, &SequencerGUI::Area::onMoveStart);
    pLine_->moveEnd.connect( this, &SequencerGUI::Area::onMoveEnd);
    pLine_->move.connect( this, &SequencerGUI::Area::onMove);		
  add( pLine_ );

	playPos_ = 0;
}

SequencerGUI::Area::~ Area( )
{
}

bool SequencerGUI::Area::lockPlayLine() const {
  return lockPlayLine_;
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
       g->setForeground( SkinReader::Instance()->sequencerview_info().pane_grid_color );
       g->drawLine(i* sView->beatPxLength(),-scrollDy(),d2i(i*sView->beatPxLength()),clientHeight()+scrollDy());
     }
  }
}

NLine* SequencerGUI::Area::vLine() {
  return vLine_;
}

NLine* SequencerGUI::Area::vLine() const {
  return vLine_;
}

NLine* SequencerGUI::Area::pLine() {
	return pLine_;
}

NLine* SequencerGUI::Area::pLine() const {
  return pLine_;
}

int SequencerGUI::Area::preferredHeight( ) const
{
  std::vector<NVisualComponent*>::const_iterator itr = visualComponents().begin();

  int yp = 0;

  for (;itr < visualComponents().end(); itr++) {
    NVisualComponent* visualChild = *itr;
    if ( visualChild != vLine() && visualChild != pLine() )
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
    if (visualChild != vLine() && visualChild != pLine() )
      xp = std::max(visualChild->preferredWidth(), xp);
  }

  return xp;
}

void SequencerGUI::Area::removeChilds() {
  NPanel::removeChilds();
  vLine_ = new NLine();
		vLine_->setForeground( SkinReader::Instance()->sequencerview_info().pane_move_line_color );
    vLine_->setVisible(false);
  add(vLine_);

  pLine_ = new NLine();
		pLine_->setForeground( SkinReader::Instance()->sequencerview_info().pane_play_line_color );
    pLine_->setClippingDistance( 3 );
    pLine_->setMoveable( nMvHorizontal | nMvParentLimit );
		pLine_->moveStart.connect( this, &SequencerGUI::Area::onMoveStart);
    pLine_->moveEnd.connect( this, &SequencerGUI::Area::onMoveEnd);
    pLine_->move.connect( this, &SequencerGUI::Area::onMove);		
  add(pLine_);
}

void SequencerGUI::Area::resize( )
{
  std::vector<NVisualComponent*>::const_iterator itr = visualComponents().begin();

  int xp = clientWidth();

  for (;itr < visualComponents().end(); itr++) {
    NVisualComponent* visualChild = *itr;
    if ( visualChild != vLine() && visualChild != pLine() ) 
      xp = std::max(visualChild->preferredWidth(), xp);
		 if ( visualChild == pLine() ) {
			if ( sView && sView->patternSequence() ) {
				int xPos =  std::min( d2i(sView->beatPxLength() * sView->patternSequence()->tickLength()), d2i(Player::Instance()->PlayPos() * sView->beatPxLength()) );
				std::cout << Player::Instance()->PlayPos() << std::endl;
				pLine_->setPoints( NPoint( xPos,0 ), NPoint( xPos, clientHeight() ) );
			}
		 }
  }

  itr = visualComponents().begin();

  int yp = 0;

  for (;itr < visualComponents().end(); itr++) {
    NVisualComponent* visualChild = *itr;
    if ( visualChild != vLine() && visualChild != pLine() ) {
      visualChild->setHeight( visualChild->preferredHeight() );
      visualChild->setWidth(xp);
      visualChild->setTop(yp);
      yp+= visualChild->preferredHeight();
    }
  }
}

void SequencerGUI::Area::onMove(const NMoveEvent & moveEvent) {
	newBeatPos_ = pLine_->left() / (double) sView->beatPxLength();
}

void SequencerGUI::Area::onMoveEnd(const NMoveEvent & moveEvent) {
  Player::Instance()->Stop();
	if ( playing_ ) Player::Instance()->Start( newBeatPos_ );
  std::cout << "new beatpos is" << newBeatPos_ << std::endl;
  lockPlayLine_ = false;
}

void SequencerGUI::Area::onMoveStart(const NMoveEvent & moveEvent) {
  lockPlayLine_ = true;
	playing_ = Player::Instance()->_playing;
  newBeatPos_ = pLine_->left() / (double) sView->beatPxLength();
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
     properties()->bind("transpose", *this, &SequencerItem::transpose, &SequencerItem::setTranspose);
     properties()->publish("transpose");
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

  NLine* line = sView->scrollArea()->vLine();
    line->setPoints( NPoint( left(), sView->scrollArea()->scrollDy() ), NPoint( left(), sView->scrollArea()->clientHeight() + sView->scrollArea()->scrollDy() ) );
    line->setVisible(true);
  line->repaint();
}

void SequencerItem::onMove( const NMoveEvent & moveEvent )
{
  const std::vector<SequencerItem*> & selItems = sView->selectedItems();

  if (sView->gridSnap()) {
    int beatPos = left() / sView->beatPxLength();
    int newItemLeft = beatPos * sView->beatPxLength();
    setLeft(newItemLeft);
  }

  int moveDx = left() - oldLeft;

  oldLeft = left();


  for (std::vector<SequencerItem*>::const_iterator it = selItems.begin(); it < selItems.end(); it++) {
    SequencerItem* item = *it;
    int newLeft = 0;
    if (item != this) newLeft = item->left() + moveDx; else newLeft = left();
    item->sequenceEntry()->track()->MoveEntry(item->sequenceEntry(), newLeft / (double) sView->beatPxLength() );
  }
  sView->resize();

  NLine* line = sView->scrollArea()->vLine();
  

  NRegion newDrag = entriesInRegion();
  NRegion repaintArea = newDrag | oldDrag | line->absoluteGeometry();

  line->setPoints( NPoint( left(), sView->scrollArea()->scrollDy() ), NPoint( left(), sView->scrollArea()->clientHeight() + sView->scrollArea()->scrollDy() ) );

  repaintArea |= line->absoluteGeometry();

  window()->repaint(sView->scrollArea(),repaintArea);

  oldDrag = newDrag;

  
    
}

void SequencerItem::onMoveEnd( const NMoveEvent & moveEvent )
{
  NLine* line = sView->scrollArea()->vLine();
  line->setVisible( false );
  line->repaint();
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

void SequencerItem::setTranspose( int offset )
{
  sequenceEntry_->setTranspose( offset );
}

int SequencerItem::transpose( ) const
{
  return sequenceEntry_->transpose();
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
	scrollArea_ = 0;
  setLayout( NAlignLayout() );
	patternSequence_ = 0;
  oldPlayPos_ = 0;

  Player::Instance()->setFileName("test1.wav");

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

		toolBar_->add( new NButton("Add Loop")) ->clicked.connect(this,&SequencerGUI::onAddLoop);

    toolBar_->add( new NButton("Delete Entry"))->clicked.connect(this,&SequencerGUI::onDeleteEntry);
    snapToGridCheck_ = new NCheckBox("Snap to Beat");
    snapToGridCheck_->setCheck(true);
    toolBar_->add( snapToGridCheck_ );

		toolBar_->add(new NToolBarSeparator());

    toolBar_->add( renderBtn = new NButton("Render As Wave"))->clicked.connect(this,&SequencerGUI::onRenderAsWave);
	
    btn = new NButton("refresh");
    btn->setHint("Refresh gui ");
    toolBar_->add( btn )->clicked.connect(this,&SequencerGUI::onRefreshGUI);


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
      hBar->change.connect(this,&SequencerGUI::onHScrollBar);
    hBarPanel->add(hBar, nAlClient);
  add(hBarPanel, nAlBottom);

  beatChangeLineal_ = new SequencerBeatChangeLineal(this);
  add( beatChangeLineal_, nAlBottom );

  vBar = new NScrollBar();
    vBar->setWidth(15);
    vBar->setOrientation(nVertical);
    vBar->change.connect(this,&SequencerGUI::onVScrollBar);
  add(vBar, nAlRight);

  // the main Sequencer gui
  add(scrollArea_, nAlClient);

  lastLine = 0;
  selectedLine_ = 0;

  patternSequence_ = 0;

	updateSkin();

	
	recStatusTimer.setIntervalTime(100);
  recStatusTimer.timerEvent.connect(this,&SequencerGUI::onRecordingTimer);
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
  std::cout << "setted sequence" << std::endl;
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
    line->setSequenceLine( patternSequence_->insertNewLine( selectedLine_->sequenceLine()) );
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
     if ( i < componentZOrderSize() ) {
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

void SequencerGUI::onVScrollBar( NScrollBar * sender )
{
  int newPos = static_cast<int>( sender->pos() );
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

void SequencerGUI::onHScrollBar( NScrollBar * sender )
{
  int newPos = static_cast<int>( sender->pos() );
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
  hBar->setRange( 0, scrollArea_->preferredWidth()  - scrollArea_->clientWidth()  );
  vBar->setRange( 0, scrollArea_->preferredHeight() - scrollArea_->clientHeight() );
}

void SequencerGUI::update( )
{
  lines.clear();
  scrollArea_->removeChilds();
  
  std::vector<SequenceLine*>::iterator it = patternSequence()->begin();
  for ( ; it < patternSequence()->end(); it++) {
    SequenceLine* seqLine = *it;
    SequencerLine* line = new SequencerLine( this );
		std::cout << "lineitems : " << seqLine->size() << std::endl;
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

SequencerGUI::Area * SequencerGUI::scrollArea( )
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

bool SequencerGUI::gridSnap( ) const
{
  return snapToGridCheck_->checked();
}

void SequencerGUI::onRenderAsWave( NButtonEvent * ev )
{
  if (renderBtn->text()=="Stop rendering") {
    Player::Instance()->stopRecording();
    onRecordingTimer();
    return;
  }

	AudioDriver* recordDriver = 0;

  	// get recordDriver from Configuration

	std::map<std::string, AudioDriver*> & driverMap =  Global::pConfig()->driverMap();
	std::map<std::string, AudioDriver*>::iterator it = driverMap.find( "wavefileout" );
	if ( it != driverMap.end() ) {
				recordDriver = it->second;
	} else {
		return;
	}
	
	// stop player
  Player::Instance()->Stop();

  // disable driver
	Player::Instance()->driver().Enable( false );

	// save oldDriver
	oldDriver = Player::Instance()->driver();		

	// setRecordDriver to Player
  
	Player::Instance()->setDriver( *recordDriver );

	// change btn text
  renderBtn->setText("Stop rendering");
  toolBar_->resize();
  toolBar_->repaint();

  Player::Instance()->setAutoRecording(true);
	
  Player::Instance()->Start(0);

	recStatusTimer.enableTimer();
}


void SequencerGUI::onRecordingTimer( )
{
  if ( !Player::Instance()->recording() ) {
	  Player::Instance()->setDriver( oldDriver );	
		renderBtn->setText("Render As Wave");
		toolBar_->resize();
  	toolBar_->repaint();
		recStatusTimer.disableTimer();
	}
}

void SequencerGUI::onAddLoop(NButtonEvent* ev) {
  SequencerLoopItem* item = new SequencerLoopItem(this);
		item->setPosition( 0, beatLineal_->preferredHeight()-10, 100,10);
	beatLineal_->add(item);
	beatLineal_->repaint();
}


void SequencerGUI::updateSkin() {
	scrollArea()->setBackground( SkinReader::Instance()->sequencerview_info().pane_bg_color );
	NFont font_ = scrollArea()->font();
	font_.setTextColor( SkinReader::Instance()->sequencerview_info().pane_text_color );
	scrollArea()->setFont( font_ );

	if ( scrollArea_->vLine() ) scrollArea_->vLine()->setForeground( SkinReader::Instance()->sequencerview_info().pane_move_line_color );

  if ( scrollArea_->pLine() ) scrollArea_->pLine()->setForeground( SkinReader::Instance()->sequencerview_info().pane_play_line_color );

}

void SequencerGUI::onRefreshGUI(NButtonEvent* ev) {
  update();
	resize();
	repaint();
}

void SequencerGUI::updatePlayPos() {
	if ( patternSequence() && scrollArea() && !scrollArea()->lockPlayLine() ) {
	 int xPos =  d2i(std::min(patternSequence()->tickLength()* beatPxLength(), Player::Instance()->PlayPos() * beatPxLength()));
	 int oxPos = d2i(std::min(patternSequence()->tickLength()* beatPxLength(), oldPlayPos_ * beatPxLength()));
   if (oxPos != xPos) {
     scrollArea()->pLine()->setPoints( NPoint( xPos,0 ), NPoint( xPos, clientHeight() ) );
     scrollArea()->pLine()->repaint();
	 }
   oldPlayPos_ = Player::Instance()->PlayPos();
	}
}

/// loop item class

SequencerLoopItem::SequencerLoopItem( SequencerGUI * seqGui )
{
  sView = seqGui;
	setMoveable( NMoveable( nMvHorizontal | nMvLeftLimit | nMvLeftBorder | nMvRightBorder) );

	loopEdit = new NEdit("01");
	add(loopEdit);
}

SequencerLoopItem::~ SequencerLoopItem( )
{
}


void SequencerLoopItem::paint( NGraphics * g )
{
	int cw = clientWidth();
	int ch = clientHeight();
	g->setForeground( NColor( 0,0,255));
	// left border
	g->drawLine(0,0,0,ch-1);
	// draw dots left
	g->fillRect(2,ch / 2 -2,1,1);
	g->fillRect(2,ch / 2 +2,1,1);
	// right border
	g->drawLine(cw-1,0,cw-1,ch-1);
  g->drawLine(cw-3,0,cw-3,ch-1);
	// draw dots right
	g->fillRect(cw-5,ch / 2 -2,1,1);
	g->fillRect(cw-5,ch / 2 +2,1,1);
}

int SequencerLoopItem::preferredWidth() const {
  return 100;
}

int SequencerLoopItem::preferredHeight() const {
  return 10;
}

void SequencerLoopItem::resize() {
	loopEdit->setPosition( clientWidth() - loopEdit->preferredWidth() - 7, 0, loopEdit->preferredWidth(), clientHeight() );
}



}}





