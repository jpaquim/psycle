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
#ifndef NCUSTOMTREEVIEW_H
#define NCUSTOMTREEVIEW_H

#include "panel.h"
#include "itemevent.h"

/**
@author Stefan Nattkemper
*/

namespace ngrs {

  class TreeNode;
  class NScrollBox;
  class NCustomItem;


  class CustomTreeView : public Panel
  {
  public:
    CustomTreeView();

    ~CustomTreeView();

    void addNode( TreeNode* node);

    signal1<ItemEvent*> itemSelected;
    signal1<ItemEvent*> itemDblClick;

    TreeNode* selectedTreeNode();
    NCustomItem* selectedItem();

    virtual void removeChilds();
    void removeItem( NCustomItem* item );

    void setSelectedItem( TreeNode* node, NCustomItem* item );

  private:

    NScrollBox* scrollBox_;
    Panel* scrollArea_;
    NCustomItem* selectedItem_;
    TreeNode* selectedTreeNode_;

    Skin itemBg;
    Skin itemFg;

    void onSelectedItem(TreeNode* node, NCustomItem* sender);
    void onItemDblClick( ButtonEvent* ev );

  };

}

#endif
