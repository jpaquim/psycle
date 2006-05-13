/***************************************************************************
 *   Copyright (C) 2005 by Stefan   *
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


NImage::NImage()
 : NPanel()
{
  setEvents(false);
  halign_ = nAlLeft;
  pBitmap_ = 0;
}

NImage::NImage( std::string fileName ) : NPanel()
{
  bitmap24bpp_ = NApp::filter.at(0)->loadFromFile(fileName);
  setEvents(false);
  halign_ = nAlLeft;
  valign_ = nAlLeft;
  pBitmap_ = 0;
}

// the class factories

extern "C" NObject* createImage() {
    return new NImage();
}

extern "C" void destroyImage(NImage* p) {
    delete p;
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

  if (pBmp->X11data()!=0) {

    switch (halign_) {
      case nAlCenter :
        xp =(int) d2i((spacingWidth() - pBmp->width()) / 2.0f);
      break;
    }
    switch (valign_) {
      case nAlCenter :
        yp = (int) d2i((spacingHeight() - pBmp->height()) / 2.0f);
      break;
    }

    g->putBitmap(xp,yp,*pBmp);
  }
}

void NImage::loadFromFile( std::string filename )
{
  bitmap24bpp_ = NApp::filter.at(0)->loadFromFile(filename);
  setHeight(bitmap24bpp_.height());
  setWidth(bitmap24bpp_.width());
}

int NImage::preferredWidth( ) const
{
  if (ownerSize()) return NVisualComponent::preferredWidth();

  if (pBitmap_!=0) return pBitmap_->width()+1;
  return bitmap24bpp_.width()+1;
}

int NImage::preferredHeight( ) const
{
  if (ownerSize()) return NVisualComponent::preferredHeight();

  if (pBitmap_!=0) return pBitmap_->height()+1;
  return bitmap24bpp_.height()+1;
}

void NImage::setHAlign( int align )
{
  halign_ = align;
}

void NImage::createFromXpmData(const char** data)
{
  XpmColorSymbol cs[256];
  XpmAttributes attr;
  attr.valuemask = XpmCloseness;
  attr.colorsymbols = cs;
  attr.numsymbols = 256;
  attr.color_key = XPM_GRAY;

  XImage* xi;
  XImage* clp;
  int err = XpmCreateImageFromData(NApp::system().dpy(),(char**)(data),&xi,&clp,0);
  if (err == XpmSuccess) {
     bitmap24bpp_.setX11Data(xi,clp);
  }
}

void NImage::setBitmap( const NBitmap & bitmap )
{
  bitmap24bpp_ = bitmap;
}

void NImage::setVAlign( int align )
{
  valign_ = align;
}

void NImage::setSharedBitmap( NBitmap * bitmap )
{
  pBitmap_ = bitmap;
}



