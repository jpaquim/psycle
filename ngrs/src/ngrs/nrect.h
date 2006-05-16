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
#ifndef NRECT_H
#define NRECT_H

#include "ngrs/npoint.h"

/**
@author Stefan
*/
class NRect{
public:
    NRect();
    NRect(long int left, long int top, long int width, long int height);

    ~NRect();

    void setPosition(long int left, long int top, long int width, long int height);
    void setLeft(long int left);
    void setTop(long int top);
    void setWidth(long int width);
    void setHeight(long int height);

    long int left() const;
    long int top() const;
    long int width() const;
    long int height() const;
    long int right() const;
    long int bottom() const;

    bool hasIntersection( const NRect & r2) const;
    bool intersects(const NPoint & p);
    bool intersects(long x, long y);
    void intersects( const NRect & r2, NRect & result ) const;

    bool operator==(const NRect & rhs) const;
    bool operator!=(const NRect & rhs) const;

private:

    long int left_;
    long int top_;
    long int width_;
    long int height_;
};

#endif
