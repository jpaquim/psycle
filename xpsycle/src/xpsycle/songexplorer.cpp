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
#include "songexplorer.h"
#include "patternview.h"
#include "machineview.h"
#include "sequencergui.h"
#include "defaultbitmaps.h"
#include "sequencergui.h"
#include "skinreader.h"
#include "childview.h"
#include "patternview.h"
#include <psycore/song.h>
#include <psycore/patterndata.h>
#include <psycore/patternsequence.h>
#include <ngrs/file.h>
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
#include <ngrs/scrollbox.h>
#include <ngrs/tabbook.h>
#include <algorithm>


namespace psy { 
  namespace host {


    SongExplorer::SongExplorer( ngrs::TabBook& book )
      : ngrs::DockPanel(), book_(book)
    {
      init();
    }


    SongExplorer::~SongExplorer()
    {
    }
   
    void SongExplorer::init( )
    {  
      ngrs::FrameBorder frBorder;
      frBorder.setOval();
      setBorder( frBorder );

      topNode = new ngrs::TreeNode( "Projects (0 songs)", 0 );
      songTreeView_ = new ngrs::CustomTreeView( topNode );
      setPreferredSize( 200, 100 );
      pane()->add( songTreeView_, ngrs::nAlClient );

      initPatternPopupMenu();
    }

    psy::core::Song* SongExplorer::newSong() {
      psy::core::Song* song = projects.createSong();
      ngrs::TreeNode* songNode = new ngrs::TreeNode( song->info().name(),0);

      ngrs::TreeNode* patternNode = createPatternNodes( song->patternSequence().patternData() );
      songMap[patternNode] = song;
      songNode->add( patternNode );

      ngrs::TreeNode* machineNode = createMachineNodes();
      songMap[machineNode] = song;
      songNode->add( machineNode );

      ngrs::TreeNode* sampleNode = createSampleNodes();
      songMap[sampleNode] = song;
      songNode->add( sampleNode );

      ngrs::TreeNode* sequencerNode = createSequencerNodes();
      songMap[sequencerNode] = song;
      songNode->add( sequencerNode );
      
      topNode->add( songNode );
      songTreeView_->updateTree();

      return song;
    }

    ngrs::TreeNode* SongExplorer::createPatternNodes( const psy::core::PatternData& patterns ) {
      ngrs::TreeNode* patternNode = new ngrs::TreeNode("Patterns",0);
      patternNode->setPopupMenu( patternPopupMenu_ );

      std::list<psy::core::SinglePattern>::const_iterator it = patterns.begin();
      for ( ; it != patterns.end(); it++ ) {
        const psy::core::SinglePattern& pattern = *it;        
        std::string patternName = ngrs::File::extractFileNameFromPath( pattern.name(), true );
        ngrs::TreeNode* leaf = new ngrs::TreeNode( patternName, 0 );
        patternNode->add( leaf );
      }
      return patternNode;
    }

    ngrs::TreeNode* SongExplorer::createMachineNodes() {
      ngrs::TreeNode* machineNode = new ngrs::TreeNode("Machines",0);
      machineNode->dblClick.connect( this, &SongExplorer::onMachineBranchNodeClicked );
      return machineNode;
    }

    ngrs::TreeNode* SongExplorer::createSequencerNodes() {
      ngrs::TreeNode* sequencerNode = new ngrs::TreeNode("Sequencer",0);
      sequencerNode->dblClick.connect( this, &SongExplorer::onSequencerBranchNodeClicked );
      return sequencerNode;
    }

    ngrs::TreeNode* SongExplorer::createSampleNodes() {
      ngrs::TreeNode* sampleNode = new ngrs::TreeNode("Samples",0);
      return sampleNode;
    }

    void SongExplorer::initPatternPopupMenu() {
      patternPopupMenu_ = new ngrs::PopupMenu();
      add( patternPopupMenu_ );

      ngrs::MenuItem* item = new ngrs::MenuItem("New Pattern");
      item->click.connect( this, &SongExplorer::onAddPatternClicked);
      patternPopupMenu_->add( item );
    }

    void SongExplorer::onAddPatternClicked( ngrs::ButtonEvent* ev ) {
      ngrs::TreeNode* node = songTreeView_->selectedTreeNode();
      std::map<ngrs::TreeNode*, psy::core::Song*>::iterator it = songMap.find( node );
      if ( it != songMap.end() ) {
        psy::core::Song* song = it->second;
        psy::core::SinglePattern& pattern = song->patternSequence().patternData().newPattern();
        pattern.setName("/pattern");
        ngrs::TreeNode* leaf = new ngrs::TreeNode( "pattern", 0 );
        node->add( leaf );
        songTreeView_->updateTree();
        PatternView* view = new PatternView(song);
        view->setPattern(&pattern);
        book_.addPage( view, "pattern" );
        /// todo
        ngrs::VisualComponent* v = (ngrs::VisualComponent*) book_.parent();
        v->repaint();
        v->repaint();
        ///todo end
      }
    }    

    void SongExplorer::onMachineBranchNodeClicked( ngrs::TreeNode* node ) {
      std::map<ngrs::TreeNode*, psy::core::Song*>::iterator it = songMap.find( node );
      if ( it != songMap.end() ) {
        psy::core::Song* song = it->second;
        if ( !node->userObject() ) {
          MachineView* view = new MachineView(*song);
          node->setUserObject( view );
          book_.addPage( view, "MachineView" );
        } else {
          book_.setActivePage( (MachineView*) node->userObject() );
        }
        book_.resize();
        book_.repaint();
      }
    }

      void SongExplorer::onSequencerBranchNodeClicked( ngrs::TreeNode* node ) {
      std::map<ngrs::TreeNode*, psy::core::Song*>::iterator it = songMap.find( node );
      if ( it != songMap.end() ) {
        psy::core::Song* song = it->second;
        if ( !node->userObject() ) {
          SequencerGUI* view = new SequencerGUI();
          view->setPatternSequence( &song->patternSequence() );
          node->setUserObject( view );
          book_.addPage( view, "Sequencer" );
        } else {
          book_.setActivePage( (SequencerGUI*) node->userObject() );
        }
        book_.resize();
        book_.repaint();
      }
    }

  }
}
