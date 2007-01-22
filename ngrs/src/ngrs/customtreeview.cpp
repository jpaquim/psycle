/***************************************************************************
 *   Copyright (C) 2005, 2006, 2007 by  Stefan Nattkemper   *
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
#include "label.h"
#include "popupmenu.h"

namespace ngrs {

  CustomTreeView::CustomTreeView()
    : Panel(), rootNode_( 0 ), selectedTreeNodeGui_(0)
  {   
    init();
  }

  CustomTreeView::CustomTreeView( TreeNode* rootNode )
    : Panel(), rootNode_( rootNode ),  selectedTreeNodeGui_(0)
  {
    init();
    updateTree();
  }

  CustomTreeView::~CustomTreeView() {
  }

  void CustomTreeView::init() {
    setLayout( AlignLayout() );
    scrollBox_ = new ScrollBox();
    scrollArea_ = new Panel();
    scrollArea_->setLayout( AlignLayout() );
    scrollArea_->setClientSizePolicy(nVertical + nHorizontal);
    scrollArea_->setBackground(Color(255,255,255));
    scrollArea_->setTransparent(false);
    scrollBox_->setScrollPane(scrollArea_);
    add( scrollBox_, nAlClient );

    nodeSkinSelected_ = App::config()->skin("lbitemsel");
    nodeSkinNone_ = App::config()->skin("lbitemnone");
  }

  void CustomTreeView::buildTree( TreeNode* node ) {
    if ( !node) return;
    TreeNodeGui* leaf = new TreeNodeGui( node );
    leaf->setEvents( true );
    leaf->mousePress.connect( this, &CustomTreeView::onNodeMousePress );
    scrollArea_->add( leaf, nAlTop );
    std::vector<TreeNode*>::iterator it = node->begin();
    for ( ; it != node->end(); it++ ) {
      if ( *it ) {
        buildTree( *it );
      }
    }
  }

  void CustomTreeView::updateTree() {
    scrollArea_->removeChilds();
    selectedTreeNodeGui_ = 0;
    if ( rootNode_ ) {
      buildTree( rootNode_ );      
    }
  }

  void CustomTreeView::onNodeMousePress( ButtonEvent* ev ) {
    // dynamic_cast<TreeNodeGui*>(), c cast for rtti disabled compilers
    TreeNodeGui* gui = (TreeNodeGui*) ( ev->sender() );
    if ( gui != selectedTreeNodeGui_ ) {
      if ( selectedTreeNodeGui_ ) {
        selectedTreeNodeGui_->setSkin( nodeSkinNone_ ) ;
        selectedTreeNodeGui_->repaint();
      }
      selectedTreeNodeGui_ = gui;
      selectedTreeNodeGui_->setSkin( nodeSkinSelected_ );
      selectedTreeNodeGui_->repaint();
    }
    if ( ev->button() == 3 && gui->node()->popupMenu() ) {
      PopupMenu* popupMenu_ = gui->node()->popupMenu();
      popupMenu_->setPosition( ev->x() + absoluteLeft() + window()->left(), ev->y() + absoluteTop() + window()->top(),100,100);
      popupMenu_->setVisible(true);
    }
  }

  TreeNode* CustomTreeView::selectedTreeNode() {
    if ( selectedTreeNodeGui_ )
      return selectedTreeNodeGui_->node();
    else
      return 0;
  }


}
