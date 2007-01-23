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
#include "shape.h"

namespace ngrs {

  Shape::Shape()
  {
  }

  Shape::~Shape()
  {
  }

  void Shape::setPosition( int left, int top, int width, int height )
  {
    rectArea_.setPosition(left,top,width,height);
  }

  const Rect & Shape::rectArea( ) const
  {
    return rectArea_;
  }

  void Shape::fill( Graphics& g, const ngrs::Region & repaintArea )
  {
  }

  int Shape::top( ) const
  {
    return rectArea().top();
  }

  int Shape::left( ) const
  {
    return rectArea().left();
  }

  int Shape::width( ) const
  {
    return rectArea().width();
  }

  int Shape::height( ) const
  {
    return rectArea().height();
  }

  void Shape::setTop( int top )
  {
    rectArea_.setTop(top);
  }

  void Shape::setWidth( int width )
  {
    rectArea_.setWidth(width);
  }

  void Shape::setHeight( int height )
  {
    rectArea_.setHeight(height);
  }

  void Shape::setLeft( int left )
  {
    rectArea_.setLeft(left);
  }

  Point Shape::pickerAt( int i )
  {
    return Point(-1,-1);
  }

  int Shape::pickerSize( )
  {
    return 0;
  }

  void Shape::drawPicker( Graphics& g )
  {
  }

  void Shape::drawRectPicker( Graphics& g )
  {
    int pickWidth  = 4;
    int pickHeight = 4;

    //if (((Window*)ownerWindow())->selected()!=this) g.setForeground(Color::gray); else
    g.setForeground(Color(0,0,0));
    /*if (moveable()->topLeft())   */  g.fillRect(left(),top(),pickWidth,pickHeight);
    /*if (moveable()->topRight())  */  g.fillRect(left()+width()-pickWidth,top(),pickWidth,pickHeight);
    /*if (moveable()->bottomLeft())*/  g.fillRect(left(),top()+height()-pickHeight,pickWidth,pickHeight);
    /*if (moveable()->bottomRight())*/ g.fillRect(left()+width()-pickWidth,top()+height()-pickHeight,pickWidth,pickHeight);
    /*if (moveable()->midLeft())*/     g.fillRect(left(),top()+height() / 2 - pickHeight / 2,pickWidth,pickHeight);
    /*if (moveable()->midRight())*/    g.fillRect(left()+width()-pickWidth,top()+ height() / 2 - pickHeight / 2,pickWidth,pickHeight);
    /*if (moveable()->topMid())*/      g.fillRect(left()+width() / 2 - pickWidth / 2 ,top(),pickWidth,pickHeight);
    /*if (moveable()->bottomMid())*/   g.fillRect(left()+width() / 2 - pickWidth / 2 ,top()+height()-pickHeight,pickWidth,pickHeight);
  }

  int Shape::overPicker( int x, int y )
  {
    return -1;
  }


  void Shape::setPicker( int index, int x, int y )
  {
  }

  ngrs::Region Shape::region( )
  {
    return ngrs::Region();
  }

  ngrs::Region Shape::spacingRegion(const Size & spacing )
  {
    return ngrs::Region();
  }

}
