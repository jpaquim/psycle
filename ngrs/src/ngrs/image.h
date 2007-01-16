/***************************************************************************
 *   Copyright (C) 2005, 2006, 2007 by Stefan Nattkemper   *
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
#ifndef NIMAGE_H
#define NIMAGE_H

#include "panel.h"
#include "bitmap.h"
#include "imgfilter.h"

/**
@author Stefan
*/

namespace ngrs {

  const int nAlWallPaper = 10;

  class Image : public Panel
  {
  public:
    Image();
    Image( const std::string & fileName );
    Image( const Bitmap & bitmap );

    ~Image();

    virtual void paint(Graphics& g);

    void loadFromFile( const std::string & filename);
    void createFromXpmData(const char** data);

    void setBitmap(const Bitmap & bitmap);

    void setSharedBitmap(Bitmap* bitmap);

    void setHAlign( int align );
    int hAlign() const;

    void setVAlign( int align );
    int vAlign() const;

    virtual int preferredWidth()  const;
    virtual int preferredHeight() const;


  private:

    Bitmap bitmap24bpp_;
    Bitmap* pBitmap_;

    int halign_;
    int valign_;

  };

}

#endif
