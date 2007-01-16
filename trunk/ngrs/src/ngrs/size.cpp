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
#include "size.h"

namespace ngrs {

  Size Size::zero(0,0,0,0);

  Size::Size()
  {
    left_=top_=right_=bottom_=0;
  }

  Size::Size( int left, int top, int right, int bottom )
  {
    setSize(left,top,right,bottom);
  }

  Size::~Size()
  {
  }

  int Size::left( ) const
  {
    return left_;
  }

  int Size::top( ) const
  {
    return top_;
  }

  int Size::right( ) const
  {
    return right_;
  }

  int Size::bottom( ) const
  {
    return bottom_;
  }

  void Size::setLeft( int left )
  {
    left_ = left;
  }

  void Size::setTop( int top )
  {
    top_ = top;
  }

  void Size::setRight( int right )
  {
    right_ = right;
  }

  void Size::setBottom( int bottom )
  {
    bottom_ = bottom;
  }

  void Size::setSize( int left, int top, int right, int bottom )
  { 
    left_   = left;
    top_    = top;
    right_  = right;
    bottom_ = bottom;
  }

  void Size::setSize( const Size & size )
  {
    setSize(size.left(),size.top(),size.right(),size.bottom());
  }

  bool Size::allZero( )
  {
    return (top_ == left_ == right_ == bottom_ == 0);
  }

  bool Size::operator ==( const Size & sz ) const
  {
    if (left_==sz.left() && right_== sz.right() && top_ == sz.top() && bottom_ == sz.bottom()) return true; else return false;
  }

  bool Size::operator !=( const Size & sz ) const
  {
    if (left_==sz.left() && right_== sz.right() && top_ == sz.top() && bottom_ == sz.bottom()) return false; else return true;
  }

  Size Size::clipBox( const Size & size )
  {
    // handles no negative -- only > 0
    Size sz;
    sz.setTop   (std::min(size.top()    , top() ));
    sz.setLeft  (std::min(size.left()   , left() ));

    sz.setRight (std::max(size.right()  , right() )) ;
    sz.setBottom(std::max(size.bottom() , bottom() ));
    return sz;
  }

  std::string Size::toString( )
  {
    return stringify(left_)+":"+stringify(top_)+":"+stringify(right_)+":"+stringify(bottom_);
  }

  std::string Size::stringify( int x )
  {
    std::ostringstream o;
    if (!(o << x))
      return "error"; else
      return o.str();
  }

}

