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
 
#ifdef __unix__ 
 
#include "pixmap.h"
#include "app.h"
#include "file.h"
#include "window.h"
#include <X11/xpm.h>

namespace ngrs {

  Pixmap::Pixmap( )
  {
    pixmap_ = 0;
    shapepixmap_ = 0;
    height_ = width_ = 0;
    owner_ = App::system().rootWindow();
  }

  Pixmap::Pixmap(Window* win)
    : Object()
  {
#ifdef __unix__
    pixmap_ = 0;
    shapepixmap_ = 0;
#endif
    height_ = width_ = 0;
    owner_ = win->win();
  }


  Pixmap::~Pixmap()
  {
    if (pixmap_) {
      XFreePixmap(App::system().dpy(), pixmap_);
    }
    if (shapepixmap_) {
      XFreePixmap(App::system().dpy(), shapepixmap_);
    }
  }

  int Pixmap::width( ) const
  {
    return width_;
  }

  int Pixmap::height( ) const
  {
    return height_;
  }


  void Pixmap::loadFromFile( const std::string & filename )
  {
    XpmColorSymbol cs[256];
    XpmAttributes attr;
    attr.valuemask = XpmCloseness;
    attr.colorsymbols = cs;
    attr.numsymbols = 256;
    attr.color_key = XPM_GRAY;


    int err = XpmReadFileToPixmap(App::system().dpy(), owner_,
      (char*) (ngrs::File::replaceTilde(filename).c_str()),
      &pixmap_,&shapepixmap_,&attr);

    if (err != XpmSuccess) throw "couldn`t open file";

    width_  = attr.width;
    height_ = attr.height;

  }

  Pixmap Pixmap::X11Pixmap( ) const
  {
    return pixmap_;
  }

  Pixmap Pixmap::X11ShapePixmap( ) const
  {
    return shapepixmap_;
  }

  WinHandle Pixmap::owner( ) const
  {
    return owner_;
  }

  void Pixmap::createFromXpmData( const char ** data )
  {
    XpmColorSymbol cs[256];
    XpmAttributes attr;
    attr.valuemask = XpmCloseness;
    attr.colorsymbols = cs;
    attr.numsymbols = 256;
    attr.color_key = XPM_GRAY;


    int err = XpmCreatePixmapFromData(App::system().dpy(), owner_,
      (char**)data,
      &pixmap_,&shapepixmap_,&attr);

    width_  = attr.width;
    height_ = attr.height;
  }

  bool Pixmap::empty( ) const
  {
    return !( pixmap_ || shapepixmap_ );
  }

}

#endif
