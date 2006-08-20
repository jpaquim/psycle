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
#include "nbitmap.h"
#include "napp.h"
#include "nfile.h"

NBitmap::NBitmap()
 : NObject(), depth_(24),width_(0),height_(0), data_(0), xi(0), clp(0)
{

}

NBitmap::NBitmap( const std::string & filename ) : NObject(), depth_(24),width_(0),height_(0),data_(0),xi(0), clp(0)
{
  loadFromFile(filename);
}

NBitmap::NBitmap( const char ** data ) : NObject(), depth_(24),width_(0),height_(0),data_(0),xi(0), clp(0)
{
  createFromXpmData(data);
}


NBitmap::~NBitmap()
{
	if (xi)
		XDestroyImage(xi);
  if (clp)
		XDestroyImage(clp);
}

unsigned char const * NBitmap::dataPtr( ) const
{
  return (unsigned char*) xi->data;
}

void NBitmap::setDepth( int depth )
{
  depth_ = depth;
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

NBitmap::NBitmap( const NBitmap & rhs ) : depth_(24),width_(0),height_(0),data_(0),xi(0),clp(0)
{
	if (rhs.X11data() == 0) {
		// empty Bitmap
		// do just nothing
	} else {
		xi = cloneXImage( rhs.X11data() );
	}

	if (rhs.X11ClpData() == 0) {
		// empty Bitmap
		// do just nothing
	} else {
		clp = cloneXImage( rhs.X11ClpData() );
	}

}

void NBitmap::setX11Data( XImage * ximage, XImage* clp_ )
{
  xi = ximage;
  clp = clp_;
  //data_ = (unsigned char*) xi->data;
}

const NBitmap & NBitmap::operator =( const NBitmap & rhs )
{
	deleteBitmapData();

	if (rhs.X11data() == 0) {
		// empty Bitmap
		// do just nothing
	} else {
		xi = cloneXImage( rhs.X11data() );
	}

	if (rhs.X11ClpData() == 0) {
		// empty Bitmap
		// do just nothing
	} else {
		clp = cloneXImage( rhs.X11ClpData() );
	}
	
  return *this;
}

void NBitmap::loadFromFile( const std::string & filename )
{
	deleteBitmapData();

  try {
    NBitmap bmp1 = NApp::filter.at(0)->loadFromFile(NFile::replaceTilde(filename));
    xi  = cloneXImage( bmp1.X11data() );
    clp = cloneXImage( bmp1.X11ClpData() );
  } catch (const char* e) {
     throw "couldn`t open file";
  }
}


void NBitmap::createFromXpmData(const char** data)
{
	deleteBitmapData();

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

XImage * NBitmap::cloneXImage( XImage * src_xi )
{
  if ( src_xi ) {
		int width  = src_xi->width;
		int height = src_xi->height;
		int depth  = src_xi->depth;
		int pad    = src_xi->bitmap_pad;
		int bytes_per_line = src_xi->bytes_per_line;

		int pixelsize = NApp::system().pixelSize( depth );

		unsigned char* src_data = reinterpret_cast<unsigned char*> (src_xi->data);
		unsigned char* dst_data = 0;

		dst_data = reinterpret_cast<unsigned char*> ( malloc( width * height * pixelsize ) );
		memcpy( dst_data, src_data, width * height * pixelsize );				

  	XImage* dst_xi = XCreateImage(NApp::system().dpy(), NApp::system().visual(), depth, ZPixmap,0,(char*) dst_data , width, height, pad , bytes_per_line );

		return dst_xi;
	}

  return 0;
}

void NBitmap::deleteBitmapData( )
{
  if (xi) {
		// destroys the image
		XDestroyImage(xi);
		xi = 0;
	}

	if (clp) {
		// destroys the transparent image mask
		XDestroyImage(clp);
		clp = 0;
	}
}



