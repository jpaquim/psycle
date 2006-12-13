/***************************************************************************
 *   Copyright (C) 2005, 2006 by Stefan Nattkemper  *
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
#ifndef NBITMAP_H
#define NBITMAP_H

#include "nobject.h"
#include "nsystem.h"

/**
@author Stefan
*/

#ifdef __unix__
typedef XImage* NSysImage;
#else
typedef HBITMAP NSysImage;
#endif

class NBitmap : public NObject
{
public:
    NBitmap();
    NBitmap( const std::string & filename );
    NBitmap( const NBitmap & rhs );
    NBitmap( const char** data );

    const NBitmap & operator= ( const NBitmap & rhs );

    ~NBitmap();

    int depth()     const;
    int pixelsize() const;
    int width()     const;
    int height()    const;

    void setDepth(int depth);

    NSysImage sysData() const;
    NSysImage clpData() const; 

    void loadFromFile( const std::string & filename );
    void createFromXpmData( const char** data );


    NSysImage cloneSysImage( NSysImage src_img );

    bool empty() const;
    
    void setSysImgData( NSysImage data, NSysImage clp );

private:

    int depth_;

    NSysImage sysData_;
    NSysImage clpData_;

    long clpColor_;
        
    #ifdef __unix__
    #else 
    HBITMAP createClipMask(HBITMAP hbmColour, COLORREF crTransparent);
    #endif

	void deleteBitmapData();
		
};

#endif
