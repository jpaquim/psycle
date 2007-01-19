/***************************************************************************
*   Copyright (C) 2006 by  Stefan Nattkemper   *
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
#include "zoombar.h"
#include "defaultbitmaps.h"
#include "configuration.h"
#include <psycore/singlepattern.h>
#include <ngrs/app.h>
#include <ngrs/scrollbar.h>
#include <ngrs/alignlayout.h>
#include <ngrs/label.h>
#include <ngrs/frameborder.h>
#include <ngrs/toolbar.h>
#include <ngrs/toolbarseparator.h>
#include <ngrs/listbox.h>
#include <ngrs/item.h>
#include <ngrs/window.h>
#include <ngrs/bevelborder.h>
#include <ngrs/property.h>
#include <ngrs/checkbox.h>
#include <ngrs/edit.h>
#include <ngrs/splitbar.h>
#include <algorithm>

#ifdef _MSC_VER
#undef min 
#undef max
#endif

namespace psy {
  namespace host {


    // this is the bpm lineal

    SequencerGUI::SequencerBeatLineal::SequencerBeatLineal(SequencerGUI* seqGui )
    {
      sView = seqGui;
      setBackground( ngrs::Color( 255, 255, 255) );
      setTransparent( false );
    }

    SequencerGUI::SequencerBeatLineal::~ SequencerBeatLineal( )
    {
    }

    void SequencerGUI::SequencerBeatLineal::paint( ngrs::Graphics& g )
    {
      drawLineal( g, 0 );
    }

    void SequencerGUI::SequencerBeatLineal::drawLineal( ngrs::Graphics& g, int dx )
    {
      ngrs::Rect area = g.repaintArea().rectClipBox();

      int cw = clientWidth();
      int ch = clientHeight();

      g.setForeground( ngrs::Color(220,220,220) );

      g.drawLine(scrollDx(), ch - 10 , cw + scrollDx(), ch - 10);

      int start = (area.left() - absoluteLeft() + scrollDx()) / sView->beatPxLength();
      ///\ todo end seems not valid 
      int end   = (area.left() + area.width() - absoluteLeft() + scrollDx() ) / sView->beatPxLength();

      for (int i = start ; i < end ; i++) {
        if (! (i % 16)) {
          g.setForeground( ngrs::Color( 180, 180, 180) );
          g.drawLine(i* sView->beatPxLength(),ch-10,d2i(i*sView->beatPxLength()), ch);
          std::string beatLabel = stringify(i/4);
          int textWidth = g.textWidth(beatLabel);
          g.drawText(i* sView->beatPxLength() - textWidth / 2, g.textAscent(), beatLabel);
        }
        else {
          if (sView->beatPxLength() > 3) {
            g.setForeground( ngrs::Color( 220, 220, 220) );
            g.drawLine(i* sView->beatPxLength(),ch-10,d2i(i*sView->beatPxLength()), ch-5);
          }
        }
      }
    }

    int SequencerGUI::SequencerBeatLineal::preferredHeight( ) const
    {
      ngrs::FontMetrics metrics(font());

      return metrics.textHeight() + 10;
    }

    // end of beat ruler





    // this is the sequencer Area

    SequencerGUI::Area::Area( SequencerGUI* seqGui )
    {

      setTransparent(false);
      lockPlayLine_ = false;

      sView = seqGui;

      vLine_ = new ngrs::Panel();
      vLine_->setBackground( SkinReader::Instance()->sequencerview_info().pane_move_line_color );
      vLine_->setTransparent(false);
      vLine_->setWidth(1);
      vLine_->setVisible( false );
      add( vLine_ );

      pLine_ = new ngrs::Panel();
      pLine_->setBackground( SkinReader::Instance()->sequencerview_info().pane_play_line_color );
      pLine_->setTransparent(false);
      pLine_->setMoveable( ngrs::nMvHorizontal | ngrs::nMvParentLimit);
      pLine_->setWidth(1);
      pLine_->moveStart.connect( this, &SequencerGUI::Area::onMoveStart);
      pLine_->moveEnd.connect( this, &SequencerGUI::Area::onMoveEnd);
      pLine_->move.connect( this, &SequencerGUI::Area::onMove);		
      pLine_->setCursor( ngrs::nCrVSplit );
      add( pLine_ );

      playPos_ = 0;
    }

    SequencerGUI::Area::~ Area( )
    {
    }

    bool SequencerGUI::Area::lockPlayLine() const {
      return lockPlayLine_;
    }

    void SequencerGUI::Area::paint( ngrs::Graphics& g )
    {
      drawTimeGrid(g);
    }


    void SequencerGUI::Area::drawTimeGrid( ngrs::Graphics& g )
    {
      ngrs::Rect area = g.repaintArea().rectClipBox();

      int start = (area.left() - absoluteLeft() + scrollDx()) / sView->beatPxLength();
      int end   = (area.left() + area.width() - absoluteLeft() + scrollDx() ) / sView->beatPxLength();

      for (int i = start ; i <= end ; i++) {
        if ( sView->beatPxLength() > 3 || (sView->beatPxLength() <= 3 && (!( i %16)))  ) {
          g.setForeground( SkinReader::Instance()->sequencerview_info().pane_grid_color );
          g.drawLine(i* sView->beatPxLength(),-scrollDy(),d2i(i*sView->beatPxLength()),clientHeight()+scrollDy());
        }
      }
    }

    ngrs::Panel* SequencerGUI::Area::vLine() {
      return vLine_;
    }

    ngrs::Panel* SequencerGUI::Area::vLine() const {
      return vLine_;
    }

    ngrs::Panel* SequencerGUI::Area::pLine() {
      return pLine_;
    }

    ngrs::Panel* SequencerGUI::Area::pLine() const {
      return pLine_;
    }

    int SequencerGUI::Area::preferredHeight( ) const
    {
      std::vector<ngrs::VisualComponent*>::const_iterator itr = visualComponents().begin();

      int yp = 0;

      for (;itr < visualComponents().end(); itr++) {
        ngrs::VisualComponent* visualChild = *itr;
        if ( visualChild != vLine() && visualChild != pLine() )
          yp += visualChild->preferredHeight();
      }

      return yp;
    }

    int SequencerGUI::Area::preferredWidth( ) const
    {
      std::vector<ngrs::VisualComponent*>::const_iterator itr = visualComponents().begin();

      int xp = 100;

      for (;itr < visualComponents().end(); itr++) {
        ngrs::VisualComponent* visualChild = *itr;
        if (visualChild != vLine() && visualChild != pLine() )
          xp = std::max(visualChild->preferredWidth(), xp);
      }

      return xp;
    }

    void SequencerGUI::Area::removeChilds() {
      ngrs::Panel::removeChilds();
      vLine_ = new ngrs::Panel();
      vLine_->setBackground( SkinReader::Instance()->sequencerview_info().pane_move_line_color );
      vLine_->setTransparent( false );
      vLine_->setVisible(false);
      vLine_->setWidth(1);
      add(vLine_);

      pLine_ = new ngrs::Panel();
      pLine_->setBackground( SkinReader::Instance()->sequencerview_info().pane_play_line_color );
      pLine_->setTransparent(false);
      pLine_->setWidth(1);
      pLine_->setMoveable( ngrs::nMvHorizontal | ngrs::nMvParentLimit );
      pLine_->moveStart.connect( this, &SequencerGUI::Area::onMoveStart );
      pLine_->moveEnd.connect( this, &SequencerGUI::Area::onMoveEnd );
      pLine_->move.connect( this, &SequencerGUI::Area::onMove );		
      pLine_->setWidth(1);
      pLine_->setCursor( ngrs::nCrVSplit );
      add(pLine_);
    }

    void SequencerGUI::Area::resize( )
    {
      std::vector<ngrs::VisualComponent*>::const_iterator itr = visualComponents().begin();

      int xp = clientWidth();

      for (;itr < visualComponents().end(); itr++) {
        ngrs::VisualComponent* visualChild = *itr;
        if ( visualChild != vLine() && visualChild != pLine() ) 
          xp = std::max(visualChild->preferredWidth(), xp);
        if ( visualChild == pLine() ) {
          if ( sView && sView->patternSequence() ) {
            //int xPos =  std::min( d2i(sView->beatPxLength() * sView->patternSequence()->tickLength()), d2i(/*Player::Instance()->playPos() * sView->beatPxLength()) );
            //pLine_->setPosition(  xPos, scrollDy(), 1, clientHeight() );
          }
        }
      }

      itr = visualComponents().begin();

      int yp = 0;

      for (;itr < visualComponents().end(); itr++) {
        ngrs::VisualComponent* visualChild = *itr;
        if ( visualChild != vLine() && visualChild != pLine() ) {
          visualChild->setHeight( visualChild->preferredHeight() );
          visualChild->setWidth(xp);
          visualChild->setTop(yp);
          yp+= visualChild->preferredHeight();
        }
      }
    }

    void SequencerGUI::Area::onMove(const ngrs::MoveEvent & moveEvent) {
      newBeatPos_ = pLine_->left() / (double) sView->beatPxLength();
    }

    void SequencerGUI::Area::onMoveEnd( const ngrs::MoveEvent & moveEvent ) {
/*      Player::Instance()->stop();
      Player::Instance()->setPlayPos( newBeatPos_ );*/
      lockPlayLine_ = false;
    }

    void SequencerGUI::Area::onMoveStart( const ngrs::MoveEvent & moveEvent ) {
      lockPlayLine_ = true;
/*      playing_ = Player::Instance()->playing();
      newBeatPos_ = pLine_->left() / static_cast<double>( sView->beatPxLength() );*/
    }
    /// end of Area class







    ///
    /// this is the gui class of one pattern entry
    ///
    SequencerItem::SequencerItem( SequencerGUI* seqGui )
    {
      setMoveable( ngrs::Moveable( ngrs::nMvHorizontal | ngrs::nMvNoneRepaint | ngrs::nMvLeftLimit));

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

      hint_ = new ngrs::NHint();
      hint_->setText("beat: ");
      add( hint_ );
    }

    SequencerItem::~ SequencerItem( )
    {
    }

    ngrs::NHint* SequencerItem::hint() {
      return hint_;
    }

    void SequencerItem::setSequenceEntry( psy::core::SequenceEntry * sequenceEntry )
    {
      sequenceEntry_ = sequenceEntry;
    }

    void SequencerItem::paint( ngrs::Graphics& g )
    {
      int cw = clientWidth();
      int ch = clientHeight();

      int tw = g.textWidth(sequenceEntry_->pattern()->name());

      int xp = (cw - tw) / 2;
      int yp = (ch + g.textHeight() /2 ) / 2;

      g.setForeground( ngrs::Color( sequenceEntry_->pattern()->category()->color() ));
      g.fillRect(0,0, clientWidth(), clientHeight() );
      g.drawText( xp, yp, sequenceEntry_->pattern()->name());

      if (selected_) {
        g.setForeground( ngrs::Color( 0,0,255) );
        g.drawRect(0,0, clientWidth()-1, clientHeight()-1 );
      }  else {
        g.setForeground( ngrs::Color( 180,180,180) );
        g.drawRect(0,0, clientWidth()-1, clientHeight()-1 );
      }
    }

    psy::core::SequenceEntry * SequencerItem::sequenceEntry( )
    {
      return sequenceEntry_;
    }


    void SequencerItem::resize( )
    {

    }


    ngrs::Region SequencerItem::entriesInRegion( )
    {
      const std::vector<SequencerItem*> & selItems = sView->selectedItems();

      ngrs::Region region;

      for (std::vector<SequencerItem*>::const_iterator it = selItems.begin(); it < selItems.end(); it++) {
        SequencerItem* item = *it;
        ngrs::Region itemregion = item->geometry()->region();
        int parentAbsLeft = ((ngrs::VisualComponent*) item->parent())->absoluteLeft() - ((VisualComponent*) item->parent())->scrollDx();
        int parentAbsTop  = ((ngrs::VisualComponent*) item->parent())->absoluteTop() - ((VisualComponent*) item->parent())->scrollDy();;
        itemregion.move(parentAbsLeft, parentAbsTop);
        region = region | itemregion;
      }

      return region;
    }

    void SequencerItem::onMoveStart( const ngrs::MoveEvent& moveEvent )
    {
      oldDrag = entriesInRegion();
      oldLeft = left();  

      ngrs::Panel* line = sView->scrollArea()->vLine();
      line->setPosition( left(), sView->scrollArea()->scrollDy(), 1, sView->scrollArea()->clientHeight() );
      line->setVisible(true);
      line->repaint();

      /*if (hint_ && !hint_->mapped()) {
      hint_->setPosition(window()->left()+ absoluteLeft() + (int) ((3*spacingWidth())/4), window()->top()+absoluteTop() + spacingHeight(),100,100);
      hint_->pack();
      hint_->setVisible(true);
      }*/
    }

    void SequencerItem::onMove( const ngrs::MoveEvent & moveEvent )
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

      ngrs::Panel* line = sView->scrollArea()->vLine();


      ngrs::Region newDrag = entriesInRegion();
      ngrs::Region repaintArea = newDrag | oldDrag | line->absoluteGeometry();

      line->setPosition( left(), sView->scrollArea()->scrollDy(), 1, sView->scrollArea()->clientHeight() );

      repaintArea |= line->absoluteGeometry();

      window()->repaint(sView->scrollArea(),repaintArea);

      oldDrag = newDrag;

      //hint_->setText("beat: "+ stringify( sequenceEntry()->tickPosition()  ));
      //hint_->pack();
      //hint_->pane()->repaint();

      //hint_->setPosition(window()->left()+ absoluteLeft() + (int) ((3*spacingWidth())/4), window()->top()+absoluteTop() + spacingHeight(),hint_->width(),hint_->height());


    }

    void SequencerItem::onMoveEnd( const ngrs::MoveEvent & moveEvent )
    {
      ngrs::Panel* line = sView->scrollArea()->vLine();
      line->setVisible( false );
      sView->scrollArea()->repaint();

      hint_->setVisible(false);
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
      setBorder( ngrs::BevelBorder( ngrs::nNone, ngrs::nLowered ) );
    }

    SequencerGUI::SequencerLine::~ SequencerLine( )
    {
    }

    void SequencerGUI::SequencerLine::paint( ngrs::Graphics& g )
    {
      if ( sView->selectedLine_ && sView->selectedLine_ == this) {
        g.setForeground( ngrs::Color( 0, 0, 255) );
        g.drawRect(0,0, clientWidth()-1, clientHeight()-1);
      }
    }

    void SequencerGUI::SequencerLine::addItem( psy::core::SinglePattern* pattern )
    {
      double endTick = sequenceLine()->tickLength();

      SequencerItem* item = new SequencerItem(sView);
      item->click.connect(this,&SequencerGUI::SequencerLine::onSequencerItemClick);
      item->setPosition(d2i(sView->beatPxLength() * endTick),5, static_cast<int>( pattern->beats() * sView->beatPxLength() ) ,20);
      item->setSequenceEntry( sequenceLine()->createEntry(pattern, endTick) );
      items.push_back( item );
      add( item );

    }

    void SequencerGUI::SequencerLine::removeItems( psy::core::SinglePattern * pattern )
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

    void SequencerGUI::SequencerLine::removeChild( ngrs::VisualComponent * item )
    {
      std::list<SequencerItem*>::iterator it = find( items.begin(), items.end(), item );
      if (it != items.end() ) items.erase(it);
      ngrs::Panel::removeChild(item);
    }

    void SequencerGUI::SequencerLine::onMousePress( int x, int y, int button )
    {
      click.emit(this);
    }

    void SequencerGUI::SequencerLine::setSequenceLine( psy::core::SequenceLine * line )
    {
      seqLine_ = line;
    }

    psy::core::SequenceLine * SequencerGUI::SequencerLine::sequenceLine( )
    {
      return seqLine_;
    }


    void SequencerGUI::SequencerLine::resize( )
    {
      std::list<SequencerItem*>::iterator it = items.begin();
      for ( ; it != items.end(); it++) {
        SequencerItem* item = *it;
        double tickPosition = item->sequenceEntry()->tickPosition();
        psy::core::SinglePattern* pattern = item->sequenceEntry()->pattern();

        item->setPosition(d2i(sView->beatPxLength() * tickPosition),5, static_cast<int>( (item->endOffset()-item->start()) * sView->beatPxLength() ),20);
      }
    }

    std::vector<SequencerItem*> SequencerGUI::SequencerLine::itemsByPattern( psy::core::SinglePattern * pattern )
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
      : ngrs::Panel()
    {
      scrollArea_ = 0;
      setLayout( ngrs::AlignLayout() );
      patternSequence_ = 0;
      oldPlayPos_ = 0;

      counter = 0;
      beatPxLength_ = 5; // default value for one beat

      scrollArea_ = new Area( this );

      DefaultBitmaps & icons = SkinReader::Instance()->bitmaps();

      ngrs::Image* img;

      toolBar_ = new ngrs::ToolBar();
      img = new ngrs::Image();
      img->setSharedBitmap(&icons.addTrack());
      img->setPreferredSize(25,25);
      ngrs::Button* btn;
      btn = new ngrs::Button(img);
      btn->setHint("Insert Track ");
      toolBar_->add( btn )->clicked.connect(this,&SequencerGUI::onInsertTrack);

      img = new ngrs::Image();
      img->setSharedBitmap(&icons.deleteTrack());
      img->setPreferredSize(25,25);
      btn = new ngrs::Button(img);
      btn->setHint("Delete Track ");
      toolBar_->add( btn)->clicked.connect(this,&SequencerGUI::onDeleteTrack);

      img = new ngrs::Image();
      img->setSharedBitmap(&icons.moveDownTrack());
      img->setPreferredSize(25,25);
      btn = new ngrs::Button(img);
      btn->setHint("Move Down Track ");
      toolBar_->add( btn)->clicked.connect(this,&SequencerGUI::onMoveDownTrack);

      img = new ngrs::Image();
      img->setSharedBitmap(&icons.moveUpTrack());
      img->setPreferredSize(25,25);
      btn = new ngrs::Button(img);
      btn->setHint("Move Up Track ");
      toolBar_->add( btn)->clicked.connect(this,&SequencerGUI::onMoveUpTrack);

      toolBar_->add(new ngrs::ToolBarSeparator());

      toolBar_->add( new ngrs::Button("Add Loop")) ->clicked.connect(this,&SequencerGUI::onAddLoop);

      toolBar_->add( new ngrs::Button("Delete Entry"))->clicked.connect(this,&SequencerGUI::onDeleteEntry);
      snapToGridCheck_ = new ngrs::NCheckBox("Snap to Beat");
      snapToGridCheck_->setCheck(true);
      toolBar_->add( snapToGridCheck_ );

      toolBar_->add(new ngrs::ToolBarSeparator());

      toolBar_->add( renderBtn = new ngrs::Button("Render As Wave"))->clicked.connect(this,&SequencerGUI::onRenderAsWave);

      btn = new ngrs::Button("refresh");
      btn->setHint("Refresh gui ");
      toolBar_->add( btn )->clicked.connect(this,&SequencerGUI::onRefreshGUI);


      add(toolBar_, ngrs::nAlTop);

      beatLineal_ = new SequencerBeatLineal(this);
      add(beatLineal_, ngrs::nAlTop);


      // create scrollBars
      ngrs::Panel* hBarPanel = new ngrs::Panel();
      hBarPanel->setLayout( ngrs::AlignLayout() );
      zoomHBar = new ZoomBar();
      zoomHBar->setRange(2,50);
      zoomHBar->setPos(5);
      zoomHBar->posChanged.connect(this, &SequencerGUI::onZoomHBarPosChanged);
      hBarPanel->add(zoomHBar, ngrs::nAlRight);
      hBar = new ngrs::ScrollBar();
      hBar->setOrientation( ngrs::nHorizontal );
      hBar->setPreferredSize(100,15);
      hBar->change.connect(this,&SequencerGUI::onHScrollBar);
      hBarPanel->add(hBar, ngrs::nAlClient);
      add(hBarPanel, ngrs::nAlBottom);

      beatChangeLineal_ = new SequencerBeatChangeLineal(this);
      add( beatChangeLineal_, ngrs::nAlBottom );

      vBar = new ngrs::ScrollBar();
      vBar->setWidth(15);
      vBar->setOrientation( ngrs::nVertical );
      vBar->change.connect( this, &SequencerGUI::onVScrollBar );
      add(vBar, ngrs::nAlRight);

      // the main Sequencer gui
      add( scrollArea_, ngrs::nAlClient );

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

    void SequencerGUI::setPatternSequence( psy::core::PatternSequence * sequence )
    {
      std::cout << "setted sequence" << std::endl;
      patternSequence_ = sequence;
    }

    psy::core::PatternSequence * SequencerGUI::patternSequence( )
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

    void SequencerGUI::onInsertTrack( ngrs::ButtonEvent * ev )
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

    void SequencerGUI::onDeleteTrack( ngrs::ButtonEvent * ev )
    {
      if (selectedLine_) {

        psy::core::SequenceLine* line = selectedLine_->sequenceLine();

        std::vector<SequencerLine*>::iterator it = find(lines.begin(), lines.end(), selectedLine_);
        if (it != lines.end() ) lines.erase(it);

        scrollArea_->removeChild(selectedLine_);
        scrollArea_->resize();

        patternSequence_->removeLine(line);
        scrollArea_->repaint();
        selectedLine_ = 0;
      }
    }

    void SequencerGUI::onMoveDownTrack( ngrs::ButtonEvent * ev )
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

    void SequencerGUI::onMoveUpTrack( ngrs::ButtonEvent * ev )
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

    void SequencerGUI::onNewPattern( ngrs::ButtonEvent * ev )
    {
      patternBox_->add(new ngrs::Item("Pattern" + stringify(counter) ));
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
        if ( ngrs::App::system().shiftState() & ngrs::nsCtrl ) {
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

    void SequencerGUI::addPattern( psy::core::SinglePattern * pattern )
    {
      if ( selectedLine_ ) {
        selectedLine_->addItem( pattern );
        resize();
        repaint();
      }
    }

    void SequencerGUI::onDeleteEntry( ngrs::ButtonEvent * ev )
    {
      std::vector<SequencerItem*>::iterator it = selectedItems_.begin();

      // deselect all
      for ( ; it < selectedItems_.end(); it++) {
        SequencerItem* selectedItem = *it;
        psy::core::SequenceEntry* entry = selectedItem->sequenceEntry();
        ngrs::VisualComponent* parentContainer = (VisualComponent*) (selectedItem->parent());
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

    void SequencerGUI::removePattern( psy::core::SinglePattern * pattern )
    {
      std::vector<SequencerLine*>::iterator it = lines.begin();
      for (; it < lines.end(); it++) {
        SequencerLine* line = *it;
        line->removeItems(pattern);
      }
    }

    std::vector<SequencerItem*> SequencerGUI::guiItemsByPattern( psy::core::SinglePattern * pattern )
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

    void SequencerGUI::onVScrollBar( ngrs::ScrollBar * sender )
    {
      int newPos = static_cast<int>( sender->pos() );
      if (newPos != scrollArea_->scrollDy() && newPos >= 0) {
        int diffY  = newPos - scrollArea_->scrollDy();
        if (diffY < scrollArea_->clientHeight()) {
          ngrs::Rect rect = scrollArea_->blitMove(0,diffY, scrollArea_->absoluteSpacingGeometry());
          scrollArea_->setScrollDy(newPos);
          window()->repaint(scrollArea_,rect);
        } else {
          scrollArea_->setScrollDy(newPos);
          scrollArea_->repaint();
        }
      }
    }

    void SequencerGUI::onHScrollBar( ngrs::ScrollBar * sender )
    {
      int newPos = static_cast<int>( sender->pos() );
      if (newPos != scrollArea_->scrollDx() && newPos >= 0) {

        int diffX  = newPos - scrollArea_->scrollDx();
        if (diffX < scrollArea_->clientWidth()) {
          // scrolls the area
          ngrs::Rect rect = scrollArea_->blitMove(diffX,0, scrollArea_->absoluteSpacingGeometry());
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
      ngrs::Panel::resize();
      // set the ScrollBar`s range new
      hBar->setRange( 0, scrollArea_->preferredWidth()  - scrollArea_->clientWidth()  );
      vBar->setRange( 0, scrollArea_->preferredHeight() - scrollArea_->clientHeight() );
    }

    void SequencerGUI::update( )
    {
      lines.clear();
      scrollArea_->removeChilds();

      std::vector<psy::core::SequenceLine*>::iterator it = patternSequence()->begin();
      for ( ; it < patternSequence()->end(); it++) {
        psy::core::SequenceLine* seqLine = *it;
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
        psy::core::SequenceLine::iterator iter = seqLine->begin();
        for(; iter!= seqLine->end(); ++iter)
        {
          psy::core::SequenceEntry* entry = iter->second;
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

    void SequencerGUI::onRenderAsWave( ngrs::ButtonEvent * ev )
    {
    }

    void SequencerGUI::onRecordingTimer( )
    {
    }

    void SequencerGUI::onAddLoop( ngrs::ButtonEvent* ev ) {
      SequencerLoopItem* item = new SequencerLoopItem(this);
      item->setPosition( 0, beatLineal_->preferredHeight()-10, 100,10);
      beatLineal_->add(item);
      beatLineal_->repaint();
    }


    void SequencerGUI::updateSkin() {
      scrollArea()->setBackground( SkinReader::Instance()->sequencerview_info().pane_bg_color );
      ngrs::Font font_ = scrollArea()->font();
      font_.setTextColor( SkinReader::Instance()->sequencerview_info().pane_text_color );
      scrollArea()->setFont( font_ );

      if ( scrollArea_->vLine() ) scrollArea_->vLine()->setForeground( SkinReader::Instance()->sequencerview_info().pane_move_line_color );

      if ( scrollArea_->pLine() ) scrollArea_->pLine()->setForeground( SkinReader::Instance()->sequencerview_info().pane_play_line_color );

    }


    void SequencerGUI::onRefreshGUI( ngrs::ButtonEvent* ev ) {
      update();
      resize();
      repaint();
    }

    void SequencerGUI::updatePlayPos() {
/*      if ( patternSequence() && scrollArea() && !scrollArea()->lockPlayLine() ) {
        int xPos =  d2i(std::min(patternSequence()->tickLength()* beatPxLength(), Player::Instance()->playPos() * beatPxLength()));
        int oxPos = d2i(std::min(patternSequence()->tickLength()* beatPxLength(), oldPlayPos_ * beatPxLength()));
        if (oxPos != xPos) {
          scrollArea()->pLine()->setPosition( xPos, scrollArea()->scrollDy(),1,scrollArea()->clientHeight());
          window()->repaint( scrollArea(), ngrs::Rect( scrollArea()->absoluteLeft() + oxPos, scrollArea()->absoluteTop(), 1, scrollArea()->clientHeight() ) );
          scrollArea()->pLine()->repaint();
        }
        oldPlayPos_ = Player::Instance()->playPos();
      }*/
    }

    /// loop item class

    SequencerLoopItem::SequencerLoopItem( SequencerGUI * seqGui )
    {
      sView = seqGui;
      setMoveable( ngrs::Moveable( ngrs::nMvHorizontal | ngrs::nMvLeftLimit | ngrs::nMvLeftBorder | ngrs::nMvRightBorder) );

      loopEdit = new ngrs::Edit("01");
      add(loopEdit);
    }

    SequencerLoopItem::~ SequencerLoopItem( )
    {
    }


    void SequencerLoopItem::paint( ngrs::Graphics& g )
    {
      int cw = clientWidth();
      int ch = clientHeight();
      g.setForeground( ngrs::Color( 0,0,255));
      // left border
      g.drawLine(0,0,0,ch-1);
      // draw dots left
      g.fillRect(2,ch / 2 -2,1,1);
      g.fillRect(2,ch / 2 +2,1,1);
      // right border
      g.drawLine(cw-1,0,cw-1,ch-1);
      g.drawLine(cw-3,0,cw-3,ch-1);
      // draw dots right
      g.fillRect(cw-5,ch / 2 -2,1,1);
      g.fillRect(cw-5,ch / 2 +2,1,1);
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

  }
}
