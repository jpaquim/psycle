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
#ifndef NPOPUPMENU_H
#define NPOPUPMENU_H

#include "nwindow.h"
#include "nlistlayout.h"
#include "nmenuseperator.h"


/**
@author Stefan
*/
class NPopupMenu : public NWindow
{
public:
    NPopupMenu();

    ~NPopupMenu();

    signal1<NObject*> hideRequest;
    signal1<NObject*> leftPress;
    signal1<NObject*> rightPress;

    virtual void add(NCustomMenuItem* item);

    virtual void setVisible(bool on);
    void addSeperator();

    void setSelItem(int index);

    virtual void updateAlign();
    virtual void onKeyPress(const NKeyEvent & keyEvent);

    void onSubMenuMapped(NObject* sender);

    NCustomMenuItem* itemByName(const std::string & name);

private:

   NListLayout* nl;

   NCustomMenuItem* subMenuItem_;
   int selItemIndex_;

   void onItemEnter(NEvent * ev);

   std::vector<NCustomMenuItem*> items;
};

#endif
