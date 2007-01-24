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
#ifndef TREENODE_H
#define TREENODE_H

/**
@author  Stefan Nattkemper
*/

#include "object.h"
#include <vector>

namespace ngrs {

  class PopupMenu;

  class TreeNode {
  public:

    TreeNode();
    TreeNode( const std::string& userText, Object* userObject );
    ~TreeNode();

    signal1<TreeNode*> clicked;
    signal1<TreeNode*> dblClick;

    std::vector<TreeNode*>::iterator begin();
    std::vector<TreeNode*>::iterator end();
    std::vector<TreeNode*>::const_iterator begin() const;
    std::vector<TreeNode*>::const_iterator end() const;
    std::vector<TreeNode*>::const_reverse_iterator rbegin() const;
    std::vector<TreeNode*>::const_reverse_iterator rend() const;

    void insert( std::vector<TreeNode*>::iterator itr, TreeNode* node );
    void add( TreeNode* node );
    void setPopupMenu( PopupMenu* popupMenu );
    
    PopupMenu* popupMenu();

    bool leaf() const;
    TreeNode* parent() const;
    void setParent( TreeNode* parent );

    Object* userObject();
    void setUserObject( Object* object );

    std::string userText() const;

  private:

    std::string userText_;
    Object* userObject_;
    PopupMenu* popupMenu_;

    std::vector<TreeNode*> children;
    TreeNode* parent_;

  };

}

#endif
