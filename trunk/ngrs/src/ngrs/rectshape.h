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
#ifndef NRECTSHAPE_H
#define NRECTSHAPE_H

#include "shape.h"

/**
@author  Stefan
*/

namespace ngrs {

  class RectShape : public Shape
  {
  public:
    RectShape();

    ~RectShape();

    virtual void fill(Graphics& g, const ngrs::Region & repaintArea);
    virtual void drawPicker(Graphics& g);

    virtual NPoint pickerAt(int i);
    virtual int pickerSize();

    virtual ngrs::Region region();
    virtual ngrs::Region spacingRegion(const Size & spacing);

    virtual RectShape* clone() const;   // Uses the copy constructor


  private:

#ifdef __unix__
    XRectangle rectangle;
#endif
  };

}

#endif
