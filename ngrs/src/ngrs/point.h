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
#ifndef POINT_H
#define POINT_H

#ifdef __unix__
  #include <X11/Xlib.h>
#endif

/**
@author  Stefan
*/

namespace ngrs {

  class NPoint{
  public:
    NPoint();
    NPoint(int x, int y);

    ~NPoint();

    void setX(int x);
    void setY(int y);
    void setXY(int x, int y);

    int x() const;
    int y() const;

#ifdef __unix__

    // implicit conversion to X const XPoint.
    inline operator const XPoint () const { XPoint pts; pts.x = x_; pts.y = y_; return pts; }

    // implicit conversion to X Region.
    inline operator XPoint ()  { XPoint pts; pts.x = x_; pts.y = y_; return pts; }


#endif

  private:

    int x_, y_;

  };

}

#endif
