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
#include "gradient.h"

namespace ngrs {

  Gradient::Gradient()
    : Panel()
  {
    active_ = true;
    horizontal_ = false;
    percent_ = 1;
    color_3 = Color(255,0,0);
    color_2 = Color(255,255,128);
    color_1 = Color(128,128,255);
  }


  Gradient::~Gradient()
  {
  }

  void Gradient::setActive( bool active )
  {
    active_ = active;
  }

  bool Gradient::active( )
  {
    return active_;
  }

  void Gradient::setColor_1( Color color )
  {
    color_1 = color;
  }

  void Gradient::setColor_2( Color color )
  {
    color_2 = color;
  }

  void Gradient::setColor_3( Color color )
  {
    color_3 = color;
  }

  void Gradient::setHorizontal( bool on )
  {
    horizontal_ = on;
  }

  void Gradient::setPercent( int percent )
  {
    if (percent < 1  ) percent_ = 0.01; else
      if (percent > 99 ) percent_ = 1; else
        percent_ = percent / 100.0f;
  }

  void Gradient::do_gradient( Graphics& g )
  {
    int middle  = 0;
    int length  = 0;

    if (horizontal_) {
      middle = (int) (clientWidth()  * percent_);
      length = clientWidth()  - middle;
    }
    else {
      middle = (int) (clientHeight() * percent_);
      length = clientHeight() - middle;
    }


    // check for repaintArea
    // Rect result;
    // g->repaintArea().interSects(absoluteSpacingGeometry(),result);
    // result.setLeft(result.left()-g->xTranslation());  
    // result.setTop(result.top()-g->yTranslation());

    // frist part to middle

    if (horizontal_)
      g.fillGradient(0,0,middle,clientHeight(), color_1,color_2,nHorizontal);
    else 
      g.fillGradient(0,0,clientWidth(),middle,color_1,color_2,nVertical);

    // second part from middle to end

    if (horizontal_)
      g.fillGradient(middle,0,clientWidth()-middle,clientHeight(),
      color_1,color_2,nHorizontal);
    else 
      g.fillGradient(0,middle,clientWidth(),clientHeight()-middle,
      color_1,color_2,nVertical);


  }

  void Gradient::paint( Graphics& g )
  {
    if (active_) 
      do_gradient(g);
  }

}
