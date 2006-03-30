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
#ifndef NMOVEABLE_H
#define NMOVEABLE_H

enum NMoveType{ nMvHorizontal = 1, nMvVertical = 2, nMvParentLimit = 4, nMvRectPicker = 8, nMvPolygonPicker = 16, nMvBorder=32, nMvRepaint = 64};

/**
@author Stefan Nattkemper
*/
class NMoveable{
public:
    NMoveable();
    NMoveable(int style);

    ~NMoveable();

    int style() const;
    void setStyle(int style);
    void addStyle(int style);
    void removeStyle(int style);

private:

   int style_;

};

#endif
