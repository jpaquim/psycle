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
#include "popupmenu.h"

namespace ngrs {

  TreeNode::TreeNode() 
    : userObject_(0), popupMenu_(0), parent_(0)
  {
  }

  TreeNode::TreeNode( const std::string& userText, Object* userObject ) 
    : userText_(userText), userObject_(userObject), popupMenu_(0), parent_(0)
  {
  }
  
  TreeNode::~TreeNode() 
  {
    for ( std::vector<TreeNode*>::iterator it = begin(); it < end(); it++ ) {      
      delete *it;
    }
    children.clear();
  }

  std::vector<TreeNode*>::iterator TreeNode::begin() {
    return children.begin();
  }

  std::vector<TreeNode*>::iterator TreeNode::end()  {
    return children.end();
  }

  std::vector<TreeNode*>::const_iterator TreeNode::begin() const{
    return children.begin();
  }

  std::vector<TreeNode*>::const_iterator TreeNode::end() const {
    return children.end();
  }

  std::vector<TreeNode*>::const_reverse_iterator TreeNode::rbegin() const {
    return children.rbegin();
  }

  std::vector<TreeNode*>::const_reverse_iterator TreeNode::rend() const {
    return children.rend();
  }

  bool TreeNode::leaf() const {
    return children.size() == 0;
  }

  TreeNode* TreeNode::parent() const {
    return parent_;
  }

  void TreeNode::setParent( TreeNode* parent ) {
    parent_ = parent;
  }

  Object* TreeNode::userObject() {
    return userObject_;
  }

  void TreeNode::setUserObject( Object* object ) {
    userObject_ = object;
  }

  std::string TreeNode::userText() const {
    return userText_;
  }

  void TreeNode::insert( std::vector<TreeNode*>::iterator itr, TreeNode* node ) {
    children.insert( itr, node );
    node->setParent( this );
  }

  void TreeNode::add( TreeNode* node ) {
    children.push_back( node );
    node->setParent( this );
  }

  void TreeNode::setPopupMenu( PopupMenu* popupMenu ) {
    popupMenu_ = popupMenu;
  }

  PopupMenu* TreeNode::popupMenu() {
    return popupMenu_;
  }

}
