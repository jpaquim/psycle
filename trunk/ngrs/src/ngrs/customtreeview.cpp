/***************************************************************************
 *   Copyright (C) 2005, 2006, 2007 by Stefan Nattkemper   *
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
#include "customtreeview.h"
#include "treenode.h"
#include "alignlayout.h"
#include "autoscrolllayout.h"
#include "scrollbox.h"
#include "customitem.h"
#include "app.h"
#include "config.h"
#include "window.h"

namespace ngrs {

  CustomTreeView::CustomTreeView()
    : Panel()
  {
    setLayout( AlignLayout() );

    scrollBox_ = new NScrollBox();
    scrollArea_ = new Panel();
    scrollArea_->setLayout( AlignLayout() );
    scrollArea_->setClientSizePolicy(nVertical + nHorizontal);
    scrollArea_->setBackground(Color(255,255,255));
    scrollArea_->setTransparent(false);
    scrollBox_->setScrollPane(scrollArea_);
    add(scrollBox_, nAlClient);

    selectedItem_ = 0;
    selectedTreeNode_ = 0;

    itemBg = App::config()->skin("lbitemsel");
    itemFg = App::config()->skin("lbitemnone");

  }


  CustomTreeView::~CustomTreeView()
  {
  }


  void CustomTreeView::addNode( TreeNode * node )
  {
    scrollArea_->add( node, nAlTop );
    node->itemSelected.connect(this,&CustomTreeView::onSelectedItem);
  }

  void CustomTreeView::onSelectedItem(TreeNode* node, NCustomItem * sender )
  {
    selectedTreeNode_ = node;

    sender->setSkin(itemBg);
    sender->repaint();


    if (selectedItem_ && sender!=selectedItem_ ) {
      selectedItem_->setSkin(itemFg);
      selectedItem_->repaint();
    }

    selectedItem_ = sender;

    ItemEvent ev(sender,sender->text());
    itemSelected.emit(&ev);
  }

  void CustomTreeView::setSelectedItem( TreeNode* node, NCustomItem* item ) {
    selectedTreeNode_ = node;

    item->setSkin(itemBg);
    item->repaint();


    if (selectedItem_ && item!=selectedItem_ ) {
      selectedItem_->setSkin(itemFg);
      selectedItem_->repaint();
    }

    selectedItem_ = item;

    ItemEvent ev(item,item->text());
    itemSelected.emit(&ev);          
  }     

  TreeNode * CustomTreeView::selectedTreeNode( )
  {
    return selectedTreeNode_;
  }

  NCustomItem * CustomTreeView::selectedItem( )
  {
    return selectedItem_;
  }

  void CustomTreeView::removeChilds( )
  {
    scrollArea_->removeChilds();
    selectedItem_ = 0;
    selectedTreeNode_ = 0;
  }

  void CustomTreeView::removeItem( NCustomItem * item )
  {
    if ( item && item->isChildOf( this ) ) {
      if ( item == selectedItem_ ) selectedItem_ = 0;
      item->erase();
      if ( window() ) window()->checkForRemove( item );
      App::addRemovePipe(item);
    }
  }

  void CustomTreeView::onItemDblClick( ButtonEvent* ev ) {
    //ItemEvent ev( sender, sender->text() );
    //itemDblClick.emit( ev );        
  }

}
