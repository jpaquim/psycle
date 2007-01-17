/***************************************************************************
 *   Copyright (C) 2005, 2006, 2007 by  Stefan Nattkemper  *
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
#include "treenode.h"
#include "alignlayout.h"
#include "frameborder.h"
#include "label.h"
#include "customitem.h"

namespace ngrs {

  TreeNode::TreeNode()
    : NFlipBox()
  {
    entries_ = new Panel();
    entries_->setLayout( AlignLayout() );
    entries_->setSpacing(flipperWidth(),0,0,0);
    pane()->add(entries_, nAlTop);

    subNodes = new Panel();
    subNodes->setLayout( AlignLayout() );
    pane()->add(subNodes, nAlTop);

    headerItem_ = 0;
  }

  TreeNode::~TreeNode()
  {
  }

  void TreeNode::addNode( TreeNode * node )
  {
    subNodes->add(node, nAlTop);
  }

  void TreeNode::addEntry( CustomItem * entry )
  {
    entries_->add(entry, nAlTop);
    entry->mousePress.connect(this,&TreeNode::onItemPress);
    entry->setTransparent(true);
  }

  void TreeNode::setHeader( CustomItem * entry )
  {
    headerItem_ = entry;
    header()->add(entry, nAlClient);
    entry->mousePress.connect(this,&TreeNode::onItemPress);
    entry->setTransparent(true);
  }

  void TreeNode::onItemPress( ButtonEvent * ev )
  {
    if (ev->button() == 1) {
      CustomItem* item = static_cast<CustomItem*>( ev->sender() );
      itemSelected.emit(this, item);
    }
  }

  CustomItem * TreeNode::headerItem( )
  {
    return headerItem_;
  }

  void TreeNode::paint( Graphics& g ) {
    /*
    VisualComponent* last = dynamic_cast<VisualComponent*>( parent() )->visualComponents().back();
    if ( this != last )
    setStyle( nFlipBoxLine );
    else
    setStyle( 0 );
    */

    NFlipBox::paint( g );
  }

}
