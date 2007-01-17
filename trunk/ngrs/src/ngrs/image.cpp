/***************************************************************************
 *   Copyright (C) 2005, 2006, 2007 by  Stefan Nattkemper   *
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
#include "image.h"
#include "xpmfilter.h"
#include "app.h"

namespace ngrs {

  Image::Image()
    : Panel()
  {
    setEvents(false);
    halign_ = nAlLeft;
    valign_ = nAlLeft;
    pBitmap_ = 0;
  }

  Image::Image( const std::string & fileName ) : Panel()
  {
    bitmap24bpp_ = App::filter.at(0)->loadFromFile(fileName);
    setEvents(false);
    halign_ = nAlLeft;
    valign_ = nAlLeft;
    pBitmap_ = 0;
  }

  Image::Image( const Bitmap & bitmap )
  {
    bitmap24bpp_ = bitmap;
    setEvents(false);
    halign_ = nAlLeft;
    valign_ = nAlLeft;
    pBitmap_ = 0;
  }

  Image::~Image()
  {
  }

  void Image::paint( Graphics& g )
  {
    int xp = 0;
    int yp = 0;

    Bitmap* pBmp = 0;
    if (pBitmap_ == 0) pBmp =  &bitmap24bpp_; else pBmp = pBitmap_;

    if ( pBmp->sysData() )
    {
      if ( halign_ == nAlWallPaper && valign_ == nAlWallPaper ) {	  
        for (int yp = 0; yp < spacingHeight(); yp += pBmp->height() ) {
          for (int xp = 0; xp < spacingWidth(); xp += pBmp->width() ) {
            g.putBitmap( xp, yp, *pBmp );
          }
        }
        return;
      }

      switch ( halign_ ) {
      case nAlCenter :
        xp =(int) d2i((spacingWidth() - pBmp->width()) / 2.0f);
        break;
      }

      switch ( valign_ ) {
      case nAlCenter :
        yp = (int) d2i((spacingHeight() - pBmp->height()) / 2.0f);
        break;
      }

      if ( halign_ == nAlWallPaper ) {
        for (int xp = 0; xp < spacingWidth(); xp += pBmp->width() ) {
          g.putBitmap( xp, yp, *pBmp );
        }
      } else
        if ( valign_ == nAlWallPaper ) {
          for (int yp = 0; yp < spacingHeight(); yp += pBmp->height() ) {
            g.putBitmap( left()+xp, top()+yp, *pBmp );
          }
        } else
          g.putBitmap( xp, yp, *pBmp );	  
    }

  }

  void Image::loadFromFile( const std::string & filename )
  {
    bitmap24bpp_ = App::filter.at( 0 )->loadFromFile( filename );
    setHeight( bitmap24bpp_.height() );
    setWidth( bitmap24bpp_.width() );
  }

  int Image::preferredWidth( ) const
  {
    if (ownerSize()) return VisualComponent::preferredWidth();
    return pBitmap_ ? pBitmap_->width()+1 : bitmap24bpp_.width()+1;
  }

  int Image::preferredHeight( ) const
  {
    if (ownerSize()) return VisualComponent::preferredHeight();
    return pBitmap_ ? pBitmap_->height()+1 : bitmap24bpp_.height()+1;
  }

  void Image::setHAlign( int align )
  {
    halign_ = align;
  }

  int Image::hAlign() const {
    return halign_;
  }

  void Image::setVAlign( int align )
  {
    valign_ = align;
  }

  int Image::vAlign() const {
    return valign_;
  }

  void Image::createFromXpmData( const char** data )
  {
    bitmap24bpp_.createFromXpmData(data);  
  }

  void Image::setBitmap( const Bitmap& bitmap )
  {
    bitmap24bpp_ = bitmap;
  }

  void Image::setSharedBitmap( Bitmap* bitmap )
  {
    pBitmap_ = bitmap;
  }

}

// the class factories

extern "C" ngrs::Object* createImage() {
  return new ngrs::Image();
}

extern "C" void destroyImage( ngrs::Image* p ) {
  delete p;
}
