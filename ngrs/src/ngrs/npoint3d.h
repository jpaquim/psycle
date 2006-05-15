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
#ifndef NPOINT3D_H
#define NPOINT3D_H

/**
@author Stefan
*/
class NPoint3D{
public:
    NPoint3D();
    NPoint3D(int x, int y, int z);

    ~NPoint3D();

    void setX(int x);
    void setY(int y);
    void setZ(int z);
    void setXYZ(int x, int y, int z);

    int x() const;
    int y() const;
    int z() const;

private:

    int x_, y_ , z_;

};


#endif
