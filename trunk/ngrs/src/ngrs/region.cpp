/***************************************************************************
*   Copyright (C) 2005, 2006, 2007 by  Stefan Nattkemper                   *
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
#include "region.h"

namespace ngrs {

  Region::Region()
  {
#ifdef __unix__
    platformRegionHandle_ = XCreateRegion();
#else
    platformRegionHandle_ = CreateRectRgn(0,0,0,0);
#endif
    update = true;
  }

  Region::Region( const Rect & rect )
  {
#ifdef __unix__
    platformRegionHandle_ = XCreateRegion();
    XRectangle rectangle;
    rectangle.x = static_cast<short>( rect.left() );
    rectangle.y = static_cast<short>( rect.top() ) ;
    rectangle.width = static_cast<unsigned short>( rect.width() );
    rectangle.height= static_cast<unsigned short>( rect.height() );
    XUnionRectWithRegion( &rectangle, platformRegionHandle_, platformRegionHandle_ );
#else
    platformRegionHandle_ = CreateRectRgn( rect.left(), rect.top(), rect.left() + rect.width() , rect.top() + rect.height() );
#endif

    update = true;
  }

  Region::Region( PlatformRegionHandle platformRegionHandle ) {
#ifdef __unix__
    platformRegionHandle_ = XCreateRegion();
    XUnionRegion( platformRegionHandle_, platformRegionHandle, platformRegionHandle_ );
#else
    platformRegionHandle_ = CreateRectRgn( 0, 0, 0, 0 );
    CombineRgn( platformRegionHandle_, platformRegionHandle, NULL, RGN_COPY);
#endif
    update = true;
  }

  Region::~Region()
  {
#ifdef __unix__
    XDestroyRegion( platformRegionHandle_ );
#else
    DeleteObject( platformRegionHandle_ );
#endif
  }

  void Region::setRect( const Rect& rect )
  {
    if ( !empty() ) {
#ifdef __unix__
      XDestroyRegion(platformRegionHandle_);
      platformRegionHandle_ = XCreateRegion();
      XRectangle rectangle;
      rectangle.x = static_cast<short>( rect.left() );
      rectangle.y = static_cast<short>( rect.top() ) ;
      rectangle.width = static_cast<unsigned short>( rect.width() );
      rectangle.height= static_cast<unsigned short>( rect.height() );
      XUnionRectWithRegion( &rectangle, platformRegionHandle_, platformRegionHandle_ );
#else
      ::DeleteObject( platformRegionHandle_ );
      platformRegionHandle_ = CreateRectRgn( rect.left(), rect.top(), rect.left() + rect.width() , rect.top() + rect.height() );
#endif
    } else {
#ifdef __unix__
      ::XRectangle rectangle;      
      rectangle.x = static_cast<short>( rect.left() );
      rectangle.y = static_cast<short>( rect.top() ) ;
      rectangle.width = static_cast<unsigned short>( rect.width() );
      rectangle.height= static_cast<unsigned short>( rect.height() );
      ::XUnionRectWithRegion( &rectangle, platformRegionHandle_, platformRegionHandle_ );
#else
      ::DeleteObject( platformRegionHandle_ );
      platformRegionHandle_ = CreateRectRgn( rect.left(), rect.top(), rect.left() + rect.width() , rect.top() + rect.height() );
#endif
    }
    update = true;
  }

  void Region::setPolygon( Point*  pts , int size )
  {
#ifdef __unix__
    ::XDestroyRegion( platformRegionHandle_ );
    ::XPoint* pt = new ::XPoint[size];
#else
    ::DeleteObject( platformRegionHandle_ );
    ::POINT* pt = new ::POINT[size];
#endif

    for (int i = 0; i< size; i++) {
      pt[i].x = pts[i].x();
      pt[i].y = pts[i].y();
    }
#ifdef __unix__
    platformRegionHandle_ = ::XPolygonRegion( pt, size, WindingRule );
#else
    platformRegionHandle_ = ::CreatePolygonRgn( pt, size, WINDING );
#endif    
    update = true;
    delete[] pt;
  }

  Region::Region( const Region& src )
  {
#ifdef __unix__
    platformRegionHandle_ = XCreateRegion();
    XUnionRegion( platformRegionHandle_, src.asPlatformRegionHandle(), platformRegionHandle_ );
#else
    platformRegionHandle_ = CreateRectRgn( 0, 0, 0, 0 );
    CombineRgn( platformRegionHandle_, src.asPlatformRegionHandle(), NULL, RGN_COPY );
#endif
    update = true;
  }

  bool Region::empty( ) const
  {
#ifdef __unix__
    return ::XEmptyRegion( platformRegionHandle_ );
#else
    ::RECT rect;
    rect.left = 0;
    rect.top = 0;
    rect. bottom = 0;
    rect. right = 0;
    return NULLREGION == GetRgnBox( platformRegionHandle_, &rect);
#endif
  }

  void Region::move( int dx, int dy )
  {
#ifdef __unix__
    ::XOffsetRegion( platformRegionHandle_, dx, dy );
#else
    ::OffsetRgn( platformRegionHandle_, dx,  dy );   
#endif
    update = true;
  }

  void Region::shrink( int dx, int dy )
  {
#ifdef __unix__
    ::XShrinkRegion( platformRegionHandle_, dx, dy );
#else
    ;
#endif
    update = true;
  }

  const Rect& Region::rectClipBox( ) const
  {
    if ( update ) {
#ifdef __unix__
      ::XRectangle r;
      ::XClipBox( platformRegionHandle_, &r  );
      clipBox.setPosition( r.x, r.y, r.width, r.height );
#else
      ::RECT r;
      ::GetRgnBox( platformRegionHandle_, &r );
      clipBox.setPosition( r.left, r.top, r.right - r.left, r.bottom - r.top );
#endif
      update = false;
    }
    return clipBox;
  }

  bool Region::intersects( int x, int y ) const
  {
#ifdef __unix__
    return ::XPointInRegion( platformRegionHandle_, x, y );
#else
    return ::PtInRegion( platformRegionHandle_, x, y ) != 0;
#endif
  }


  // operator implementation

  Region& Region::operator=( const Region& rhs )
  {
    if ( this == &rhs ) return *this;
#ifdef __unix__
    ::XDestroyRegion( platformRegionHandle_);
    platformRegionHandle_ = ::XCreateRegion();      
    ::XUnionRegion( platformRegionHandle_, rhs.asPlatformRegionHandle(), platformRegionHandle_ );
#else
    ::DeleteObject( platformRegionHandle_ );
    platformRegionHandle_ = ::CreateRectRgn( 0, 0, 0, 0 );
    ::CombineRgn( platformRegionHandle_, rhs.asPlatformRegionHandle(), NULL, RGN_COPY );
#endif
    update = true;
    return *this;
  }

  Region& Region::operator&=( const Region& rhs )
  {
    if ( this == &rhs ) return *this;
#ifdef __unix__
    ::XIntersectRegion( platformRegionHandle_, rhs.asPlatformRegionHandle(), platformRegionHandle_);
#else
    ::CombineRgn( platformRegionHandle_, rhs.asPlatformRegionHandle(), platformRegionHandle_, RGN_AND );
#endif
    return *this;
  }

  const Region Region::operator&( const Region& rhs ) const
  {
    return Region(*this) &= rhs;
  }

  Region& Region::operator|=( const Region& rhs )
  {
    if ( this == &rhs ) return *this;
#ifdef __unix__
    ::XUnionRegion( platformRegionHandle_, rhs.asPlatformRegionHandle(), platformRegionHandle_ );
#else
    ::CombineRgn( platformRegionHandle_, rhs.asPlatformRegionHandle(), platformRegionHandle_, RGN_OR );
#endif
    return *this;
  }

  const Region Region::operator|( const Region& rhs ) const
  {
    return Region(*this) |= rhs;
  }

  Region& Region::operator-=( const Region& rhs )
  {
#ifdef __unix__
    ::XSubtractRegion( platformRegionHandle_, rhs.asPlatformRegionHandle(), platformRegionHandle_ );
#else
    ::CombineRgn( platformRegionHandle_, rhs.asPlatformRegionHandle(), platformRegionHandle_, RGN_DIFF);
#endif
    return *this;
  }

  const Region Region::operator-( const Region& rhs ) const
  {
    return Region(*this) -= rhs;
  }

  Region& Region::operator^=( const Region& rhs )
  {
#ifdef __unix__
    ::XXorRegion( platformRegionHandle_, rhs.asPlatformRegionHandle(), platformRegionHandle_ );
#else
    ::CombineRgn( platformRegionHandle_, rhs.asPlatformRegionHandle(), platformRegionHandle_, RGN_XOR );
#endif
    return *this;
  }

  const Region Region::operator^( const Region& rhs ) const
  {
    return Region(*this) -= rhs;
  }

}
