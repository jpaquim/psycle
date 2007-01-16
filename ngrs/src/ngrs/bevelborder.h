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
#ifndef NBEVELBORDER_H
#define NBEVELBORDER_H

#include "border.h"

namespace ngrs {

  const int nRaised  = 0;
  const int nLowered   = 1;
  const int nNone      =3;


  /**
  @author Stefan
  */
  class BevelBorder : public Border
  {
  public:
    BevelBorder();
    BevelBorder(int outerStyle, int innerStyle);

    ~BevelBorder();

    virtual void paint(Graphics& g, const Shape & geometry);

    void setInnerStyle(int style);
    void setOuterStyle(int style);
    void setStyle(int outerStyle, int innerStyle, int size);

    int innerStyle();
    int outerStyle();

    virtual BevelBorder* clone()  const;   // Uses the copy constructor

  private:

    int style_;
    int innerStyle_;
    int outerStyle_;
    bool oval_;

    void mapStyle();
    void drawRectBorder(Graphics& g, int width, int height);


  };

}

#endif
