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
#include "ngrs/nbitmap.h"
#include "ngrs/napp.h"
#include "ngrs/nfile.h"

NBitmap::NBitmap()
 : NObject(), depth_(24),width_(0),height_(0), data_(0), xi(0), clp(0)
{

}

NBitmap::NBitmap( const std::string & filename ) : NObject(), depth_(24),width_(0),height_(0),data_(0),xi(0), clp(0)
{
  loadFromFile(filename);
}

NBitmap::~NBitmap()
{
  /*delete[] data_;
  if (xi!=0) {
   xi->data = 0;
   XDestroyImage(xi);
  }*/
}

unsigned char const * NBitmap::dataPtr( ) const
{
  return (unsigned char*) xi->data;
}

void NBitmap::setDepth( int depth )
{
  depth_ = depth;
}

void NBitmap::setSize( int width, int height )
{
   xi = XCreateImage(NApp::system().dpy(), NApp::system().visual(), NApp::system().depth(), ZPixmap,0,(char*)data_,width,height,32,width*pixelsize());

  /*if (width>0 && height >0) {
    if (data_ == 0) {
       data_ = (pRGBA) new char[width*height*pixelsize()];
       xi = XCreateImage(NApp::system().dpy(), NApp::system().visual(), NApp::system().depth(), ZPixmap,0,(char*)data_,width,height,32,width*pixelsize());
    } else {
      pRGBA tmp_ = (pRGBA) new char[width*height*pixelsize()];
      xi = XCreateImage(NApp::system().dpy(), NApp::system().visual(), NApp::system().depth(), ZPixmap,0,(char*)data_,width,height,32,width*pixelsize());
      delete[] data_;
      data_ = tmp_;
    }
    width_  = width;
    height_ = height;
  }*/
}

int NBitmap::depth( ) const
{
  return (xi != 0) ? xi->depth : 0;
}

int NBitmap::width( ) const
{
  return (xi != 0) ? xi->width : 0;
}

int NBitmap::height( ) const
{
  return (xi != 0) ? xi->height : 0;
}

int NBitmap::pixelsize( ) const
{
 int pixelsize_;
 switch(depth())
      {
      case 8:
        pixelsize_=1;
        break;
      case 16:
        pixelsize_=2;
        break;
      case 24:
        pixelsize_=4;
        break;
      default:
        pixelsize_=1;
      }
  return pixelsize_;
}

XImage * NBitmap::X11data( ) const
{
  return xi;
}

NBitmap::NBitmap( const NBitmap & src ) : depth_(24),width_(0),height_(0),data_(0),xi(0),clp(0)
{
  xi = src.X11data();
  clp = src.X11ClpData();
   /*setSize(src.width(),src.height());
   for (int i = 0; i < width_ * height_ * src.pixelsize(); i++) {
      data_[i] = src.dataPtr()[i];
   }*/
}

void NBitmap::setX11Data( XImage * ximage, XImage* clp_ )
{
  xi = ximage;
  clp = clp_;
  //data_ = (unsigned char*) xi->data;
}

const NBitmap & NBitmap::operator =( const NBitmap & rhs )
{
   /*setSize(rhs.width(),rhs.height());
   for (int i = 0; i < width_ * height_*rhs.pixelsize(); i++) {
      data_[i] = rhs.dataPtr()[i];
   }
   if (xi!=0) {
      xi->data =(char*) data_;
   }*/
  clp = rhs.X11ClpData();
  xi = rhs.X11data();
  return *this;
}

void NBitmap::loadFromFile( std::string filename )
{
  NBitmap bmp1 = NApp::filter.at(0)->loadFromFile(NFile::replaceTilde(filename));
  xi = bmp1.X11data();
}


void NBitmap::createFromXpmData(const char** data)
{
  XpmColorSymbol cs[256];
  XpmAttributes attr;
  attr.valuemask = XpmCloseness;
  attr.colorsymbols = cs;
  attr.numsymbols = 256;
  attr.color_key = XPM_GRAY;

  XImage* xi1;
  int err = XpmCreateImageFromData(NApp::system().dpy(),(char**)(data),&xi1,&clp,0);
  if (err == XpmSuccess) {
     xi = xi1;
  }
}

XImage * NBitmap::X11ClpData( ) const
{
  return clp;
}


