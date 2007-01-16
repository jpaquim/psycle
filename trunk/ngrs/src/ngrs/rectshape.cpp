/***************************************************************************
 *   Copyright (C) 2005, 2006, 2007 by Stefan Nattkemper  *
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
#include "rectshape.h"

namespace ngrs {

  RectShape::RectShape()
    : Shape()
  {
  }

  RectShape::~RectShape()
  {
  }

  void RectShape::fill( Graphics& g, const ngrs::Region & repaintArea )
  {
    g.fillRect(rectArea().left(),rectArea().top(),rectArea().width(),rectArea().height());
  }

  void RectShape::drawPicker( Graphics& g )
  {
  }

  NPoint RectShape::pickerAt( int i )
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

  int RectShape::pickerSize( )
  {
    return 4;
  }

  ngrs::Region RectShape::region( )
  {
    return ngrs::Region(rectArea());
  }

  ngrs::Region RectShape::spacingRegion( const Size & spacing )
  {
    return ngrs::Region(Rect(rectArea().left()+ spacing.left(),
      rectArea().top() + spacing.top(),
      rectArea().width()  - (spacing.right()+spacing.left()),
      rectArea().height() - (spacing.top()+spacing.bottom())
      ));
  }

  RectShape * RectShape::clone( ) const
  {
    return new RectShape(*this);
  }

}
