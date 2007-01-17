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
    platformRegion_ = XCreateRegion();
#else
    platformRegion_ = CreateRectRgn(0,0,0,0);
#endif
    update = true;
  }

  Region::Region( const Rect & rect )
  {
#ifdef __unix__
    platformRegion_ = XCreateRegion();
    XRectangle rectangle;
    rectangle.x = static_cast<short>( rect.left() );
    rectangle.y = static_cast<short>( rect.top() ) ;
    rectangle.width = static_cast<unsigned short>( rect.width() );
    rectangle.height= static_cast<unsigned short>( rect.height() );
    XUnionRectWithRegion( &rectangle, platformRegion_, platformRegion_ );
#else
    platformRegion_ = CreateRectRgn( rect.left(), rect.top(), rect.left() + rect.width() , rect.top() + rect.height() );
#endif

    update = true;
  }

  Region::Region( PlatformRegion platformRegion ) {
#ifdef __unix__
    platformRegion_ = XCreateRegion();
    XUnionRegion( platformRegion_, platformRegion, platformRegion_ );
#else
    platformRegion_ = CreateRectRgn( 0, 0, 0, 0 );
    CombineRgn( platformRegion_, platformRegion, NULL, RGN_COPY);
#endif
    update = true;
  }

  Region::~Region()
  {
#ifdef __unix__
    XDestroyRegion( platformRegion_ );
#else
    DeleteObject( platformRegion_ );
#endif
  }

  void Region::setRect( const Rect& rect )
  {
    if ( !isEmpty() ) {
#ifdef __unix__
      XDestroyRegion(platformRegion_);
      platformRegion_ = XCreateRegion();
      XRectangle rectangle;
      rectangle.x = static_cast<short>( rect.left() );
      rectangle.y = static_cast<short>( rect.top() ) ;
      rectangle.width = static_cast<unsigned short>( rect.width() );
      rectangle.height= static_cast<unsigned short>( rect.height() );
      XUnionRectWithRegion( &rectangle, platformRegion_, platformRegion_ );
#else
      ::DeleteObject( platformRegion_ );
      platformRegion_ = CreateRectRgn( rect.left(), rect.top(), rect.left() + rect.width() , rect.top() + rect.height() );
#endif
    } else {
#ifdef __unix__
      ::XRectangle rectangle;      
      rectangle.x = static_cast<short>( rect.left() );
      rectangle.y = static_cast<short>( rect.top() ) ;
      rectangle.width = static_cast<unsigned short>( rect.width() );
      rectangle.height= static_cast<unsigned short>( rect.height() );
      ::XUnionRectWithRegion( &rectangle, platformRegion_, platformRegion_ );
#else
      ::DeleteObject( platformRegion_ );
      platformRegion_ = CreateRectRgn( rect.left(), rect.top(), rect.left() + rect.width() , rect.top() + rect.height() );
#endif
    }
    update = true;
  }

  void Region::setPolygon( NPoint*  pts , int size )
  {
#ifdef __unix__
    ::XDestroyRegion( platformRegion_ );
    ::XPoint* pt = new ::XPoint[size];
#else
    ::DeleteObject( platformRegion_ );
    ::POINT* pt = new ::POINT[size];
#endif

    for (int i = 0; i< size; i++) {
      pt[i].x = pts[i].x();
      pt[i].y = pts[i].y();
    }
#ifdef __unix__
    platformRegion_ = ::XPolygonRegion( pt, size, WindingRule );
#else
    platformRegion_ = ::CreatePolygonRgn( pt, size, WINDING );
#endif    
    update = true;
    delete[] pt;
  }

  Region::Region( const Region& src )
  {
#ifdef __unix__
    platformRegion_ = XCreateRegion();
    XUnionRegion( platformRegion_, src.asPlatformRegion(), platformRegion_ );
#else
    platformRegion_ = CreateRectRgn( 0, 0, 0, 0 );
    CombineRgn( platformRegion_, src.asPlatformRegion(), NULL, RGN_COPY );
#endif
    update = true;
  }

  bool Region::isEmpty( ) const
  {
#ifdef __unix__
    return ::XEmptyRegion( platformRegion_ );
#else
    ::RECT rect;
    rect.left = 0;
    rect.top = 0;
    rect. bottom = 0;
    rect. right = 0;
    return NULLREGION == GetRgnBox( platformRegion_, &rect);
#endif
  }

  void Region::move( int dx, int dy )
  {
#ifdef __unix__
    ::XOffsetRegion( platformRegion_, dx, dy );
#else
    ::OffsetRgn( platformRegion_, dx,  dy );   
#endif
    update = true;
  }

  void Region::shrink( int dx, int dy )
  {
#ifdef __unix__
    ::XShrinkRegion( platformRegion_, dx, dy );
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
      ::XClipBox( platformRegion_, &r  );
      clipBox.setPosition( r.x, r.y, r.width, r.height );
#else
      ::RECT r;
      ::GetRgnBox( platformRegion_, &r );
      clipBox.setPosition( r.left, r.top, r.right - r.left, r.bottom - r.top );
#endif
      update = false;
    }
    return clipBox;
  }

  bool Region::intersects( int x, int y ) const
  {
#ifdef __unix__
    return ::XPointInRegion( platformRegion_, x, y );
#else
    return ::PtInRegion( platformRegion_, x, y ) != 0;
#endif
  }


  // operator implementation

  Region& Region::operator=( const Region& rhs )
  {
    if ( this == &rhs ) return *this;
#ifdef __unix__
    ::XDestroyRegion( platformRegion_);
    platformRegion_ = ::XCreateRegion();      
    ::XUnionRegion( platformRegion_, rhs.asPlatformRegion(), platformRegion_ );
#else
    ::DeleteObject( platformRegion_ );
    platformRegion_ = ::CreateRectRgn( 0, 0, 0, 0 );
    ::CombineRgn( platformRegion_, rhs.asPlatformRegion(), NULL, RGN_COPY );
#endif
    update = true;
    return *this;
  }

  Region& Region::operator&=( const Region& rhs )
  {
    if ( this == &rhs ) return *this;
#ifdef __unix__
    ::XIntersectRegion( platformRegion_, rhs, platformRegion_);
#else
    ::CombineRgn( platformRegion_, rhs.asPlatformRegion(), platformRegion_, RGN_AND );
#endif
    return *this;
  }

  Region Region::operator&( const Region& rhs )
  {
    return Region(*this) &= rhs;
  }

  Region& Region::operator|=( const Region& rhs )
  {
    if ( this == &rhs ) return *this;
#ifdef __unix__
    ::XUnionRegion( platformRegion_, rhs, platformRegion_ );
#else
    ::CombineRgn( platformRegion_, rhs.asPlatformRegion(), platformRegion_, RGN_OR );
#endif
    return *this;
  }

  Region Region::operator|( const Region& rhs )
  {
    return Region(*this) |= rhs;
  }

  Region& Region::operator-=( const Region& rhs )
  {
#ifdef __unix__
    ::XSubtractRegion( platformRegion_, rhs, platformRegion_ );
#else
    ::CombineRgn( platformRegion_, rhs.asPlatformRegion(), platformRegion_, RGN_DIFF);
#endif
    return *this;
  }

  Region Region::operator-( const Region& rhs )
  {
    return Region(*this) -= rhs;
  }

  Region& Region::operator^=( const Region& rhs )
  {
#ifdef __unix__
    ::XXorRegion( platformRegion_, rhs, platformRegion_ );
#else
    ::CombineRgn( platformRegion_, rhs.asPlatformRegion(), platformRegion_, RGN_XOR );
#endif
    return *this;
  }

  Region Region::operator^( const Region& rhs )
  {
    return Region(*this) -= rhs;
  }

}
