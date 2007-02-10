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

  class NTreeNode;
  class ScrollBox;
  class CustomItem;


  class NCustomTreeView : public Panel
  {
  public:
    NCustomTreeView();

    ~NCustomTreeView();

    void addNode( NTreeNode* node);

    signal1<ItemEvent*> itemSelected;
    signal1<ItemEvent*> itemDblClick;

    NTreeNode* selectedTreeNode();
    CustomItem* selectedItem();

    virtual void removeChilds();
    void removeItem( CustomItem* item );

    void setSelectedItem( NTreeNode* node, CustomItem* item );

  private:

    ScrollBox* scrollBox_;
    Panel* scrollArea_;
    CustomItem* selectedItem_;
    NTreeNode* selectedTreeNode_;

    Skin itemBg;
    Skin itemFg;

    void onSelectedItem(NTreeNode* node, CustomItem* sender);
    void onItemDblClick( ButtonEvent* ev );

  };

}

#endif
