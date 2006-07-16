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
#include "sequencerbar.h"
#include "global.h"
#include "song.h"
#include "configuration.h"
#include "patternview.h"
#include "defaultbitmaps.h"
#include "player.h"
#include <ngrs/nlabel.h>
#include <ngrs/nitem.h>
#include <ngrs/napp.h>
#include <ngrs/nconfig.h>
#include <ngrs/n7segdisplay.h>
#include <ngrs/nframeborder.h>
#include <ngrs/ntoolbar.h>
#include <ngrs/ngridlayout.h>
#include <ngrs/ntreenode.h>


namespace psycle { namespace host {


CategoryItem::CategoryItem( const std::string & text )
{
  init();
  label_->setText(text);
}

CategoryItem::CategoryItem( )
{
  init();
}


void CategoryItem::init( )
{
  setLayout( NAlignLayout(5,0) );
  label_ = new NLabel();
  add(label_, nAlLeft);
}


CategoryItem::~ CategoryItem( )
{
}

void CategoryItem::setText( const std::string & text )
{
  label_->setText(text);
}

std::string CategoryItem::text( ) const
{
  return label_->text();
}

void CategoryItem::paint( NGraphics * g )
{
  g->setForeground( NColor(255,0,0) );
  g->drawRect(0,0,clientWidth()-1, clientHeight()-1);
}



PatternItem::PatternItem( SinglePattern* pattern,  const std::string & text )
: NItem(text)
{
  pattern_ = pattern;
}

PatternItem::~ PatternItem( )
{
}

void PatternItem::setText( const std::string & text )
{
  pattern_->setName(text);
  NItem::setText(text);
}



SequencerBar::SequencerBar()
  : NPanel()
{
  init();
}



SequencerBar::~SequencerBar()
{
}

void SequencerBar::init( )
{
  DefaultBitmaps & icons = Global::pConfig()->icons();

  counter = 0;
  patternData_ = 0;

  skin_ = NApp::config()->skin("seqbar");

  NFrameBorder frBorder;
    frBorder.setOval();
    frBorder.setLineCount(2,4,4);
  setBorder(frBorder);

  setLayout(NAlignLayout());
  setWidth(90);

  NPanel* patternPanel = new NPanel();
    patternPanel->setLayout( NAlignLayout(0,5) );
    patternPanel->add(new NLabel("Patterns"), nAlTop);

    NToolBar* patToolBar = new NToolBar();
      NImage* img = new NImage();
      img->setSharedBitmap(&icons.new_category());
      img->setPreferredSize(25,25);
      NButton* newCatBtn = new NButton(img);
        newCatBtn->setHint("New Category");
      patToolBar->add( newCatBtn )->clicked.connect(this,&SequencerBar::onNewCategory);
      patToolBar->add( new NButton("New Pattern"))->clicked.connect(this,&SequencerBar::onNewPattern);
      patToolBar->add( new NButton("Add"))->clicked.connect(this,&SequencerBar::onPatternAdd);
    patternPanel->add(patToolBar, nAlTop);

    propertyBox_ = new PatternBoxProperties();
       propertyBox_->nameChanged.connect(this,&SequencerBar::onNameChanged);
    patternPanel->add(propertyBox_, nAlBottom);

    patternBox_ = new NCustomTreeView();
      patternBox_->setPreferredSize(100,200);
      patternBox_->itemSelected.connect(this,&SequencerBar::onItemSelected);

    patternPanel->add(patternBox_, nAlClient);

  add(patternPanel, nAlTop);

  ///\ todo remove this somewhere else and rename this class to patternBox

/*
  NPanel* checkPanel = new NPanel();
    //checkPanel->skin_.setTranslucent(NColor(200,200,200),70);
    checkPanel->setLayout(NListLayout());
    checkPanel->add( follow_                = new NCheckBox("Follow song"));
    checkPanel->add( multichannel_audition_ = new NCheckBox("Multichannel\nAudition"));
    checkPanel->add( record_noteoff_        = new NCheckBox("Record NoteOffs"));
    checkPanel->add( record_tweaks_         = new NCheckBox("Record Tweaks"));
    record_tweaks_->clicked.connect(this,&SequencerBar::onRecordTweakChange);
    checkPanel->add( notestoeffects_        = new NCheckBox("Allow Notes\nto Effects"));
    checkPanel->add( movecursorpaste_       = new NCheckBox("Move Cursor\nWhen Paste"));
    movecursorpaste_->clicked.connect(this,&SequencerBar::onMoveCursorPaste);
  add(checkPanel,nAlTop);
*/

//  seqList()->itemSelected.connect(this,&SequencerBar::onSelChangeSeqList);

}

void SequencerBar::setPatternData( PatternData * patternData )
{
  patternData_ = patternData;
}

void SequencerBar::updateSequencer()
{

}

bool SequencerBar::followSong( ) const
{
  return follow_->checked();
}

void SequencerBar::onMoveCursorPaste( NButtonEvent * ev )
{
   //patternView_->setMoveCursorWhenPaste(movecursorpaste_->checked() );
}


void SequencerBar::onRecordTweakChange( NButtonEvent * ev )
{
  Global::configuration()._RecordTweaks = record_tweaks_->checked();
}

void SequencerBar::onNewCategory( NButtonEvent * ev )
{
  NTreeNode* node = new NTreeNode();

  node->setHeader(new CategoryItem("Category"));

  patternBox_->addNode(node);
  patternBox_->resize();
  patternBox_->repaint();
}

void SequencerBar::onNewPattern( NButtonEvent * ev )
{
  if (patternBox_->selectedTreeNode() ) {
     NTreeNode* node = patternBox_->selectedTreeNode();
     SinglePattern* pattern = patternData_->createNewPattern("Pattern" + stringify(counter) );
     PatternItem* item = new PatternItem( pattern, pattern->name() );
     node->addEntry(item);
     patternMap[item] = pattern;
     patternBox_->resize();
     patternBox_->repaint();
     counter++;
  }
}


}}



void psycle::host::SequencerBar::onItemSelected( NItemEvent * ev )
{
  NCustomItem* item = patternBox_->selectedItem();
  if (item) propertyBox_->setName( item->text() );
  std::map<NCustomItem*, SinglePattern*>::iterator itr = patternMap.find(item);
  if(itr!=patternMap.end())
     selected.emit(itr->second);
}

void psycle::host::SequencerBar::onPatternAdd( NButtonEvent * ev )
{
  NCustomItem* item = patternBox_->selectedItem();
  if (item) {
    std::map<NCustomItem*, SinglePattern*>::iterator itr = patternMap.find(item);
    if(itr!=patternMap.end())
      added.emit(itr->second);
  }
}

void psycle::host::SequencerBar::onNameChanged( const std::string & name )
{
  NCustomItem* item = patternBox_->selectedItem();
    item->setText(name);
  patternBox_->repaint();
}
















