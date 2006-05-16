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
#include "nsize.h"

NSize NSize::zero(0,0,0,0);


NSize::NSize()
{
  left_=top_=right_=bottom_=0;
}

NSize::NSize( int left, int top, int right, int bottom )
{
 setSize(left,top,right,bottom);
}


NSize::~NSize()
{
}

int NSize::left( ) const
{
  return left_;
}

int NSize::top( ) const
{
  return top_;
}

int NSize::right( ) const
{
  return right_;
}

int NSize::bottom( ) const
{
  return bottom_;
}

void NSize::setLeft( int left )
{
  left_ = left;
}

void NSize::setTop( int top )
{
  top_ = top;
}

void NSize::setRight( int right )
{
  right_ = right;
}

void NSize::setBottom( int bottom )
{
  bottom_ = bottom;
}

void NSize::setSize( int left, int top, int right, int bottom )
{ 
  left_   = left;
  top_    = top;
  right_  = right;
  bottom_ = bottom;
}

void NSize::setSize( const NSize & size )
{
  setSize(size.left(),size.top(),size.right(),size.bottom());
}

bool NSize::allZero( )
{
  return (top_ == left_ == right_ == bottom_ == 0);
}

bool NSize::operator ==( const NSize & sz ) const
{
  if (left_==sz.left() && right_== sz.right() && top_ == sz.top() && bottom_ == sz.bottom()) return true; else return false;
}

bool NSize::operator !=( const NSize & sz ) const
{
   if (left_==sz.left() && right_== sz.right() && top_ == sz.top() && bottom_ == sz.bottom()) return false; else return true;
}

NSize NSize::clipBox( const NSize & size )
{
  // handles no negative -- only > 0
  NSize sz;
  sz.setTop   (std::min(size.top()    , top() ));
  sz.setLeft  (std::min(size.left()   , left() ));

  sz.setRight (std::max(size.right()  , right() )) ;
  sz.setBottom(std::max(size.bottom() , bottom() ));
  return sz;
}

std::string NSize::toString( )
{
  return stringify(left_)+":"+stringify(top_)+":"+stringify(right_)+":"+stringify(bottom_);
}

std::string NSize::stringify( int x )
{
     std::ostringstream o;
    if (!(o << x))
      return "error"; else
    return o.str();
}


