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
#include "ngrs/npoint.h"

NPoint::NPoint() : x_(0), y_(0) {}

NPoint::NPoint( int x, int y ) : x_(x),y_(y) {}

NPoint::~NPoint()
{
}

void NPoint::setX( int x )
{
  x_ = x;
}

void NPoint::setY( int y )
{
  y_ = y;
}

int NPoint::x( ) const
{
  return x_;
}

int NPoint::y( ) const
{
  return y_;
}

void NPoint::setXY( int x, int y )
{
  x_ = x;
  y_ = y;
}




