/***************************************************************************
*   Copyright (C) 2005, 2006, 2007 by  Stefan Nattkemper                   *
*   Made in Germany                                                       *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
*   This program is distributed in the hope rhs it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
*   You should have received a copy of the GNU General Public License     *
*   along with this program; if not, write to the                         *
*   Free Software Foundation, Inc.,                                       *
*   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
***************************************************************************/
#ifndef REGION_H
#define REGION_H

#include "system.h"
#include "rect.h"

/**
@author  Stefan Nattkemper
*/

namespace ngrs {

#ifdef __unix__
  typedef ::Region PlatformRegion;
#else
  typedef ::HRGN PlatformRegion;
#endif

  class Region{
  public:
    Region();
    Region( const Rect& rect );
    Region( PlatformRegion platformRegion );

    ~Region();

    Region( const Region& src );

    void shrink( int dx, int dy );
    void move( int dx, int dy );
    void setRect( const Rect & rect );
    void setPolygon( NPoint*  pts, int size );
    bool isEmpty() const;
    const Rect & rectClipBox() const;
    bool intersects( int x, int y ) const;

    Region& operator=( const Region& rhs );
    Region& operator&=( const Region& rhs ); // intersection
    Region operator&( const Region& rhs );
    Region& operator|=( const Region& rhs ); // union
    Region operator|( const Region& rhs );
    Region& operator-= ( const Region& rhs ); // difference
    Region operator-( const Region& rhs );
    Region& operator^=( const Region& rhs ); // symetric difference
    Region operator^( const Region& rhs );

    // warning: this pointer can change
    inline PlatformRegion asPlatformRegion() const  { 
      return platformRegion_;
    }

  private:

    PlatformRegion platformRegion_;
    mutable Rect clipBox;
    mutable bool update;

  };

}

#endif
