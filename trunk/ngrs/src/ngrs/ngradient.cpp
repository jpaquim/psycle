/***************************************************************************
 *   Copyright (C) 2005 by Stefan   *
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
#include "ngradient.h"

NGradient::NGradient()
 : NPanel()
{
  active_ = true;
  horizontal_ = false;
  percent_ = 1;
  color_3 = NColor(255,0,0);
  color_2 = NColor(255,255,128);
  color_1 = NColor(128,128,255);
}


NGradient::~NGradient()
{
}

void NGradient::setActive( bool active )
{
  active_ = active;
}

bool NGradient::active( )
{
  return active_;
}

void NGradient::setColor_1( NColor color )
{
  color_1 = color;
}

void NGradient::setColor_2( NColor color )
{
  color_2 = color;
}

void NGradient::setColor_3( NColor color )
{
  color_3 = color;
}

void NGradient::setHorizontal( bool on )
{
  horizontal_ = on;
}

void NGradient::setPercent( int percent )
{
  if (percent < 1  ) percent_ = 0.01; else
  if (percent > 99 ) percent_ = 1; else
  percent_ = percent / 100.0f;
}

void NGradient::do_gradient(NGraphics* g )
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
  // NRect result;
  // g->repaintArea().interSects(absoluteSpacingGeometry(),result);
  // result.setLeft(result.left()-g->xTranslation());  
  // result.setTop(result.top()-g->yTranslation());

  // frist part to middle

  if (horizontal_)
    g->fillGradient(0,0,middle,clientHeight(), color_1,color_2,nHorizontal);
  else 
    g->fillGradient(0,0,clientWidth(),middle,color_1,color_2,nVertical);

  // second part from middle to end

  if (horizontal_)
    g->fillGradient(middle,0,clientWidth()-middle,clientHeight(),
                    color_1,color_2,nHorizontal);
  else 
    g->fillGradient(0,middle,clientWidth(),clientHeight()-middle,
                    color_1,color_2,nVertical);


}

void NGradient::paint( NGraphics * g )
{
  if (active_) do_gradient(g);
}


