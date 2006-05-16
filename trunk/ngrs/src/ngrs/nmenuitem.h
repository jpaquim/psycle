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
#ifndef NMENUITEM_H
#define NMENUITEM_H

#include "ncustommenuitem.h"

class NLabel;
class NMenu;
class NImage;


/**
@author Stefan
*/


class NMenuItem : public NCustomMenuItem
{
public:
    NMenuItem();
    NMenuItem(const std::string & text);

    ~NMenuItem();

private:

    NMenu* menu_;

    NLabel* captionLbl_;
    NImage* iconImg_;
    NImage* subMenuImg_;

    void init();

};

#endif
