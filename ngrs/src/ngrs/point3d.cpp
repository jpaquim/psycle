/***************************************************************************
 *   Copyright (C) 2005, 2006, 2007 by Stefan Nattkemper   *
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
#include "point3d.h"

namespace ngrs {

  Point3D::Point3D() : x_(0), y_(0) , z_(0) {}

  Point3D::Point3D( int x, int y, int z ) : x_(x),y_(y),z_(z) {}

  Point3D::~Point3D()
  {
  }

  void Point3D::setX( int x )
  {
    x_ = x;
  }

  void Point3D::setY( int y )
  {
    y_ = y;
  }

  void Point3D::setZ( int z )
  {
    z_ = z;
  }

  int Point3D::x( ) const
  {
    return x_;
  }

  int Point3D::y( ) const
  {
    return y_;
  }

  int Point3D::z( ) const
  {
    return z_;
  }

  void Point3D::setXYZ( int x, int y, int z )
  {
    x_ = x;
    y_ = y;
    z_ = z;
  }

}
