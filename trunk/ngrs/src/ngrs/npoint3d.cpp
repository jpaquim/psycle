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
#include "ngrs/npoint3d.h"

NPoint3D::NPoint3D() : x_(0), y_(0) , z_(0) {}

NPoint3D::NPoint3D( int x, int y, int z ) : x_(x),y_(y),z_(z) {}

NPoint3D::~NPoint3D()
{
}

void NPoint3D::setX( int x )
{
  x_ = x;
}

void NPoint3D::setY( int y )
{
  y_ = y;
}

void NPoint3D::setZ( int z )
{
  z_ = z;
}

int NPoint3D::x( ) const
{
  return x_;
}

int NPoint3D::y( ) const
{
  return y_;
}

int NPoint3D::z( ) const
{
  return z_;
}

void NPoint3D::setXYZ( int x, int y, int z )
{
  x_ = x;
  y_ = y;
  z_ = z;
}



