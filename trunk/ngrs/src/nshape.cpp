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
#include "nshape.h"

NShape::NShape()
{

}


NShape::~NShape()
{
}

void NShape::setPosition( int left, int top, int width, int height )
{
  rectArea_.setPosition(left,top,width,height);
}

const NRect & NShape::rectArea( ) const
{
  return rectArea_;
}

void NShape::fill( NGraphics * g, const NRect & repaintArea )
{
}

int NShape::top( ) const
{
  return rectArea().top();
}

int NShape::left( ) const
{
  return rectArea().left();
}

int NShape::width( ) const
{
  return rectArea().width();
}

int NShape::height( ) const
{
  return rectArea().height();
}

void NShape::setTop( int top )
{
  rectArea_.setTop(top);
}

void NShape::setWidth( int width )
{
  rectArea_.setWidth(width);
}

void NShape::setHeight( int height )
{
  rectArea_.setHeight(height);
}

void NShape::setLeft( int left )
{
  rectArea_.setLeft(left);
}

NPoint NShape::pickerAt( int i )
{
  return NPoint(-1,-1);
}

int NShape::pickerSize( )
{
  return 0;
}

void NShape::drawPicker( NGraphics * g )
{
}

void NShape::drawRectPicker( NGraphics * g )
{
 int pickWidth  = 4;
 int pickHeight = 4;

 //if (((NWindow*)ownerWindow())->selected()!=this) g->setForeground(NColor::gray); else
                                    g->setForeground(NColor(0,0,0));
 /*if (moveable()->topLeft())   */  g->fillRect(left(),top(),pickWidth,pickHeight);
 /*if (moveable()->topRight())  */  g->fillRect(left()+width()-pickWidth,top(),pickWidth,pickHeight);
 /*if (moveable()->bottomLeft())*/  g->fillRect(left(),top()+height()-pickHeight,pickWidth,pickHeight);
 /*if (moveable()->bottomRight())*/ g->fillRect(left()+width()-pickWidth,top()+height()-pickHeight,pickWidth,pickHeight);
 /*if (moveable()->midLeft())*/     g->fillRect(left(),top()+height() / 2 - pickHeight / 2,pickWidth,pickHeight);
 /*if (moveable()->midRight())*/    g->fillRect(left()+width()-pickWidth,top()+ height() / 2 - pickHeight / 2,pickWidth,pickHeight);
 /*if (moveable()->topMid())*/      g->fillRect(left()+width() / 2 - pickWidth / 2 ,top(),pickWidth,pickHeight);
 /*if (moveable()->bottomMid())*/   g->fillRect(left()+width() / 2 - pickWidth / 2 ,top()+height()-pickHeight,pickWidth,pickHeight);
}

int NShape::overPicker( int x, int y )
{
  return -1;
}


void NShape::setPicker( int index, int x, int y )
{
}

Region NShape::region( )
{
  return 0;
}

Region NShape::spacingRegion(const NSize & spacing )
{
  return 0;
}

void NShape::destroyRegion( )
{

}

void NShape::destroySpacingRegion( )
{

}




