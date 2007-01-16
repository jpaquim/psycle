/***************************************************************************
 *   Copyright (C) 2005, 2006, 2007 by Stefan Nattkemper  *
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
#include "rect.h"
#include <algorithm>

namespace ngrs {

  Rect::Rect() : left_(0), top_(0), width_(0), height_(0)
  {}

  Rect::Rect(long left,long top,long width,long height): left_(left),top_(top),width_(width),height_(height)
  {}

  Rect::~Rect()
  {
  }

  void Rect::setPosition(long left,long top,long width, long height )
  {
    left_    = left;
    top_     = top;
    width_   = width;
    height_  = height;
  }

  long int Rect::left( ) const
  {
    return left_;
  }

  long int Rect::top( ) const
  {
    return top_;
  }

  long int Rect::width( ) const
  {
    return width_;
  }

  long int Rect::height( ) const
  {
    return height_;
  }

  bool Rect::hasIntersection( const Rect & r2) const
  {
    return (r2.left() < left_ + width_) && (r2.top() < top_ + height_) &&
      (r2.left() + r2.width() > left_) && (r2.top() + r2.height() > top_);
  }

  void Rect::setTop( long top )
  {
    top_ = top;
  }

  void Rect::setWidth( long width )
  {
    width_ = width;
  }

  void Rect::setHeight( long height )
  {
    height_ = height;
  }

  void Rect::setLeft( long left )
  {
    left_ = left;
  }

  bool Rect::intersects( const NPoint & p ) const
  {
    if ((left() <= p.x()) && (width()+ left() >= p.x()) && (top()  <= p.y())
      && (height()+top()  >= p.y())) return true; else return false;
  }


  void Rect::intersects( const Rect & r2, Rect & result ) const
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

  bool Rect::operator ==( const Rect & rhs ) const
  {
    return (rhs.top()==top() && rhs.left()==left() && rhs.width()==width() && rhs.height() == height());
  }

  bool Rect::operator !=( const Rect & rhs ) const
  {
    return !(*this == rhs);
  }

  bool Rect::intersects( long x, long y ) const
  {
    if ((left() <= x) && (width()+ left() >= x) && (top()  <= y)
      && (height()+top()  >= y))
      return true;
    else 
      return false;
  }

}
