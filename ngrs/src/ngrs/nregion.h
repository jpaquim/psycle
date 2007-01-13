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

#include "nsystem.h"
#include "nrect.h"

/**
@author Stefan Nattkemper
*/

namespace ngrs {

#ifdef __unix__
#else
  typedef HRGN Region;
#endif

  class NRegion{
  public:
    NRegion();
    NRegion( const NRect & rect );

    ~NRegion();

    NRegion( const NRegion & src );
    const NRegion & operator= ( const NRegion & rhs );

    void shrink( int dx, int dy );
    void move( int dx, int dy );
    void setRect( const NRect & rect );
    void setPolygon( NPoint*  pts, int size );
    bool isEmpty() const;
    const NRect & rectClipBox() const;
    bool intersects( int x, int y ) const;


    inline NRegion & operator &= ( const NRegion & ); // intersection

    inline NRegion & operator |= ( const NRegion & ); // union

    inline NRegion & operator -= ( const NRegion & ); // difference

    inline NRegion & operator ^= ( const NRegion & ); // symetric difference


    // warning: this pointer can change

    inline Region sRegion() const  { return region_; }

    // implicit conversion to X const Region.
    inline operator const Region () const  { return region_; }

    // implicit conversion to X Region.
    inline operator Region ()  { return region_; }


  private:

    Region region_;
    mutable NRect clipBox;
    mutable bool update;
  };

  inline NRegion & NRegion::operator &= ( const NRegion & that )
  {
#ifdef __unix__
    XIntersectRegion(*this, that, *this);
#else
    CombineRgn(*this, that, *this, RGN_AND);
#endif
    return *this;
  }

  inline NRegion operator & ( const NRegion & lhs, const NRegion & rhs )
  {
    NRegion nrv( lhs );
    nrv &= rhs;
    return nrv;
  }

  inline NRegion & NRegion::operator |= ( const NRegion & that )
  {
#ifdef __unix__
    XUnionRegion( *this, that, *this );
#else
    CombineRgn( *this, that, *this, RGN_OR);
#endif
    return *this;
  }

  inline NRegion operator | ( const NRegion & lhs, const NRegion & rhs )
  {
    NRegion nrv( lhs );
    nrv |= rhs;
    return nrv;
  }

  inline NRegion & NRegion::operator -= ( const NRegion & that )
  {
#ifdef __unix__
    XSubtractRegion( *this, that, *this );
#else
    CombineRgn( *this, that, *this, RGN_DIFF);
#endif
    return *this;
  }

  inline NRegion operator - ( const NRegion & lhs, const NRegion & rhs )
  {
    NRegion nrv( lhs );
    nrv -= rhs;
    return nrv;
  }

  inline NRegion & NRegion::operator ^= ( const NRegion & that )
  {
#ifdef __unix__
    XXorRegion( *this, that, *this );
#else
    CombineRgn( *this, that, *this, RGN_XOR);
#endif
    return *this;
  }

  inline NRegion operator ^ ( const NRegion & lhs , const NRegion & rhs )
  {
    NRegion nrv( lhs );
    nrv ^= rhs;
    return nrv;
  }

}

#endif
