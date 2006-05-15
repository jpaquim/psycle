/***************************************************************************
 *   Copyright (C) 2005 by Stefan   *
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
#ifndef NLISTBOX_H
#define NLISTBOX_H

#include "nscrollbox.h"
#include "nlistlayout.h"
#include "ncustomitem.h"
#include "nitemevent.h"

/**
@author Stefan
*/
class NListBox : public NScrollBox
{
public:
    NListBox();

    ~NListBox();

    signal1<NItemEvent*> itemSelected;

    void add(NCustomItem* item);
    void add(NCustomItem* item, bool align);
    void insert(NCustomItem* item, int index, bool align = true);

    virtual void onItemSelected(NCustomItem * item);
    virtual void removeChilds();

    int itemCount();
    void setIndex(unsigned int i);
    int selIndex() const;
    std::vector<int> selIndexList();
    void selClear();

    void setMultiSelect(bool on);
    NCustomItem* itemAt(unsigned int index);

private:

    NPanel* listBoxPane_;
    bool multiSelect_;
    std::vector<NCustomItem*> selItems_;

    NSkin itemBg;
    NSkin itemFg;

    void onItemPress(NButtonEvent * ev);
    void deSelectItems();


};

#endif
