/***************************************************************************
 *   Copyright (C) 2006 by Stefan Nattkemper, Johan Boule                  *
 *   Made in Germany, France                                               *
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
#include "nregion.h"

NRegion::NRegion()
{
  #ifdef __unix__
  region_ = XCreateRegion();
  #endif
  update = true;
}

NRegion::NRegion( const NRect & rect )
{
  #ifdef __unix__
  region_ = XCreateRegion();
  XRectangle rectangle;
  rectangle.x= (short) rect.left();
  rectangle.y= (short) rect.top();
  rectangle.width=(unsigned short)  rect.width();
  rectangle.height=(unsigned short) rect.height();
  XUnionRectWithRegion( &rectangle, region_, region_ );
  #endif
  update = true;
}

NRegion::~NRegion()
{
  #ifdef __unix__
  XDestroyRegion( region_ );
  #endif
}

void NRegion::setRect( const NRect & rect )
{
  #ifdef __unix__
  XDestroyRegion(region_);
  region_ = XCreateRegion();
  XRectangle rectangle;
  rectangle.x= (short) rect.left();
  rectangle.y= (short) rect.top();
  rectangle.width=(unsigned short)  rect.width();
  rectangle.height=(unsigned short) rect.height();
  XUnionRectWithRegion( &rectangle, region_, region_ );
  #endif
  update = true;
}


// shouldnt be XPoint
void NRegion::setPolygon( NPoint*  pts , int size )
{
  #ifdef __unix__
  XDestroyRegion( region_ );
  XPoint pt[size];
  for (int i = 0; i< size; i++) {
    pt[i].x = pts[i].x();
    pt[i].y = pts[i].y();
  }
  region_ = XPolygonRegion( pt, size, WindingRule );
  #endif
  update = true;
}


NRegion::NRegion( const NRegion & src )
{
  #ifdef __unix__
  region_ = XCreateRegion();
  XUnionRegion( region_, src.xRegion(), region_ );
  #endif
  update = true;
}

const NRegion & NRegion::operator =( const NRegion & rhs )
{
  #ifdef __unix__
  XDestroyRegion( region_);
  region_ = XCreateRegion();
  XUnionRegion( region_, rhs.xRegion(), region_ );
  #endif
  update = true;
  return *this;
}


bool NRegion::isEmpty( ) const
{
  #ifdef __unix__
  return XEmptyRegion( region_ );
  #else
  ;
  #endif
}

void NRegion::move( int dx, int dy )
{
  #ifdef __unix__
  XOffsetRegion( region_, dx, dy );
  #endif
  update = true;
}

void NRegion::shrink( int dx, int dy )
{
  #ifdef __unix__
  XShrinkRegion( region_, dx, dy );
  #else
  ;
  #endif
  update = true;
}

const NRect & NRegion::rectClipBox( ) const
{
  if ( update ) {
    #ifdef __unix__
    XRectangle r;
    XClipBox( region_, &r  );
    clipBox.setPosition( r.x, r.y, r.width, r.height );
    #endif
    update = false;
  }
  return clipBox;
}

bool NRegion::intersects( int x, int y ) const
{
  #ifdef __unix__
  return XPointInRegion( region_, x, y );
  #endif
}
