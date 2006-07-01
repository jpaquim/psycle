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
#ifndef NPEN_H
#define NPEN_H

#include <X11/Xlib.h>

enum LineStyle {nLineSolid          = LineSolid,
                nLineOnOffDash      = LineOnOffDash,
                nLineLineDoubleDash = LineDoubleDash };

/**
@author Stefan Nattkemper
*/

class NPen{
public:
    NPen();

    ~NPen();

    void setLineWidth(int width);
    int lineWidth() const;

    void setLineStyle(LineStyle style);
    LineStyle lineStyle() const;

private:

    int lineWidth_;
    LineStyle style_;

    XGCValues gcv;

};

#endif
