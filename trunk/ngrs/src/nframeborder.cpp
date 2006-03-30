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
#include "nframeborder.h"


NFrameBorder::NFrameBorder()
 : NBorder()
{
  setSpacing(NSize(1,1,1,1));
  color = NColor(150,150,150);
  lcount_ = 1;
  dx_ = 2; dy_ = 2;
  arcWidth_  = 5;
  arcHeight_ = 5;
  oval_ = false;
}


NFrameBorder::~NFrameBorder()
{
}

void NFrameBorder::paint( NGraphics * g, const NShape & geometry )
{
  g->setForeground(color);
  NRect r = geometry.rectArea();
  for (int i = 0; i < lcount_; i++) {
    if (oval_) 
      g->drawRoundRect(r.left()+i*dx_,r.top()+i*dy_,r.width()-1-2*(i*dx_),r.height()-1-2*(i*dy_),arcWidth_,arcHeight_);
    else
      g->drawRect(r.left()+i*dx_,r.top()+i*dy_,r.width()-1-2*(i*dx_),r.height()-1-2*(i*dy_));
  }
}

void NFrameBorder::setOval( bool on, int arcWidth, int arcHeight )
{
  oval_ = on;
  arcWidth_  = arcWidth;
  arcHeight_ = arcHeight;
}

void NFrameBorder::setLineCount( int count, int dWidth, int dHeight )
{
  dx_ = dWidth;
  dy_ = dHeight;
  lcount_ = count;
  setSpacing(NSize( lcount_*dx_,lcount_*dy_,lcount_*dx_,lcount_*dy_));
}


