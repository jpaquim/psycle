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
#ifndef NREGION_H
#define NREGION_H

#include "system.h"
#include "rect.h"

/**
@author Stefan Nattkemper
*/


namespace ngrs {
  class Region;
}

namespace ngrs {

#ifdef __unix__
  typedef ::Region PlatformRegion;
#else
  typedef HRGN PlatformRegion;
#endif
  
  class Region{
  public:
    Region();
    Region( const Rect & rect );
    Region( PlatformRegion sRegion );

    ~Region();

    Region( const Region & src );
    const Region & operator= ( const Region & rhs );

    void shrink( int dx, int dy );
    void move( int dx, int dy );
    void setRect( const Rect & rect );
    void setPolygon( NPoint*  pts, int size );
    bool isEmpty() const;
    const Rect & rectClipBox() const;
    bool intersects( int x, int y ) const;


    inline Region & operator &= ( const Region & ); // intersection

    inline Region & operator |= ( const Region & ); // union

    inline Region & operator -= ( const Region & ); // difference

    inline Region & operator ^= ( const Region & ); // symetric difference


    // warning: this pointer can change

    inline PlatformRegion sRegion() const  { return region_; }

    // implicit conversion to X const Region.
    inline operator const PlatformRegion () const  { return region_; }

    // implicit conversion to X Region.
    inline operator PlatformRegion ()  { return region_; }


  private:

    PlatformRegion region_;
    mutable Rect clipBox;
    mutable bool update;
  };

  inline Region & Region::operator &= ( const Region & that )
  {
#ifdef __unix__
    XIntersectRegion(*this, that, *this);
#else
    CombineRgn(*this, that, *this, RGN_AND);
#endif
    return *this;
  }

  inline Region operator & ( const Region & lhs, const Region & rhs )
  {
    Region nrv( lhs );
    nrv &= rhs;
    return nrv;
  }

  inline Region & Region::operator |= ( const Region & that )
  {
#ifdef __unix__
    XUnionRegion( *this, that, *this );
#else
    CombineRgn( *this, that, *this, RGN_OR);
#endif
    return *this;
  }

  inline Region operator | ( const Region & lhs, const Region & rhs )
  {
    Region nrv( lhs );
    nrv |= rhs;
    return nrv;
  }

  inline Region & Region::operator -= ( const Region & that )
  {
#ifdef __unix__
    XSubtractRegion( *this, that, *this );
#else
    CombineRgn( *this, that, *this, RGN_DIFF);
#endif
    return *this;
  }

  inline Region operator - ( const Region & lhs, const Region & rhs )
  {
    Region nrv( lhs );
    nrv -= rhs;
    return nrv;
  }

  inline Region & Region::operator ^= ( const Region & that )
  {
#ifdef __unix__
    XXorRegion( *this, that, *this );
#else
    CombineRgn( *this, that, *this, RGN_XOR);
#endif
    return *this;
  }

  inline Region operator ^ ( const Region & lhs , const Region & rhs )
  {
    Region nrv( lhs );
    nrv ^= rhs;
    return nrv;
  }

}

#endif
