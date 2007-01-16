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
#ifndef LISTBOX_H
#define LISTBOX_H

#include "scrollbox.h"
#include "listlayout.h"
#include "customitem.h"
#include "itemevent.h"

/**
@author Stefan
*/

namespace ngrs {

  class ListBox : public ScrollBox
  {
  public:
    ListBox();

    ~ListBox();

    signal1<ItemEvent*> itemSelected;

    void add(CustomItem* item);
    void add(CustomItem* item, bool align);
    void insert(CustomItem* item, int index, bool align = true);

    virtual void onItemSelected(CustomItem * item);
    virtual void removeChilds();
    virtual void removeChild(CustomItem* item);

    int itemCount();
    void setIndex(unsigned int i);
    int selIndex() const;
    std::vector<int> selIndexList();
    void selClear();

    void setMultiSelect(bool on);
    CustomItem* itemAt(unsigned int index);

    void setOrientation(int orientation);

    std::vector<CustomItem*> & items();

    virtual void resize();

  private:

    Panel* listBoxPane_;
    bool multiSelect_;
    std::vector<CustomItem*> selItems_;
    std::vector<CustomItem*> items_;

    Skin itemBg;
    Skin itemFg;

    void onItemPress(ButtonEvent * ev);
    void deSelectItems();


  };

}

#endif
