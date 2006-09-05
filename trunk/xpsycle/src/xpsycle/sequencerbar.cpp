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
#include "sequencergui.h"

#include <ngrs/nlabel.h>
#include <ngrs/nitem.h>
#include <ngrs/napp.h>
#include <ngrs/nconfig.h>
#include <ngrs/n7segdisplay.h>
#include <ngrs/nframeborder.h>
#include <ngrs/ntoolbar.h>
#include <ngrs/ngridlayout.h>
#include <ngrs/ntreenode.h>
#include <ngrs/nobjectinspector.h>
#include <ngrs/nproperty.h>


namespace psycle { namespace host {


CategoryTreeNode::CategoryTreeNode( PatternCategory * cat )
{
  cat_ = cat;
}

CategoryTreeNode::~ CategoryTreeNode( )
{
}

PatternCategory * CategoryTreeNode::category( )
{
  return cat_;
}



CategoryItem::CategoryItem( PatternCategory* category , const std::string & text )
{
  init();
  label_->setText(text);
  category_ = category;
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
  g->setForeground( NColor(category_->color()) );
  g->drawRect(0,0,clientWidth()-1, clientHeight()-1);

  int cw = clientWidth();

  XPoint pts[3];
  pts[0].x = cw-6;
  pts[0].y = 0;
  pts[1].x = cw-1;
  pts[1].y = 0;
  pts[2].x = cw-1;
  pts[2].y = 6;

  g->fillPolygon(pts,3);
}

PatternCategory * CategoryItem::category( )
{
  return category_;
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
  seqGui = 0;
  patView = 0;

  NFrameBorder frBorder;
    frBorder.setOval();
    frBorder.setLineCount(2,4,4);
  setBorder(frBorder);

  setLayout(NAlignLayout());
  setWidth(90);

  NFlipBox* flipBox = new NFlipBox();
    NFrameBorder fr;
    flipBox->setBorder(fr);
    flipBox->header()->add(new NLabel("Pattern box"), nAlClient);

    NPanel* patternPanel = new NPanel();
    patternPanel->setLayout( NAlignLayout() );

    NToolBar* patToolBar = new NToolBar();
      NImage* img = new NImage();
      img->setSharedBitmap(&icons.new_category());
      img->setPreferredSize(25,25);
      NButton* newCatBtn = new NButton(img);
        newCatBtn->setHint("New Category");
      patToolBar->add( newCatBtn )->clicked.connect(this,&SequencerBar::onNewCategory);
      patToolBar->add( new NButton("New Pattern"))->clicked.connect(this,&SequencerBar::onNewPattern);
      img = new NImage();
      img->setSharedBitmap(&icons.delPattern());
      img->setPreferredSize(25,25);
      NButton* delPatBtn = new NButton(img);
        delPatBtn->setHint("Delete Category");
      patToolBar->add( delPatBtn )->clicked.connect(this,&SequencerBar::onDeletePattern);


      patToolBar->add( new NButton("Add"))->clicked.connect(this,&SequencerBar::onPatternAdd);
    patternPanel->add(patToolBar, nAlTop);

    patternBox_ = new NCustomTreeView();
      patternBox_->setPreferredSize(100,300);
      patternBox_->itemSelected.connect(this,&SequencerBar::onItemSelected);
    patternPanel->add(patternBox_, nAlClient);

    flipBox->pane()->add(patternPanel, nAlClient);
    flipBox->setExpanded(true);
    add(flipBox, nAlTop);

    propertyBox_ = new PatternBoxProperties();
       propertyBox_->nameChanged.connect(this,&SequencerBar::onNameChanged);
    add(propertyBox_, nAlTop);

    flipBox = new NFlipBox();
      entryBox_ = new NObjectInspector();
        entryBox_->setPreferredSize(100,100);
      flipBox->setBorder(fr);
      flipBox->header()->add(new NLabel("Sequence Properties"), nAlClient);

    flipBox->pane()->add(entryBox_, nAlClient);

    add(flipBox, nAlTop);



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

void SequencerBar::setSequenceGUI(SequencerGUI* sequenceGUI)
{
  seqGui = sequenceGUI;
}

void SequencerBar::setPatternView( PatternView * patternView )
{
  patView = patternView;
}

void SequencerBar::update()
{
  patternBox_->removeChilds();
  categoryMap.clear();
  catItems.clear();
  patternMap.clear();

  std::vector<PatternCategory*>::iterator it = seqGui->patternSequence()->patternData()->begin();
  for ( ; it < seqGui->patternSequence()->patternData()->end(); ++it) {
    PatternCategory* category = *it;
    CategoryTreeNode* node = new CategoryTreeNode(category);
    node->setExpanded(true);
    categoryMap[node]=category;
    CategoryItem* catItem = new CategoryItem(category,"Category");
    catItems.push_back(catItem);
    node->setHeader(catItem);
    patternBox_->addNode(node);
    std::vector<SinglePattern*>::iterator patIt = category->begin();
    for ( ; patIt < category->end(); patIt++) {
       SinglePattern* pattern = *patIt;
       PatternItem* item = new PatternItem( pattern, pattern->name() );
       node->addEntry(item);
       patternMap[item] = pattern;
    }
  }
  resize();
}

bool SequencerBar::followSong( ) const
{
  //return follow_->checked();
return false;
}

void SequencerBar::onMoveCursorPaste( NButtonEvent * ev )
{
   //patternView_->setMoveCursorWhenPaste(movecursorpaste_->checked() );
}


void SequencerBar::onRecordTweakChange( NButtonEvent * ev )
{
 // Global::configuration()._RecordTweaks = record_tweaks_->checked();
}

void SequencerBar::onNewCategory( NButtonEvent * ev )
{
  PatternCategory* category = seqGui->patternSequence()-> patternData()->createNewCategory("category");
	category->setColor(0x0000FF);

  CategoryTreeNode* node = new CategoryTreeNode(category);
  node->setExpanded(true);
  categoryMap[node]=category;
  CategoryItem* catItem = new CategoryItem(category,"Category");
  catItems.push_back(catItem);
  node->setHeader(catItem);

  patternBox_->addNode(node);
  patternBox_->resize();
  patternBox_->repaint();
}

void SequencerBar::onNewPattern( NButtonEvent * ev )
{
  if (patternBox_->selectedTreeNode() ) {
     NTreeNode* node = patternBox_->selectedTreeNode();

     std::map<NTreeNode*, PatternCategory*>::iterator itr = categoryMap.find(node);
     if(itr != categoryMap.end()) {
        PatternCategory* cat = itr->second;
        SinglePattern* pattern = cat->createNewPattern("Pattern" + stringify(counter) );
        PatternItem* item = new PatternItem( pattern, pattern->name() );
        node->addEntry(item);
        patternMap[item] = pattern;
        patternBox_->resize();
        patternBox_->repaint();
        counter++;
     }
  }
}

void SequencerBar::onDeletePattern( NButtonEvent* ev ) {
  NCustomItem* item = patternBox_->selectedItem();
  std::map<NCustomItem*, SinglePattern*>::iterator itr = patternMap.find(item);

  if(itr!=patternMap.end())
  {
     SinglePattern* pattern = itr->second;
     patternMap.erase(itr);
     ((NVisualComponent*)item->parent())->erase(item);
     //item->erase(); ///\todo put this in customtreeview
     NApp::addRemovePipe(item);
     patternBox_->resize(); ///\todo end
     patternBox_->repaint();

     seqGui->removePattern(pattern);
     seqGui->repaint();

     if (patView) {
       if (patView->pattern() == pattern) {
         patView->setPattern(0);
         patView->repaint();
       }
     }

     seqGui->patternSequence()->removeSinglePattern(pattern);
  }
}

void SequencerBar::selectNextPattern() {
	NCustomItem* item = patternBox_->selectedItem();
  std::map<NCustomItem*, SinglePattern*>::iterator itr = patternMap.find(item);
  
  if ( itr != patternMap.end() ) {
		itr++;
		if (itr != patternMap.end() ) {
			// todo under construction
		}
	}
}

void SequencerBar::selectPrevPattern() {
  NCustomItem* item = patternBox_->selectedItem();
  std::map<NCustomItem*, SinglePattern*>::iterator itr = patternMap.find(item);

	if ( itr != patternMap.end() && itr != patternMap.begin() ) {
		itr--;
		// todo under construction
	}
}

}}



void psycle::host::SequencerBar::onItemSelected( NItemEvent * ev )
{
  NCustomItem* item = patternBox_->selectedItem();

  std::vector<CategoryItem*>::iterator it = find(catItems.begin(),catItems.end(),item);
  if ( it != catItems.end() )  propertyBox_->setCategoryItem(*it);

  if (item) propertyBox_->setName( item->text() );
  std::map<NCustomItem*, SinglePattern*>::iterator itr = patternMap.find(item);
  if(itr!=patternMap.end()) {
     patView->setPattern(itr->second);
     patView->repaint();
  }
}

void psycle::host::SequencerBar::onPatternAdd( NButtonEvent * ev )
{
  NCustomItem* item = patternBox_->selectedItem();
  if (item) {
    std::map<NCustomItem*, SinglePattern*>::iterator itr = patternMap.find(item);
    if(itr!=patternMap.end())
      seqGui->addPattern(itr->second);
  }
}

void psycle::host::SequencerBar::onNameChanged( const std::string & name )
{
  NCustomItem* item = patternBox_->selectedItem();
    item->setText(name);
  patternBox_->repaint();

  std::map<NCustomItem*, SinglePattern*>::iterator itr = patternMap.find(item);
  if(itr!=patternMap.end()) {
    std::vector<SequencerItem*> list = seqGui->guiItemsByPattern(itr->second);
    std::vector<SequencerItem*>::iterator it = list.begin();
    for ( ; it < list.end(); it++) {
       SequencerItem* guiItem = *it;
       guiItem->repaint();
    }
  }
}

void psycle::host::SequencerBar::setEntry( NObject * obj )
{
  entryBox_->setControlObject(obj);
  entryBox_->resize();
  repaint();
}























