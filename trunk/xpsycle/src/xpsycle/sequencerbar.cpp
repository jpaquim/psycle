/***************************************************************************
*   Copyright (C) 2006 by  Stefan   *
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
#include "patternview.h"
#include "defaultbitmaps.h"
#include "sequencergui.h"
#include "skinreader.h"
#include "childview.h"
#include <psycore/song.h>
#include <ngrs/label.h>
#include <ngrs/item.h>
#include <ngrs/app.h>
#include <ngrs/config.h>
#include <ngrs/seg7display.h>
#include <ngrs/frameborder.h>
#include <ngrs/toolbar.h>
#include <ngrs/toolbarseparator.h>
#include <ngrs/gridlayout.h>
#include <ngrs/treenode.h>
#include <ngrs/objectinspector.h>
#include <ngrs/property.h>

#include <algorithm>


namespace psy { 
  namespace host {


    CategoryTreeNode::CategoryTreeNode( psy::core::PatternCategory * cat )
    {
      cat_ = cat;
    }

    CategoryTreeNode::~ CategoryTreeNode( )
    {
    }

    psy::core::PatternCategory * CategoryTreeNode::category( )
    {
      return cat_;
    }



    CategoryItem::CategoryItem( psy::core::PatternCategory* category , const std::string& text )
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
      setLayout( ngrs::AlignLayout(5,0) );
      label_ = new ngrs::Label();
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

    void CategoryItem::paint( ngrs::Graphics& g )
    {
      g.setForeground( ngrs::Color(category_->color()) );
      g.drawRect(0,0,clientWidth()-1, clientHeight()-1);

      int cw = clientWidth();

      ngrs::NPoint pts[3];
      pts[0].setX( cw-6 );
      pts[0].setY( 0 );
      pts[1].setX( cw-1 );
      pts[1].setY( 0 );
      pts[2].setX( cw-1 );
      pts[2].setY( 6 );

      g.fillPolygon(pts,3);
    }

    psy::core::PatternCategory * CategoryItem::category( )
    {
      return category_;
    }



    PatternItem::PatternItem( psy::core::SinglePattern* pattern,  const std::string& text )
      : ngrs::Item(text)
    {
      pattern_ = pattern;
    }

    PatternItem::~ PatternItem( )
    {
    }

    void PatternItem::setText( const std::string & text )
    {
      pattern_->setName(text);
      ngrs::Item::setText(text);
    }

    SequencerBar::SequencerBar()
      : ngrs::Panel()
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
      DefaultBitmaps & icons = SkinReader::Instance()->bitmaps();

      counter = 0;
      seqGui = 0;
      patView = 0;

      ngrs::FrameBorder frBorder;
      frBorder.setOval();
      frBorder.setLineCount( 2, 4, 4 );
      setBorder( frBorder );

      setLayout( ngrs::AlignLayout() );
      setWidth( 90 );

      setSkin();

      ngrs::NFlipBox* flipBox = new ngrs::NFlipBox();
      ngrs::FrameBorder fr;
      flipBox->setBorder(fr);
      flipBox->header()->add(new ngrs::Label("Pattern box"), ngrs::nAlClient);

      ngrs::Panel* patternPanel = new ngrs::Panel();
      patternPanel->setLayout( ngrs::AlignLayout() );

      ngrs::ToolBar* patToolBar = new ngrs::ToolBar();
      ngrs::Image* img = new ngrs::Image();
      img->setSharedBitmap(&icons.new_category());
      img->setPreferredSize(25,25);
      ngrs::Button* newCatBtn = new ngrs::Button(img);
      newCatBtn->setHint("New Category");
      patToolBar->add( newCatBtn )->clicked.connect(this,&SequencerBar::onNewCategory);
      patToolBar->add(new ngrs::ToolBarSeparator());
      img = new ngrs::Image();
      img->setSharedBitmap( &icons.pattern_new() );
      ngrs::Button* newPatBtn = new ngrs::Button( img );
      newPatBtn->setHint("New Pattern");
      patToolBar->add( newPatBtn )->clicked.connect( this, &SequencerBar::onNewPattern );
      img = new ngrs::Image();
      img->setSharedBitmap( &icons.delPattern() );
      img->setPreferredSize(25,25);
      ngrs::Button* delPatBtn = new ngrs::Button( img );
      delPatBtn->setHint( "Delete Pattern" );
      patToolBar->add( delPatBtn )->clicked.connect( this, &SequencerBar::onDeletePattern );
      ngrs::Button* clnPatBtn = new ngrs::Button("Cln");
      clnPatBtn->setHint("Clone Pattern");
      patToolBar->add( clnPatBtn )->clicked.connect(this, &SequencerBar::onClonePattern );

      ngrs::Button* addPatBtn = new ngrs::Button("Add");
      addPatBtn->setHint("Add Pattern To Sequencer");
      patToolBar->add( addPatBtn )->clicked.connect(this,&SequencerBar::onPatternAdd);
      patternPanel->add(patToolBar, ngrs::nAlTop);

      patternBox_ = new ngrs::CustomTreeView();
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
      entryBox_ = new ngrs::ObjectInspector();
      entryBox_->setPreferredSize(100,100);
      flipBox->setBorder(fr);
      flipBox->header()->add(new ngrs::Label("Sequence Properties"), ngrs::nAlClient);

      flipBox->pane()->add(entryBox_, ngrs::nAlClient);

      add(flipBox, ngrs::nAlTop);



      ///\ todo remove this somewhere else and rename this class to patternBox

      /*
      ngrs::Panel* checkPanel = new ngrs::Panel();
      //checkPanel->skin_.setTranslucent(Color(200,200,200),70);
      checkPanel->setLayout(ListLayout());
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
      std::vector<psy::core::PatternCategory*>::iterator it = seqGui->patternSequence()->patternData()->begin();
      for ( ; it < seqGui->patternSequence()->patternData()->end(); ++it) {
        psy::core::PatternCategory* category = *it;
        CategoryTreeNode* node = new CategoryTreeNode(category);
        node->setExpanded(true);
        categoryMap[node]=category;
        CategoryItem* catItem = new CategoryItem(category,"Category");
        catItems.push_back(catItem);
        node->setHeader(catItem);
        patternBox_->addNode(node);
        std::vector<psy::core::SinglePattern*>::iterator patIt = category->begin();
        for ( ; patIt < category->end(); patIt++) {
          psy::core::SinglePattern* pattern = *patIt;
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

    void SequencerBar::onMoveCursorPaste( ngrs::ButtonEvent * ev )
    {
      //patternView_->setMoveCursorWhenPaste(movecursorpaste_->checked() );
    }


    void SequencerBar::onRecordTweakChange( ngrs::ButtonEvent * ev )
    {
      // Global::configuration()._RecordTweaks = record_tweaks_->checked();
    }

    void SequencerBar::onNewCategory( ngrs::ButtonEvent * ev )
    {
      psy::core::PatternCategory* category = seqGui->patternSequence()-> patternData()->createNewCategory("category");
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

    void SequencerBar::onNewPattern( ngrs::ButtonEvent * ev )
    {
      if (patternBox_->selectedTreeNode() ) {
        ngrs::TreeNode* node = patternBox_->selectedTreeNode();

        std::map<ngrs::TreeNode*, psy::core::PatternCategory*>::iterator itr = categoryMap.find(node);
        if(itr != categoryMap.end()) {
          psy::core::PatternCategory* cat = itr->second;
          psy::core::SinglePattern* pattern = cat->createNewPattern("Pattern");
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

    void SequencerBar::onClonePattern( ngrs::ButtonEvent * ev )
    {
      ngrs::CustomItem* item = patternBox_->selectedItem();
      std::map<ngrs::CustomItem*, psy::core::SinglePattern*>::iterator itr = patternMap.find(item);

      if(itr!=patternMap.end())
      {
        ngrs::TreeNode* node = patternBox_->selectedTreeNode();
        psy::core::SinglePattern* pattern = itr->second;
        psy::core::SinglePattern* clonedPat = pattern->category()->clonePattern( *pattern, pattern->name()+"clone" );
        PatternItem* item = new PatternItem( clonedPat, clonedPat->name() );
        item->mouseDoublePress.connect(this,&SequencerBar::onPatternItemDblClick);
        node->addEntry(item);
        patternMap[item] = clonedPat;
        patternBox_->setSelectedItem( node, item );
        patternBox_->resize();
        patternBox_->repaint();
      }

    }

    void SequencerBar::onDeletePattern( ngrs::ButtonEvent* ev ) {
      ngrs::CustomItem* item = patternBox_->selectedItem();
      std::map<ngrs::CustomItem*, psy::core::SinglePattern*>::iterator itr = patternMap.find(item);

      if( itr!=patternMap.end() )
      {
        psy::core::SinglePattern* pattern = itr->second;
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
      ngrs::CustomItem* item = patternBox_->selectedItem();
      std::map< ngrs::CustomItem*, psy::core::SinglePattern*>::iterator itr = patternMap.find( item );

      if ( itr != patternMap.end() ) {
        itr++;
        if (itr != patternMap.end() ) {
          // todo under construction
        }
      }
    }

    void SequencerBar::selectPrevPattern() {
      ngrs::CustomItem* item = patternBox_->selectedItem();
      std::map<ngrs::CustomItem*, psy::core::SinglePattern*>::iterator itr = patternMap.find(item);

      if ( itr != patternMap.end() && itr != patternMap.begin() ) {
        itr--;
        // todo under construction
      }
    }


    void SequencerBar::onItemSelected( ngrs::ItemEvent* ev )
    {
      ngrs::CustomItem* item = patternBox_->selectedItem();
      switchPatternViewPattern( item );
    }

    void SequencerBar::onPatternItemDblClick( ngrs::ButtonEvent* ev )
    {
      if (ev->button() == 1) { // if left double-click
        ngrs::CustomItem* item = patternBox_->selectedItem();
        childView_->showPatternView();
        switchPatternViewPattern(item);
      }
    }

    void SequencerBar::switchPatternViewPattern( ngrs::CustomItem* item )
    {
      std::vector<CategoryItem*>::iterator it = find(catItems.begin(),catItems.end(),item);
      if ( it != catItems.end() )  propertyBox_->setCategoryItem(*it);

      if (item) propertyBox_->setName( item->text() );
      std::map<ngrs::CustomItem*, psy::core::SinglePattern*>::iterator itr = patternMap.find(item);
      if(itr!=patternMap.end()) {
        patView->setPattern(itr->second);
        patView->repaint();
      }
    }

    void SequencerBar::onPatternAdd( ngrs::ButtonEvent * ev )
    {
      ngrs::CustomItem* item = patternBox_->selectedItem();
      if (item) {
        std::map<ngrs::CustomItem*, psy::core::SinglePattern*>::iterator itr = patternMap.find(item);
        if(itr!=patternMap.end())
          seqGui->addPattern(itr->second);
      }
    }

    void SequencerBar::onNameChanged( const std::string& name )
    {
      ngrs::CustomItem* item = patternBox_->selectedItem();
      item->setText(name);
      patternBox_->repaint();

      std::map<ngrs::CustomItem*, psy::core::SinglePattern*>::iterator itr = patternMap.find(item);
      if(itr!=patternMap.end()) {
        std::vector<SequencerItem*> list = seqGui->guiItemsByPattern(itr->second);
        std::vector<SequencerItem*>::iterator it = list.begin();
        for ( ; it < list.end(); it++) {
          SequencerItem* guiItem = *it;
          guiItem->repaint();
        }
      }
    }

    void SequencerBar::setEntry( ngrs::Object* obj )
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
