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
#ifndef NBORDER_H
#define NBORDER_H

#include "size.h"
#include "graphics.h"
#include "shape.h"
#include "color.h"

/**
@author  Stefan
*/

namespace ngrs {

  class Border{
  public:
    Border();

    virtual ~Border();

    virtual void paint( Graphics& g, const Shape & geometry );
    void setSpacing( Size size );

    const Size & spacing() const;

    void setColor( const Color & color );
    const Color & color();

    virtual Border* clone()  const = 0;   // Uses the copy constructor

  private:

    Color color_;
    Size sizing_;

  };

}

#endif
