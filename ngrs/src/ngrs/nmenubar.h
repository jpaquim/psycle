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
#ifndef NMENUBAR_H
#define NMENUBAR_H

#include "ngrs/npanel.h"

class NMenu;


/**
@author Stefan
*/
class NMenuBar : public NPanel
{
public:

    NMenuBar();

    ~NMenuBar();

    virtual void add(NMenu* menu);
    virtual void onMessage(NEvent* ev);

private:

    bool isMenuMapped_;
    NMenu* lastMenuOver;

    std::vector<NObject*> menus;

};

#endif
