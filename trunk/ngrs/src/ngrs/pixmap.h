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
#ifndef NPIXMAP_H
#define NPIXMAP_H

#include "object.h"
#include "system.h"

namespace ngrs {

#ifdef __unix__
#else
#include "bitmap.h"
  typedef Bitmap Pixmap;
#endif

#ifdef __unix__

  class Window;

  /**
  @author Stefan Nattkemper
  */
  class Pixmap : public Object
  {
  public:

    Pixmap();
    Pixmap(Window* win);

    ~Pixmap();

    int width()  const;
    int height() const;


    ::Pixmap X11Pixmap() const;
    ::Pixmap X11ShapePixmap() const;

    WinHandle owner() const;

    void loadFromFile( const std::string & filename );
    void createFromXpmData(const char** data);

    bool empty() const;


  private:

    int width_, height_;

    ::Pixmap pixmap_;
    ::Pixmap shapepixmap_;

    WinHandle owner_;
  };

#endif

}

#endif
