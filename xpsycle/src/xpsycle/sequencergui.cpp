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
#include <ngrs/nscrollbox.h>
#include <ngrs/nautoscrolllayout.h>
#include <ngrs/nalignlayout.h>
#include <ngrs/nlabel.h>
#include <ngrs/nframeborder.h>
#include <ngrs/ntoolbar.h>
#include <ngrs/ntoolbarseparator.h>
#include <ngrs/nlistbox.h>
#include <ngrs/nitem.h>

namespace psycle {
	namespace host {


// this is the sequencer Area

SequencerGUI::Area::Area( )
{
  setBackground(NColor(150,150,180));
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
  int timeDx = 20;
  for (int i = 0; i < 1000; i++) {
     g->setForeground(NColor(220,220,220));
     g->drawLine(i*timeDx,0,i*timeDx,clientHeight());
     if (i % 10) g->setForeground(NColor(180,180,180));
  }
}

// end of Area class


// this is the gui class of one pattern entry
SequencerGUI::SequencerLine::SequencerItem::SequencerItem( )
{
  caption_ = new NLabel("Pattern");
    caption_->setVAlign(nAlCenter);
    caption_->setHAlign(nAlCenter);
  add(caption_);

  setBorder (NFrameBorder());
  setMoveable(nMvHorizontal);

  setTransparent(false);

  sequenceEntry_ = 0;
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

// end of SequencerItem class


// this is the gui class that represents one SequenceLine

SequencerGUI::SequencerLine::SequencerLine( )
{
}

SequencerGUI::SequencerLine::~ SequencerLine( )
{
}

void SequencerGUI::SequencerLine::paint( NGraphics * g )
{
  int cw = clientHeight();
  g->drawLine(0 ,cw / 2 , clientWidth(), cw / 2);
}

void SequencerGUI::SequencerLine::addItem( SinglePattern* pattern )
{
  SequencerItem* item = new SequencerItem();
    item->setPosition(0,10,100,30);
    item->setSequenceEntry(sequenceLine()->createEntry(pattern, 0));
  add(item);
}

void SequencerGUI::SequencerLine::onMousePress( int x, int y, int button )
{
  click.emit(this);
}

// main class

SequencerGUI::SequencerGUI()
 : NPanel()
{
  setLayout( NAlignLayout() );

  counter = 0;

  toolBar_ = new NToolBar();
    toolBar_->add( new NButton("New"))->clicked.connect(this,&SequencerGUI::onNewTrack);
    toolBar_->add( new NButton("Insert"));
    toolBar_->add( new NButton("Delete"));
  add(toolBar_, nAlTop);

  scrollBox_ = new NScrollBox();
    scrollArea_ = new Area();
      scrollArea_->setLayout(NAutoScrollLayout());
      scrollArea_->setClientSizePolicy(nVertical + nHorizontal);
    scrollBox_->setScrollPane(scrollArea_);
  add(scrollBox_, nAlClient);

  lastLine = 0;
  selectedLine = 0;

  patternSequence_ = 0;
}


SequencerGUI::~SequencerGUI()
{
}

void SequencerGUI::setPatternSequence( PatternSequence * sequence )
{
  patternSequence_ = sequence;
}

void SequencerGUI::addSequencerLine( )
{
  SequencerLine* line = new SequencerLine();
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
  selectedLine = line;
}

void SequencerGUI::addPattern( SinglePattern * pattern )
{
  if (selectedLine) {
    selectedLine->addItem( pattern );
    selectedLine->repaint();
 }
}

void SequencerGUI::SequencerLine::setSequenceLine( SequenceLine * line )
{
  seqLine_ = line;
}

SequenceLine * SequencerGUI::SequencerLine::sequenceLine( )
{
  return seqLine_;
}



}}





























