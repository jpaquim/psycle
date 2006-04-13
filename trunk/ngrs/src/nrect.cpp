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
#include "nrect.h"
#include <algorithm>

NRect::NRect() : left_(0), top_(0), width_(0), height_(0)
{}

NRect::NRect(long left,long top,long width,long height): left_(left),top_(top),width_(width),height_(height)
{}

NRect::~NRect()
{
}

void NRect::setPosition(long left,long top,long width, long height )
{
  left_    = left;
  top_     = top;
  width_   = width;
  height_  = height;
}

long int NRect::left( ) const
{
  return left_;
}

long int NRect::top( ) const
{
  return top_;
}

long int NRect::width( ) const
{
  return width_;
}

long int NRect::height( ) const
{
  return height_;
}

bool NRect::hasIntersection( const NRect & r2) const
{
  return (r2.left() < left_ + width_) && (r2.top() < top_ + height_) &&
                (r2.left() + r2.width() > left_) && (r2.top() + r2.height() > top_);
}

void NRect::setTop( long top )
{
  top_ = top;
}

void NRect::setWidth( long width )
{
  width_ = width;
}

void NRect::setHeight( long height )
{
  height_ = height;
}

void NRect::setLeft( long left )
{
  left_ = left;
}

bool NRect::intersects( const NPoint & p )
{
  if ((left() <= p.x()) && (width()+ left() >= p.x()) && (top()  <= p.y())
                        && (height()+top()  >= p.y())) return true; else return false;
}


void NRect::intersects( const NRect & r2, NRect & result ) const
{
    int ymin = std::max(top(),  r2.top());
    int ymax = std::min(top() + height(), r2.top() + r2.height());
    if (ymin > ymax) {
      result.setPosition(0,0,0,0); return;
    }
    int xmax = std::min(left() + this->width(), r2.left() + r2.width());
    int xmin = std::max(left(), r2.left());
    if (xmin > xmax) {
      result.setPosition(0,0,0,0); return;
    }
    // there is an intersection which is the rectangle  { xmin, ymin, xmax, ymax } 
    result.setPosition(xmin,ymin,xmax-xmin,ymax-ymin);
}

bool NRect::operator ==( const NRect & rhs ) const
{
  return (rhs.top()==top() && rhs.left()==left() && rhs.width()==width() && rhs.height() == height());
}

bool NRect::operator !=( const NRect & rhs ) const
{
 return !(*this == rhs);
}

bool NRect::intersects( long x, long y )
{
   if ((left() <= x) && (width()+ left() >= x) && (top()  <= y)
                        && (height()+top()  >= y))
      return true;
   else 
      return false;
}


