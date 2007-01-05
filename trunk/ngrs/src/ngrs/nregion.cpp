/***************************************************************************
 *   Copyright (C) 2005, 2006, 2007 by Stefan Nattkemper                   *
 *   Made in Germany                                                       *
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
  #else
  region_ = CreateRectRgn(0,0,0,0);
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
  #else
  region_ = CreateRectRgn( rect.left(), rect.top(), rect.left() + rect.width() , rect.top() + rect.height() );
  #endif
  
  update = true;
}

NRegion::~NRegion()
{
  #ifdef __unix__
  XDestroyRegion( region_ );
  #else
  DeleteObject( region_ );
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
  #else
  DeleteObject( region_ );
  region_ = CreateRectRgn( rect.left(), rect.top(), rect.left() + rect.width() , rect.top() + rect.height() );
  #endif
  update = true;
}


// shouldnt be XPoint
void NRegion::setPolygon( NPoint*  pts , int size )
{
  #ifdef __unix__
  XDestroyRegion( region_ );
  XPoint* pt = new XPoint[size];
  #else
  DeleteObject( region_ );
  POINT* pt = new POINT[size];
  #endif
  
  for (int i = 0; i< size; i++) {
    pt[i].x = pts[i].x();
    pt[i].y = pts[i].y();
  }
  #ifdef __unix__
  region_ = XPolygonRegion( pt, size, WindingRule );
  #else
  region_ = CreatePolygonRgn( pt, size, WINDING );
  #endif    
  update = true;
  delete[] pt;
}


NRegion::NRegion( const NRegion & src )
{
  #ifdef __unix__
  region_ = XCreateRegion();
  XUnionRegion( region_, src.sRegion(), region_ );
  #else
  region_ = CreateRectRgn( 0, 0, 0, 0 );
  CombineRgn( region_, src.sRegion(), NULL, RGN_COPY );
  #endif
  update = true;
}

const NRegion & NRegion::operator =( const NRegion & rhs )
{
  #ifdef __unix__
  XDestroyRegion( region_);
  region_ = XCreateRegion();
  XUnionRegion( region_, rhs.sRegion(), region_ );
  #else
  DeleteObject( region_ );
  region_ = CreateRectRgn( 0, 0, 0, 0 );
  CombineRgn( region_, rhs.sRegion(), NULL, RGN_COPY );    
  #endif
  update = true;
  return *this;
}


bool NRegion::isEmpty( ) const
{
  #ifdef __unix__
  return XEmptyRegion( region_ );
  #else
  RECT rect;
  rect.left = 0;
  rect.top = 0;
  rect. bottom = 0;
  rect. right = 0;
  return NULLREGION == GetRgnBox( region_, &rect);
  #endif
}

void NRegion::move( int dx, int dy )
{
  #ifdef __unix__
  XOffsetRegion( region_, dx, dy );
  #else
  OffsetRgn( region_, dx,  dy );   
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
    #else
    RECT r;
    GetRgnBox( region_, &r );
    clipBox.setPosition( r.left, r.top, r.right - r.left, r.bottom - r.top );
    #endif
    update = false;
  }
  return clipBox;
}

bool NRegion::intersects( int x, int y ) const
{
  #ifdef __unix__
  return XPointInRegion( region_, x, y );
  #else
  return PtInRegion( region_, x, y ) != 0;
  #endif
}
