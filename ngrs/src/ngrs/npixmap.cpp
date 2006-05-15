/***************************************************************************
 *   Copyright (C) 2006 by Stefan Nattkemper   *
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
#include "npixmap.h"
#include "napp.h"
#include <X11/xpm.h>
#include <nfile.h>
#include <nwindow.h>

NPixmap::NPixmap( )
{
  pixmap_ = 0;
  shapepixmap_ = 0;
  height_ = width_ = 0;
  owner_ = NApp::system().rootWindow();
}

NPixmap::NPixmap(NWindow* win)
 : NObject()
{
  pixmap_ = 0;
  shapepixmap_ = 0;
  height_ = width_ = 0;
  owner_ = win->win();
}


NPixmap::~NPixmap()
{
  if (pixmap_) {
     XFreePixmap(NApp::system().dpy(), pixmap_);
  }
  if (shapepixmap_) {
     XFreePixmap(NApp::system().dpy(), shapepixmap_);
  }
}

int NPixmap::width( ) const
{
  return width_;
}

int NPixmap::height( ) const
{
  return height_;
}

void NPixmap::loadFromFile( const std::string & filename )
{
  XpmColorSymbol cs[256];
  XpmAttributes attr;
  attr.valuemask = XpmCloseness;
  attr.colorsymbols = cs;
  attr.numsymbols = 256;
  attr.color_key = XPM_GRAY;


  int err = XpmReadFileToPixmap(NApp::system().dpy(), owner_,
                                (char*) (NFile::replaceTilde(filename).c_str()),
                                &pixmap_,&shapepixmap_,&attr);

  width_  = attr.width;
  height_ = attr.height;
}

Pixmap NPixmap::X11Pixmap( ) const
{
  return pixmap_;
}

Pixmap NPixmap::X11ShapePixmap( ) const
{
  return shapepixmap_;
}

Window NPixmap::owner( ) const
{
  return owner_;
}

void NPixmap::createFromXpmData( const char ** data )
{
  XpmColorSymbol cs[256];
  XpmAttributes attr;
  attr.valuemask = XpmCloseness;
  attr.colorsymbols = cs;
  attr.numsymbols = 256;
  attr.color_key = XPM_GRAY;


  int err = XpmCreatePixmapFromData(NApp::system().dpy(), owner_,
                                   (char**)data,
                                &pixmap_,&shapepixmap_,&attr);

  width_  = attr.width;
  height_ = attr.height;
}





