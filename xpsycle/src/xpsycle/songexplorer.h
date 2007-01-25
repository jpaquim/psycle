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
#ifndef SONGEXPLORER_H
#define SONGEXPLORER_H

#include "projectdata.h"
#include "module.h"
#include <psycore/patternsequence.h>
#include <ngrs/dockpanel.h>
#include <ngrs/item.h>
#include <ngrs/treenode.h>
#include <ngrs/customtreeview.h>
#include <ngrs/popupmenu.h>
#include <ngrs/menuitem.h>
#include <ngrs/tabbook.h>
#include <map>

/**
@author  Stefan
*/

namespace psy { 
  namespace core {
    class Song;
    class TabBook;
  }
  namespace host {

    class SongExplorer : public ngrs::DockPanel
    {
    public:
      SongExplorer( ngrs::TabBook& book );

      ~SongExplorer();

      psy::core::Song* newSong();

    private:
     
      ngrs::TabBook& book_;
      ProjectData projects;
      std::vector<Module*> modules_;

      ngrs::CustomTreeView* songTreeView_;
      ngrs::TreeNode* topNode;
      ngrs::PopupMenu* patternPopupMenu_;
      
      void init();
      void initPatternPopupMenu();

      ngrs::TreeNode* createPatternNodes( const psy::core::PatternData& patterns );
      ngrs::TreeNode* createMachineNodes();
      ngrs::TreeNode* createSampleNodes();
      ngrs::TreeNode* createSequencerNodes();

      void refreshBook();
      
      void onPatternNewClick( ngrs::ButtonEvent* ev );
      void onTreeNodeClick( ngrs::TreeNode& node );

    };

  }
}
#endif
