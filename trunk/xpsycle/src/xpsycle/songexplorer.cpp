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
      songTreeView_->nodeDblClick.connect( this, &SongExplorer::onTreeNodeClick);
      setPreferredSize( 200, 100 );
      pane()->add( songTreeView_, ngrs::nAlClient );

      initPatternPopupMenu();
    }

    psy::core::Song* SongExplorer::newSong() {
      psy::core::Song* song = projects.createSong();
      ngrs::TreeNode* songNode = new ngrs::TreeNode( song->info().name(),0);

      ngrs::TreeNode* patternNode = createPatternNodes( song->patternSequence().patternData() );
      songNode->add( patternNode );

      ngrs::TreeNode* machineNode = createMachineNodes();
      songNode->add( machineNode );

      ngrs::TreeNode* sampleNode = createSampleNodes();
      songNode->add( sampleNode );

      ngrs::TreeNode* sequencerNode = createSequencerNodes();
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
      return machineNode;
    }

    ngrs::TreeNode* SongExplorer::createSequencerNodes() {
      ngrs::TreeNode* sequencerNode = new ngrs::TreeNode("Sequencer",0);
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
      item->click.connect( this, &SongExplorer::onPatternNewClick );
      patternPopupMenu_->add( item );
    }

    void SongExplorer::onTreeNodeClick( ngrs::TreeNode& node ) {
      // build tree path as string
      std::vector<std::string> path;
      path.push_back( node.userText() );
      ngrs::TreeNode* walkNode = &node;
      while ( walkNode = walkNode->parent() )
        path.insert( path.begin(), walkNode->userText() );

      if ( path.size() == 3 ) {
        psy::core::Song* song = projects.songByName( path.at(1) );
        if ( song ) {
          if ( path.at(2) == "Sequencer" ) {
            showSequencerView(*song);
          }
        }
      }
    }

    void SongExplorer::onPatternNewClick( ngrs::ButtonEvent* ev ) {
      ngrs::TreeNode* node = songTreeView_->selectedTreeNode();
      if ( node ) {
        // build tree path as string
        std::vector<std::string> path;
        path.push_back( node->userText() );
        while ( node = node->parent() ) 
          path.insert( path.begin(), node->userText() );

        if ( path.size() == 3 ) {
          psy::core::Song* song = projects.songByName( path.at(1) );
          if ( song ) {
            psy::core::SinglePattern& pattern = song->patternSequence().patternData().newPattern();
            PatternView* module = new PatternView( song );
            module->setPattern( &pattern );
            book_.addPage( module, pattern.name() );
            modules_.push_back( module );
          }
        }
      }
    }

    void SongExplorer::showSequencerView( psy::core::Song& song ) {
      bool found = false;
      std::vector<Module*>::iterator it = modules_.begin();
      for ( ; it != modules_.end(); it++ ) {
        Module* module = *it;
        if ( module->song() == &song ) {
          book_.setActivePage( module );
          found = true;
          break;
        }
      }
      if ( !found ) {
        SequencerGUI* view = new SequencerGUI();
        view->setSong( &song );
        book_.addPage( view, "Sequencer");
        modules_.push_back( view );
      }
    }

  }
}
