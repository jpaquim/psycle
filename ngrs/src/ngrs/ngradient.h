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
#ifndef NGRADIENT_H
#define NGRADIENT_H

#include "npanel.h"

/**
@author Stefan
*/

namespace ngrs {

  class NGradient : public NPanel
  {
  public:
    NGradient();

    ~NGradient();

    void setActive(bool active);
    bool active();

    void setColor_1(NColor color);
    void setColor_2(NColor color);
    void setColor_3(NColor color);

    void setHorizontal(bool on);
    void setPercent(int percent);

    void paint(NGraphics* g);

  private:

    bool active_;
    bool horizontal_;
    double percent_;
    int middle_;

    NColor color_1;
    NColor color_2;
    NColor color_3;

    void do_gradient(NGraphics* g);

  };

}

#endif
