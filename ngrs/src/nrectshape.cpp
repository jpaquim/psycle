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
#include "nrectshape.h"

NRectShape::NRectShape()
 : NShape()
{
  region_ = 0;
}


NRectShape::~NRectShape()
{
}

void NRectShape::fill( NGraphics * g, const NRect & repaintArea )
{
  g->fillRect(rectArea().left(),rectArea().top(),rectArea().width(),rectArea().height());
}

void NRectShape::drawPicker( NGraphics * g )
{
}

NPoint NRectShape::pickerAt( int i )
{
  switch (i) {
    case 0 :
       return NPoint(rectArea().left(), rectArea().top());
    break;
    case 1 :
       return NPoint(rectArea().left()+rectArea().width(), rectArea().top());
    break;
    case 2 :
       return NPoint(rectArea().left()+rectArea().width(), rectArea().top()+rectArea().height());
    break;
    case 3 :
       return NPoint(rectArea().left(), rectArea().top()+rectArea().height());
    break;
    default:;

  }
  return NPoint(-1,1);
}

int NRectShape::pickerSize( )
{
  return 4;
}

Region NRectShape::region( )
{
  region_ = XCreateRegion();
  rectangle.x= (short) rectArea().left();
  rectangle.y= (short) rectArea().top();
  rectangle.width=(unsigned short)  rectArea().width();
  rectangle.height=(unsigned short) rectArea().height();
  XUnionRectWithRegion(&rectangle,region_,region_);
  return region_;
}

Region NRectShape::spacingRegion( const NSize & spacing )
{
  spacingRegion_ = XCreateRegion();
  rectangle.x= (short) rectArea().left()+spacing.left();
  rectangle.y= (short) rectArea().top()+spacing.top();
  rectangle.width=(unsigned short)  rectArea().width()  - (spacing.left()+spacing.right());
  rectangle.height=(unsigned short) rectArea().height() - (spacing.top()+spacing.bottom());
  XUnionRectWithRegion(&rectangle,spacingRegion_,spacingRegion_);
  return spacingRegion_;
}

void NRectShape::destroyRegion( )
{
  if (region_!=0) XDestroyRegion(region_);
  region_ = 0;
}

void NRectShape::destroySpacingRegion( )
{
  if (spacingRegion_!=0) XDestroyRegion(spacingRegion_);
  spacingRegion_ = 0;
}



