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
#include "image.h"
#include "popupmenu.h"
#include "listlayout.h"
#include <algorithm>

#ifdef _MSC_VER
#undef min 
#undef max
#endif


namespace ngrs {


  // flipper

  /* XPM */
const char * node_expanded_xpm[] = {
"11 11 2 1",
" 	c #404040",
".	c #FFFFFF",
"           ",
" ......... ",
" ......... ",
" ......... ",
" ......... ",
" ..     .. ",
" ......... ",
" ......... ",
" ......... ",
" ......... ",
"           "};
/* XPM */
const char * node_expand_xpm[] = {
"11 11 2 1",
" 	c #404040",
".	c #FFFFFF",
"           ",
" ......... ",
" ......... ",
" .... .... ",
" .... .... ",
" ..     .. ",
" .... .... ",
" .... .... ",
" ......... ",
" ......... ",
"           "};

  TreeNodeGui::FlipperImage::FlipperImage() {
  }

  TreeNodeGui::FlipperImage::~FlipperImage() {
  }

  void TreeNodeGui::FlipperImage::paint( Graphics& g ) {
    Pen pen;
    pen.setLineStyle( nLineOnOffDash );
    g.setPen( pen );
    int ch = clientHeight();
    int cw = clientWidth();
    g.drawLine( 0, ch/2, cw, ch/2 );
    Image::paint(g);
  }

  int TreeNodeGui::FlipperImage::preferredWidth() const {
    return static_cast<int>( 1.5 * Image::preferredWidth() );
  }

  TreeNodeGui::TreeNodeGui( CustomTreeView& treeView, TreeNode* node )
   : Panel(), treeView_(treeView), node_( node )
  {
    expanded_ = true;
    expandBmp.createFromXpmData(node_expand_xpm);
    expandedBmp.createFromXpmData(node_expanded_xpm);
    // the left flipper, node line image
    img_ = new FlipperImage();
    img_->setVAlign(nAlCenter);
    img_->setHAlign(nAlLeft);
    img_->setSharedBitmap(&expandedBmp);
    img_->mousePress.connect( this, &TreeNodeGui::onImgClick );
    img_->setEvents( true );
    img_->setVisible( false );
    Panel::add( img_ );
    // the label;
    label_ = new Label( node->userText() );    
    Panel::add( label_ );
    // panel for children
    children_ = new Panel();
    children_->setLayout( ListLayout() );
    children_->setSpacing( Size( img_->preferredWidth(),0,0,0));
    Panel::add( children_ );
  }

  TreeNodeGui::~TreeNodeGui() {
  }

  TreeNode* TreeNodeGui::node() { 
    return node_; 
  }

  void TreeNodeGui::add( TreeNodeGui* nodeGui ) {
    children_->add( nodeGui,nAlTop );
    img_->setVisible( true );
  }

  void TreeNodeGui::resize() {
    img_->setPosition(0,0, img_->preferredWidth(), label_->preferredHeight() );
    label_->setPosition( img_->preferredWidth(), 0, clientWidth() - img_->preferredWidth(), label_->preferredHeight() );
    children_->setPosition( 0, label_->preferredHeight(), clientWidth(), clientHeight() - label_->preferredHeight() );
  }

  int TreeNodeGui::preferredWidth() const {
    return std::max( img_->preferredWidth() + label_->preferredWidth(), children_->preferredWidth() );
  }

  int TreeNodeGui::preferredHeight() const {    
    return label_->preferredHeight() +
      ( (!node_->leaf() && expanded_)  ? children_->preferredHeight() : 0 );
  }

  void TreeNodeGui::setSkin( const Skin& skin ) {
    label_->setSkin( skin );
  }

  void TreeNodeGui::onImgClick( ButtonEvent* ev ) {
    expanded_ = !expanded_;
    if ( expanded_ ) {
      img_->setSharedBitmap(&expandedBmp);
      children_->setVisible( true );
    } else {
      children_->setVisible( false );
      img_->setSharedBitmap(&expandBmp);
    }
    treeView_.resize();
    treeView_.repaint();
  }  


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

  void CustomTreeView::buildTree( TreeNode* node, TreeNodeGui* nodeGui ) {
    if ( !node) return;
    TreeNodeGui* leaf = new TreeNodeGui( *this, node );
    leaf->setEvents( true );
    leaf->mousePress.connect( this, &CustomTreeView::onNodeMousePress );
    leaf->mouseDoublePress.connect( this, &CustomTreeView::onNodeDblClick );
    if ( !nodeGui )
      scrollArea_->add( leaf, nAlTop );
    else
      nodeGui->add( leaf );
    std::vector<TreeNode*>::iterator it = node->begin();
    for ( ; it != node->end(); it++ ) {
      if ( *it ) {
         buildTree( *it, leaf );
      }
    }
  }

  void CustomTreeView::updateTree() {
    scrollArea_->removeChilds();
    selectedTreeNodeGui_ = 0;
    if ( rootNode_ ) {
      buildTree( rootNode_, 0 );      
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
      nodeClicked.emit( gui->node() );
      gui->node()->clicked.emit( gui->node() );
    }
    if ( ev->button() == 3 && gui->node()->popupMenu() ) {
      PopupMenu* popupMenu_ = gui->node()->popupMenu();
      Point mouse_pos = App::system().mousePosition();
      popupMenu_->setPosition( mouse_pos.x(), mouse_pos.y(), 100,100);
      popupMenu_->setVisible(true);
    }
  }

  void CustomTreeView::onNodeDblClick( ButtonEvent* ev ) {
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
    gui->node()->dblClick.emit( gui->node() );
  }

  TreeNode* CustomTreeView::selectedTreeNode() {
    if ( selectedTreeNodeGui_ )
      return selectedTreeNodeGui_->node();
    else
      return 0;
  }

  


}
