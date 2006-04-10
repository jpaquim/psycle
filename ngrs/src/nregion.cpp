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
#include "nregion.h"

NRegion::NRegion()
{
  region_ = XCreateRegion();
  update = true;
}

NRegion::NRegion( const NRect & rect )
{
  region_ = XCreateRegion();
  XRectangle rectangle;
  rectangle.x= (short) rect.left();
  rectangle.y= (short) rect.top();
  rectangle.width=(unsigned short)  rect.width();
  rectangle.height=(unsigned short) rect.height();
  XUnionRectWithRegion(&rectangle,region_,region_);
  update = true;
}

NRegion::~NRegion()
{
  XDestroyRegion(region_);
}

void NRegion::setRect( const NRect & rect )
{
  XDestroyRegion(region_);
  region_ = XCreateRegion();
  XRectangle rectangle;
  rectangle.x= (short) rect.left();
  rectangle.y= (short) rect.top();
  rectangle.width=(unsigned short)  rect.width();
  rectangle.height=(unsigned short) rect.height();
  XUnionRectWithRegion(&rectangle,region_,region_);
  update = true;
}


// shouldnt be XPoint
void NRegion::setPolygon(XPoint*  pts , int size)
{
  XDestroyRegion(region_);
  region_ = XPolygonRegion(pts,4,WindingRule);
  update = true;
}


NRegion::NRegion( const NRegion & src )
{
  region_ = XCreateRegion();
  XUnionRegion(region_, src.xRegion(), region_);
  update = true;
}

const NRegion & NRegion::operator =( const NRegion & rhs )
{
  XDestroyRegion(region_);
  region_ = XCreateRegion();
  XUnionRegion(region_, rhs.xRegion(), region_);
  update = true;
  return *this;
}


bool NRegion::isEmpty( ) const
{
  return XEmptyRegion(region_);
}

void NRegion::move( int dx, int dy )
{
  XOffsetRegion(region_, dx, dy);
  update = true;
}

void NRegion::shrink( int dx, int dy )
{
  XShrinkRegion(region_, dx, dy);
  update = true;
}

const NRect & NRegion::rectClipBox( ) const
{
  if (update) {
    XRectangle r;
    XClipBox(region_, &r);
    clipBox.setPosition(r.x,r.y,r.width,r.height);
    update = false;
  }
  return clipBox;
}
