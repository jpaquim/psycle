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
#include "skinreader.h"
#include "childview.h"

#include <ngrs/nlabel.h>
#include <ngrs/nitem.h>
#include <ngrs/napp.h>
#include <ngrs/nconfig.h>
#include <ngrs/n7segdisplay.h>
#include <ngrs/nframeborder.h>
#include <ngrs/ntoolbar.h>
#include <ngrs/ntoolbarseparator.h>
#include <ngrs/ngridlayout.h>
#include <ngrs/ntreenode.h>
#include <ngrs/nobjectinspector.h>
#include <ngrs/nproperty.h>

#include <algorithm>


namespace psycle { 
  namespace host {


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
      setLayout( ngrs::NAlignLayout(5,0) );
      label_ = new ngrs::NLabel();
      add(label_, ngrs::nAlLeft);
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

    void CategoryItem::paint( ngrs::NGraphics * g )
    {
      g->setForeground( ngrs::NColor(category_->color()) );
      g->drawRect(0,0,clientWidth()-1, clientHeight()-1);

      int cw = clientWidth();

      ngrs::NPoint pts[3];
      pts[0].setX( cw-6 );
      pts[0].setY( 0 );
      pts[1].setX( cw-1 );
      pts[1].setY( 0 );
      pts[2].setX( cw-1 );
      pts[2].setY( 6 );

      g->fillPolygon(pts,3);
    }

    PatternCategory * CategoryItem::category( )
    {
      return category_;
    }



    PatternItem::PatternItem( SinglePattern* pattern,  const std::string & text )
      : ngrs::NItem(text)
    {
      pattern_ = pattern;
    }

    PatternItem::~ PatternItem( )
    {
    }

    void PatternItem::setText( const std::string & text )
    {
      pattern_->setName(text);
      ngrs::NItem::setText(text);
    }

    SequencerBar::SequencerBar()
      : ngrs::NPanel()
    {
      init();
    }



    SequencerBar::~SequencerBar()
    {
    }


    void SequencerBar::setSkin( )
    {  
    }

