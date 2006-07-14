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
#include <ngrs/nscrollbar.h>
#include <ngrs/nalignlayout.h>
#include <ngrs/nlabel.h>
#include <ngrs/nframeborder.h>
#include <ngrs/ntoolbar.h>
#include <ngrs/ntoolbarseparator.h>
#include <ngrs/nlistbox.h>
#include <ngrs/nitem.h>

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
  g->setForeground(NColor(0,0,220));

  int cw = clientWidth();
  int ch = clientHeight();

  std::string timeScaleText = "t = bpm";
  int rightIdent = 2;
  int scaleTextWidth = g->textWidth(timeScaleText) + rightIdent;
  g->drawText(cw - scaleTextWidth, g->textAscent(), timeScaleText);

  g->setForeground(NColor(220,220,220));

  g->drawLine(0, ch - 10, cw, ch - 10);

  for (int i = 0; i < cw / sView->beatPxLength() ; i++) {
     if (! (i % 4)) {
        g->setForeground(NColor(180,180,180));
        g->drawLine(i* sView->beatPxLength(),ch-10,d2i(i*sView->beatPxLength()), ch);
        if (i * sView->beatPxLength() < cw - scaleTextWidth) {
          std::string beatLabel = stringify(i);
          int textWidth = g->textWidth(beatLabel);
          g->drawText(i* sView->beatPxLength() - textWidth / 2, g->textAscent(), beatLabel);
        }
     }
     else {
        g->setForeground(NColor(220,220,220));
        g->drawLine(i* sView->beatPxLength(),ch-10,d2i(i*sView->beatPxLength()), ch-5);
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
  setBackground(NColor(150,150,180));

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
  for (int i = 0; i < 1000; i++) {
     g->setForeground(NColor(220,220,220));
     g->drawLine(i* sView->beatPxLength(),0,d2i(i*sView->beatPxLength()),clientHeight());
     if (i % 10) g->setForeground(NColor(180,180,180));
  }
}

// end of Area class


// this is the gui class of one pattern entry
SequencerGUI::SequencerLine::SequencerItem::SequencerItem( SequencerGUI* seqGui )
{
  caption_ = new NLabel("Pattern");
    caption_->setVAlign(nAlCenter);
    caption_->setHAlign(nAlCenter);
  add(caption_);

  setBorder (NFrameBorder());
  setMoveable(nMvHorizontal);

  setTransparent(false);

  sequenceEntry_ = 0;

  sView = seqGui;
}

SequencerGUI::SequencerLine::SequencerItem::~ SequencerItem( )
{
}

void SequencerGUI::SequencerLine::SequencerItem::setSequenceEntry( SequenceEntry * sequenceEntry )
{
  sequenceEntry_ = sequenceEntry;

  if (sequenceEntry_) {
     caption_->setText( sequenceEntry_->pattern()->name() );
  }
}

SequenceEntry * SequencerGUI::SequencerLine::SequencerItem::sequenceEntry( )
{
  return sequenceEntry_;
}


void SequencerGUI::SequencerLine::SequencerItem::resize( )
{
  caption_->setPosition(0,0,clientWidth(), clientHeight());
}

void SequencerGUI::SequencerLine::SequencerItem::onMove( const NMoveEvent & moveEvent )
{
  sequenceEntry_->track()->MoveEntry(sequenceEntry_, left() / (double) sView->beatPxLength() );
//  caption_->setText( sequenceEntry_->pattern()->name() + ":" + stringify(sequenceEntry_->tickPosition()));
}

void SequencerGUI::SequencerLine::SequencerItem::onMousePress( int x, int y, int button )
{
}

// end of SequencerItem class



// this is the gui class that represents one SequenceLine

SequencerGUI::SequencerLine::SequencerLine( SequencerGUI* seqGui )
{
  sView = seqGui;
  lock = false;
}

SequencerGUI::SequencerLine::~ SequencerLine( )
{
  lock = true;
}

void SequencerGUI::SequencerLine::paint( NGraphics * g )
{
  int cw = clientHeight();
  g->drawLine(0 ,cw / 2 , clientWidth(), cw / 2);

  if (sView->selectedLine_ && sView->selectedLine_ == this) {
    g->setForeground(NColor(0,0,255));
    g->drawRect(0,0, clientWidth()-1, clientHeight()-1);
  }
}

void SequencerGUI::SequencerLine::addItem( SinglePattern* pattern )
{
  double endTick = sequenceLine()->tickLength();

  SequencerItem* item = new SequencerItem(sView);
    item->setPosition(d2i(sView->beatPxLength() * endTick),10,pattern->beats() * sView->beatPxLength() ,30);
    item->setSequenceEntry(sequenceLine()->createEntry(pattern, endTick));
    item->sequenceEntry()->beforeDelete.connect( this,&SequencerGUI::SequencerLine::onDeleteEntry);
    items.push_back(item);
  add(item);
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

void SequencerGUI::SequencerLine::onDeleteEntry( SequenceEntry * entry )
{
  if (!lock) {
    std::vector<SequencerItem*>::iterator it = items.begin();
    for ( ; it < items.end(); it++) {
      SequencerItem* item = *it;
      if (item->sequenceEntry() == entry) {
         items.erase(it);
         removeChild(item);
         repaint();
         break;
      }
    }
  }
}

// main class

SequencerGUI::SequencerGUI()
 : NPanel()
{
  setLayout( NAlignLayout() );

  counter = 0;
  beatPxLength_ = 20; // default value for one beat

  toolBar_ = new NToolBar();
    toolBar_->add( new NButton("New"))->clicked.connect(this,&SequencerGUI::onNewTrack);
    toolBar_->add( new NButton("Insert"));
    toolBar_->add( new NButton("Delete"));
  add(toolBar_, nAlTop);

  beatLineal_ = new SequencerBeatLineal(this);
  add(beatLineal_, nAlTop);

  // create scrollBars
  hBar = new NScrollBar();
    hBar->setOrientation(nHorizontal);
    hBar->setHeight(15);
    //hBar->posChange.connect(this,&PatternView::onHScrollBar);
  add(hBar, nAlBottom);

  beatChangeLineal_ = new SequencerBeatChangeLineal(this);
  add( beatChangeLineal_, nAlBottom );

  vBar = new NScrollBar();
    vBar->setWidth(15);
    vBar->setOrientation(nVertical);
    //vBar->posChange.connect(this,&PatternView::onVScrollBar);
  add(vBar, nAlRight);

  // the main Sequencer gui
  scrollArea_ = new Area( this );
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
  line->setSequenceLine( patternSequence_->createNewLine() );
  line->click.connect(this, &SequencerGUI::onSequencerLineClick);
  if (!lastLine)
     line->setPosition(0,0,1000,50);
  else
     line->setPosition(0,lastLine->top() + lastLine->height(),1000,50);
  scrollArea_->add(line);

  lastLine = line;

}

void SequencerGUI::onNewTrack( NButtonEvent * ev )
{
  addSequencerLine();
  repaint();
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
  repaint();
}

void SequencerGUI::addPattern( SinglePattern * pattern )
{
  if ( selectedLine_ ) {
    selectedLine_->addItem( pattern );
    selectedLine_->repaint();
 }
}



}}


