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
#include "nframeborder.h"

namespace ngrs {

  NFrameBorder::NFrameBorder()
    : NBorder()
  {
    setSpacing(NSize(1,1,1,1));
    lcount_ = 1;
    dx_ = 2; dy_ = 2;
    arcWidth_  = 5;
    arcHeight_ = 5;
    oval_ = false;
  }

  NFrameBorder::NFrameBorder( bool oval, int arcWidth, int arcHeight )
  {
    setSpacing(NSize(1,1,1,1));
    lcount_ = 1;
    dx_ = 2; dy_ = 2;
    arcWidth_  = arcWidth;
    arcHeight_ = arcHeight;
    oval_ = oval;
  }

  NFrameBorder::~NFrameBorder()
  {
  }

  void NFrameBorder::paint( Graphics& g, const NShape & geometry )
  {
    g.setForeground(color());
    NRect r = geometry.rectArea();
    for (int i = 0; i < lcount_; i++) {
      if (oval_) 
        g.drawRoundRect(r.left()+i*dx_,r.top()+i*dy_,r.width()-1-2*(i*dx_),r.height()-1-2*(i*dy_),arcWidth_,arcHeight_);
      else
        g.drawRect(r.left()+i*dx_,r.top()+i*dy_,r.width()-1-2*(i*dx_),r.height()-1-2*(i*dy_));
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

  bool NFrameBorder::oval( ) const
  {
    return oval_;
  }

  int NFrameBorder::arcWidth( ) const
  {
    return arcWidth_;
  }

  int NFrameBorder::arcHeight( ) const
  {
    return arcHeight_;
  }

  int NFrameBorder::lineCount( ) const
  {
    return lcount_;
  }

  int NFrameBorder::dWidth( ) const
  {
    return dx_;
  }

  int NFrameBorder::dHeight( ) const
  {
    return dy_;
  }

  // usage of Covariant Return Types, a feature that was not originally part of c++. If your //compiler complains at the declaration of NFrameBorder* clone() const, you have an old compiler and you'll have to change the return type to NBorder*.

  NFrameBorder * NFrameBorder::clone( ) const
  {
    return new NFrameBorder(*this);
  }

}
