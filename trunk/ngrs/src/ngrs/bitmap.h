/***************************************************************************
 *   Copyright (C) 2005, 2006, 2007 by  Stefan Nattkemper  *
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
#ifndef BITMAP_H
#define BITMAP_H

#include "object.h"
#include "system.h"

/**
@author  Stefan
*/

#ifdef __unix__
typedef XImage* PlatformImageHandle;
#else
typedef HBITMAP PlatformImageHandle;
#endif


namespace ngrs {

  class Bitmap : public Object
  {
  public:
    Bitmap();
    Bitmap( const std::string& filename );
    Bitmap( const Bitmap& rhs );
    Bitmap( const char** data );

    Bitmap& operator=( const Bitmap& rhs );

    ~Bitmap();

    int depth()     const;
    int pixelsize() const;
    int width()     const;
    int height()    const;

    void setDepth( int depth );

    void loadFromFile( const std::string& filename );
    void createFromXpmData( const char** data );

    bool empty() const;


    void setSysImgData( PlatformImageHandle data, PlatformImageHandle clp );
    PlatformImageHandle cloneSysImage( PlatformImageHandle src_img );
    
    PlatformImageHandle sysData() const;
    PlatformImageHandle clpData() const; 

  private:

    int depth_;

    PlatformImageHandle sysData_;
    PlatformImageHandle clpData_;

    long clpColor_;

#ifdef __unix__
#else 
    HBITMAP createClipMask(HBITMAP hbmColour, COLORREF crTransparent);
#endif

    void deleteBitmapData();

  };

}

#endif