    void SequencerBar::init( )
    {  
      DefaultBitmaps & icons = Global::pConfig()->icons();

      counter = 0;
      seqGui = 0;
      patView = 0;

      ngrs::NFrameBorder frBorder;
      frBorder.setOval();
      frBorder.setLineCount( 2, 4, 4 );
      setBorder( frBorder );

      setLayout( ngrs::NAlignLayout() );
      setWidth( 90 );

      setSkin();

      ngrs::NFlipBox* flipBox = new ngrs::NFlipBox();
      ngrs::NFrameBorder fr;
      flipBox->setBorder(fr);
      flipBox->header()->add(new ngrs::NLabel("Pattern box"), ngrs::nAlClient);

      ngrs::NPanel* patternPanel = new ngrs::NPanel();
      patternPanel->setLayout( ngrs::NAlignLayout() );

      ngrs::NToolBar* patToolBar = new ngrs::NToolBar();
      ngrs::NImage* img = new ngrs::NImage();
      img->setSharedBitmap(&icons.new_category());
      img->setPreferredSize(25,25);
      ngrs::NButton* newCatBtn = new ngrs::NButton(img);
      newCatBtn->setHint("New Category");
      patToolBar->add( newCatBtn )->clicked.connect(this,&SequencerBar::onNewCategory);
      patToolBar->add(new ngrs::NToolBarSeparator());
      img = new ngrs::NImage();
      img->setSharedBitmap( &icons.pattern_new() );
      ngrs::NButton* newPatBtn = new ngrs::NButton( img );
      newPatBtn->setHint("New Pattern");
      patToolBar->add( newPatBtn )->clicked.connect( this, &SequencerBar::onNewPattern );
      img = new ngrs::NImage();
      img->setSharedBitmap( &icons.delPattern() );
      img->setPreferredSize(25,25);
      ngrs::NButton* delPatBtn = new ngrs::NButton( img );
      delPatBtn->setHint( "Delete Pattern" );
      patToolBar->add( delPatBtn )->clicked.connect( this, &SequencerBar::onDeletePattern );
      ngrs::NButton* clnPatBtn = new ngrs::NButton("Cln");
      clnPatBtn->setHint("Clone Pattern");
      patToolBar->add( clnPatBtn )->clicked.connect(this, &SequencerBar::onClonePattern );

      ngrs::NButton* addPatBtn = new ngrs::NButton("Add");
      addPatBtn->setHint("Add Pattern To Sequencer");
      patToolBar->add( addPatBtn )->clicked.connect(this,&SequencerBar::onPatternAdd);
      patternPanel->add(patToolBar, ngrs::nAlTop);

      patternBox_ = new ngrs::NCustomTreeView();
      patternBox_->setPreferredSize( 100, 300 );
      patternBox_->itemSelected.connect(this,&SequencerBar::onItemSelected);
      patternPanel->add( patternBox_, ngrs::nAlClient);

      flipBox->pane()->add(patternPanel, ngrs::nAlClient);
      flipBox->setExpanded(true);
      add(flipBox, ngrs::nAlTop);

      propertyBox_ = new PatternBoxProperties();
      propertyBox_->nameChanged.connect(this,&SequencerBar::onNameChanged);
      add(propertyBox_, ngrs::nAlTop);

      flipBox = new ngrs::NFlipBox();
      entryBox_ = new ngrs::NObjectInspector();
      entryBox_->setPreferredSize(100,100);
      flipBox->setBorder(fr);
      flipBox->header()->add(new ngrs::NLabel("Sequence Properties"), ngrs::nAlClient);

      flipBox->pane()->add(entryBox_, ngrs::nAlClient);

      add(flipBox, ngrs::nAlTop);



      ///\ todo remove this somewhere else and rename this class to patternBox

      /*
      ngrs::NPanel* checkPanel = new ngrs::NPanel();
      //checkPanel->skin_.setTranslucent(NColor(200,200,200),70);
      checkPanel->setLayout(NListLayout());
      checkPanel->add( follow_                = new ngrs::NCheckBox("Follow song"));
      checkPanel->add( multichannel_audition_ = new ngrs::NCheckBox("Multichannel\nAudition"));
      checkPanel->add( record_noteoff_        = new ngrs::NCheckBox("Record ngrs::NoteOffs"));
      checkPanel->add( record_tweaks_         = new ngrs::NCheckBox("Record Tweaks"));
      record_tweaks_->clicked.connect(this,&SequencerBar::onRecordTweakChange);
      checkPanel->add( notestoeffects_        = new ngrs::NCheckBox("Allow ngrs::Notes\nto Effects"));
      checkPanel->add( movecursorpaste_       = new ngrs::NCheckBox("Move Cursor\nWhen Paste"));
      movecursorpaste_->clicked.connect(this,&SequencerBar::onMoveCursorPaste);
      add(checkPanel,ngrs::nAlTop);
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

      bool isFirst = true;
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
          item->mouseDoublePress.connect(this,&SequencerBar::onPatternItemDblClick);
          node->addEntry(item);
          patternMap[item] = pattern;
          if (isFirst) { 
            patternBox_->setSelectedItem( node, item ); 
            isFirst = false; 
          }
        }
      }
      resize();
    }

    bool SequencerBar::followSong( ) const
    {
      //return follow_->checked();
      return false;
    }

    void SequencerBar::onMoveCursorPaste( ngrs::NButtonEvent * ev )
    {
      //patternView_->setMoveCursorWhenPaste(movecursorpaste_->checked() );
    }


    void SequencerBar::onRecordTweakChange( ngrs::NButtonEvent * ev )
    {
      // Global::configuration()._RecordTweaks = record_tweaks_->checked();
    }

    void SequencerBar::onNewCategory( ngrs::NButtonEvent * ev )
    {
      PatternCategory* category = seqGui->patternSequence()-> patternData()->createNewCategory("category");
      category->setColor(0xFF0000);

      CategoryTreeNode* node = new CategoryTreeNode(category);
      node->setExpanded(true);
      categoryMap[node]=category;
      CategoryItem* catItem = new CategoryItem(category,"Category");
      catItems.push_back(catItem);
      node->setHeader(catItem);

      patternBox_->addNode(node);
      patternBox_->setSelectedItem( node, catItem );
      patternBox_->resize();
      patternBox_->repaint();
    }

    void SequencerBar::onNewPattern( ngrs::NButtonEvent * ev )
    {
      if (patternBox_->selectedTreeNode() ) {
        ngrs::NTreeNode* node = patternBox_->selectedTreeNode();

        std::map<ngrs::NTreeNode*, PatternCategory*>::iterator itr = categoryMap.find(node);
        if(itr != categoryMap.end()) {
          PatternCategory* cat = itr->second;
          SinglePattern* pattern = cat->createNewPattern("Pattern");
          pattern->setName("Pattern"+ stringify(pattern->id()) );
          PatternItem* item = new PatternItem( pattern, pattern->name() );
          item->mouseDoublePress.connect(this,&SequencerBar::onPatternItemDblClick);
          node->addEntry(item);
          patternMap[item] = pattern;
          patternBox_->setSelectedItem( node, item );
          patternBox_->resize();
          patternBox_->repaint();
          counter++;
        }
      }
    }

    void SequencerBar::onClonePattern( ngrs::NButtonEvent * ev )
    {
      ngrs::NCustomItem* item = patternBox_->selectedItem();
      std::map<ngrs::NCustomItem*, SinglePattern*>::iterator itr = patternMap.find(item);

      if(itr!=patternMap.end())
      {
        ngrs::NTreeNode* node = patternBox_->selectedTreeNode();
        SinglePattern* pattern = itr->second;
        SinglePattern* clonedPat = pattern->category()->clonePattern( *pattern, pattern->name()+"clone" );
        PatternItem* item = new PatternItem( clonedPat, clonedPat->name() );
        item->mouseDoublePress.connect(this,&SequencerBar::onPatternItemDblClick);
        node->addEntry(item);
        patternMap[item] = clonedPat;
        patternBox_->setSelectedItem( node, item );
        patternBox_->resize();
        patternBox_->repaint();
      }

    }

    void SequencerBar::onDeletePattern( ngrs::NButtonEvent* ev ) {
      ngrs::NCustomItem* item = patternBox_->selectedItem();
      std::map<ngrs::NCustomItem*, SinglePattern*>::iterator itr = patternMap.find(item);

      if( itr!=patternMap.end() )
      {
        SinglePattern* pattern = itr->second;
        patternMap.erase(itr);
        patternBox_->removeItem( item );
        patternBox_->resize();
        patternBox_->repaint();

        seqGui->removePattern(pattern);
        seqGui->repaint();

        if ( patView ) {
          if ( patView->pattern() == pattern ) {
            patView->setPattern( 0 );
            patView->repaint();
          }
        }

        seqGui->patternSequence()->removeSinglePattern(pattern);
      }
    }

    void SequencerBar::selectNextPattern() {
      ngrs::NCustomItem* item = patternBox_->selectedItem();
      std::map< ngrs::NCustomItem*, SinglePattern*>::iterator itr = patternMap.find( item );

      if ( itr != patternMap.end() ) {
        itr++;
        if (itr != patternMap.end() ) {
          // todo under construction
        }
      }
    }

    void SequencerBar::selectPrevPattern() {
      ngrs::NCustomItem* item = patternBox_->selectedItem();
      std::map<ngrs::NCustomItem*, SinglePattern*>::iterator itr = patternMap.find(item);

      if ( itr != patternMap.end() && itr != patternMap.begin() ) {
        itr--;
        // todo under construction
      }
    }


    void SequencerBar::onItemSelected( ngrs::NItemEvent* ev )
    {
      ngrs::NCustomItem* item = patternBox_->selectedItem();
      switchPatternViewPattern( item );
    }

    void SequencerBar::onPatternItemDblClick( ngrs::NButtonEvent* ev )
    {
      if (ev->button() == 1) { // if left double-click
        ngrs::NCustomItem* item = patternBox_->selectedItem();
        childView_->showPatternView();
        switchPatternViewPattern(item);
      }
    }

    void SequencerBar::switchPatternViewPattern( ngrs::NCustomItem* item )
    {
      std::vector<CategoryItem*>::iterator it = find(catItems.begin(),catItems.end(),item);
      if ( it != catItems.end() )  propertyBox_->setCategoryItem(*it);

      if (item) propertyBox_->setName( item->text() );
      std::map<ngrs::NCustomItem*, SinglePattern*>::iterator itr = patternMap.find(item);
      if(itr!=patternMap.end()) {
        patView->setPattern(itr->second);
        patView->repaint();
      }
    }

    void SequencerBar::onPatternAdd( ngrs::NButtonEvent * ev )
    {
      ngrs::NCustomItem* item = patternBox_->selectedItem();
      if (item) {
        std::map<ngrs::NCustomItem*, SinglePattern*>::iterator itr = patternMap.find(item);
        if(itr!=patternMap.end())
          seqGui->addPattern(itr->second);
      }
    }

    void SequencerBar::onNameChanged( const std::string& name )
    {
      ngrs::NCustomItem* item = patternBox_->selectedItem();
      item->setText(name);
      patternBox_->repaint();

      std::map<ngrs::NCustomItem*, SinglePattern*>::iterator itr = patternMap.find(item);
      if(itr!=patternMap.end()) {
        std::vector<SequencerItem*> list = seqGui->guiItemsByPattern(itr->second);
        std::vector<SequencerItem*>::iterator it = list.begin();
        for ( ; it < list.end(); it++) {
          SequencerItem* guiItem = *it;
          guiItem->repaint();
        }
      }
    }

    void SequencerBar::setEntry( ngrs::NObject* obj )
    {
      /*entryBox_->setControlObject(obj);
      entryBox_->resize();
      repaint();*/
    }

    void SequencerBar::setChildView( ChildView* view )
    {
      childView_ = view;
    }

  }
}
