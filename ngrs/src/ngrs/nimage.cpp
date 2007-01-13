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
#include "nimage.h"
#include "nxpmfilter.h"
#include "napp.h"

namespace ngrs {

  NImage::NImage()
    : NPanel()
  {
    setEvents(false);
    halign_ = nAlLeft;
    valign_ = nAlLeft;
    pBitmap_ = 0;
  }

  NImage::NImage( const std::string & fileName ) : NPanel()
  {
    bitmap24bpp_ = NApp::filter.at(0)->loadFromFile(fileName);
    setEvents(false);
    halign_ = nAlLeft;
    valign_ = nAlLeft;
    pBitmap_ = 0;
  }

  NImage::NImage( const NBitmap & bitmap )
  {
    bitmap24bpp_ = bitmap;
    setEvents(false);
    halign_ = nAlLeft;
    valign_ = nAlLeft;
    pBitmap_ = 0;
  }

  NImage::~NImage()
  {
  }

  void NImage::paint( NGraphics * g )
  {
    int xp = 0;
    int yp = 0;

    NBitmap* pBmp = 0;
    if (pBitmap_ == 0) pBmp =  &bitmap24bpp_; else pBmp = pBitmap_;

    if ( pBmp->sysData() )
    {
      if ( halign_ == nAlWallPaper && valign_ == nAlWallPaper ) {	  
        for (int yp = 0; yp < spacingHeight(); yp += pBmp->height() ) {
          for (int xp = 0; xp < spacingWidth(); xp += pBmp->width() ) {
            g->putBitmap( xp, yp, *pBmp );
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
          g->putBitmap( xp, yp, *pBmp );
        }
      } else
        if ( valign_ == nAlWallPaper ) {
          for (int yp = 0; yp < spacingHeight(); yp += pBmp->height() ) {
            g->putBitmap( left()+xp, top()+yp, *pBmp );
          }
        } else
          g->putBitmap( xp, yp, *pBmp );	  
    }

  }

  void NImage::loadFromFile( const std::string & filename )
  {
    bitmap24bpp_ = NApp::filter.at( 0 )->loadFromFile( filename );
    setHeight( bitmap24bpp_.height() );
    setWidth( bitmap24bpp_.width() );
  }

  int NImage::preferredWidth( ) const
  {
    if (ownerSize()) return NVisualComponent::preferredWidth();
    return pBitmap_ ? pBitmap_->width()+1 : bitmap24bpp_.width()+1;
  }

  int NImage::preferredHeight( ) const
  {
    if (ownerSize()) return NVisualComponent::preferredHeight();
    return pBitmap_ ? pBitmap_->height()+1 : bitmap24bpp_.height()+1;
  }

  void NImage::setHAlign( int align )
  {
    halign_ = align;
  }

  int NImage::hAlign() const {
    return halign_;
  }

  void NImage::setVAlign( int align )
  {
    valign_ = align;
  }

  int NImage::vAlign() const {
    return valign_;
  }

  void NImage::createFromXpmData( const char** data )
  {
    bitmap24bpp_.createFromXpmData(data);  
  }

  void NImage::setBitmap( const NBitmap& bitmap )
  {
    bitmap24bpp_ = bitmap;
  }

  void NImage::setSharedBitmap( NBitmap* bitmap )
  {
    pBitmap_ = bitmap;
  }

}

// the class factories

extern "C" ngrs::NObject* createImage() {
  return new ngrs::NImage();
}

extern "C" void destroyImage( ngrs::NImage* p ) {
  delete p;
}
