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

#include "npopupwindow.h"
#include "nmenuseperator.h"


/**
@author Stefan
*/
class NPopupMenu : public NPopupWindow
{
public:
    NPopupMenu();

    ~NPopupMenu();


    virtual void add(NCustomMenuItem* item);
    virtual void setVisible(bool on);

    virtual void onMessage(NEvent* ev);
    virtual void onKeyPress(const NKeyEvent & event);

    virtual void removeChilds();


private:

   std::vector<class NCustomItem*> items;
   class NObject* lastOverItem;

};

#endif
