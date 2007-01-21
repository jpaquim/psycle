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
#include <ngrs/scrollbox.h>

#include <algorithm>


namespace psy { 
  namespace host {


    SongExplorer::SongExplorer()
      : ngrs::Panel()
    {
      init();
    }


    SongExplorer::~SongExplorer()
    {
    }


    void SongExplorer::setSkin( )
    {  
    }

    void SongExplorer::init( )
    {  
      ngrs::FrameBorder frBorder;
      frBorder.setOval();
      setBorder( frBorder );

      setLayout( ngrs::AlignLayout() );
      setWidth( 90 );

      songTreeView_ = new ngrs::CustomTreeView();
      songTreeView_->setPreferredSize( 200, 100 );
      add( songTreeView_, ngrs::nAlClient );
    }

    void SongExplorer::addSong( psy::core::Song& song ) {
      DefaultBitmaps & icons = SkinReader::Instance()->bitmaps();
      ngrs::TreeNode* node = new ngrs::TreeNode();
        node->setExpanded(true);
        node->setHeader( new ngrs::Item( song.info().name() ));
       songTreeView_->addNode(node);

       node = new ngrs::TreeNode();
        node->setExpanded(true);
        node->setHeader( new ngrs::Item( song.info().name() ));
       songTreeView_->addNode(node);

       node = new ngrs::TreeNode();
        node->setExpanded(true);
        node->setHeader( new ngrs::Item( song.info().name() ));
       songTreeView_->addNode(node);
    }


  }
}
