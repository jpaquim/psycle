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

#include "bevelborder.h"

namespace ngrs {

  BevelBorder::BevelBorder()
    : Border()
  {
    innerStyle_=nRaised;
    outerStyle_=nLowered;
    oval_=false;
    mapStyle();
  }

  BevelBorder::BevelBorder( int outerStyle, int innerStyle ) : Border()
  {
    innerStyle_=innerStyle;
    outerStyle_=outerStyle;
    oval_=false;
    mapStyle();
  }


  BevelBorder::~BevelBorder()
  {
  }

  void BevelBorder::paint( Graphics& g, const Shape & geometry)
  { 
    g.setForeground(Color(200,200,200));

    int x      = geometry.left();
    int y      = geometry.top();
    int width  = geometry.width();
    int height = geometry.height();

    g.setTranslation(g.xTranslation()+x,g.yTranslation()+y);
    drawRectBorder(g,width,height);
    g.setTranslation(g.xTranslation()-x,g.yTranslation()-y);
  }


  void BevelBorder::drawRectBorder(Graphics& g, int width, int height) 
  {
    int sz = 2;

    switch (style_)
    {
    case 1: // bevelOuter raised , inner lowered
      g.setForeground(Color(230,230,230));
      g.drawLine(0,0,width-1,0);
      g.drawLine(0,0,0,height-1);
      g.setForeground(Color(150,150,150));
      g.drawLine(0,height-1,width-1,height-1);
      g.drawLine(width-1,0,width-1,height-1);

      g.setForeground(Color(150,150,150));
      g.drawLine(sz-1,sz-1,width-1-sz+1,sz-1);
      g.drawLine(sz-1,sz-1,sz-1,height-1-sz+1);
      g.setForeground(Color(230,230,230));
      g.drawLine(sz-1,height-1-sz+1,width-1-sz+1,height-1-sz+1);
      g.drawLine(width-1-sz+1,sz-1,width-1-sz+1,height-1-sz+1);
      break;
    case 2: // bevelOuter lowerd , inner raised
      g.setForeground(Color(150,150,150));
      g.drawLine(0,0,width-1,0);
      g.drawLine(0,0,0,height-1);
      g.setForeground(Color(230,230,230));

      g.drawLine(0,height-1,width-1,height-1);
      g.drawLine(width-1,0,width-1,height-1);

      g.setForeground(Color(230,230,230));
      g.drawLine(sz-1,sz-1,width-1-sz+1,sz-1);
      g.drawLine(sz-1,sz-1,sz-1,height-1-sz+1);
      g.setForeground(Color(150,150,150));

      g.drawLine(sz-1,height-1-sz+1,width-1-sz+1,height-1-sz+1);
      g.drawLine(width-1-sz+1,sz-1,width-1-sz+1,height-1-sz+1);
      break;
    case 6:
      g.setForeground(Color(230,230,230));
      g.drawLine(sz-1,sz-1,width-1-sz+1,sz-1);
      g.drawLine(sz-1,sz-1,sz-1,height-1-sz+1);

      g.setForeground(Color(150,150,150));
      g.drawLine(sz-1,height-1-sz+1,width-1-sz+1,height-1-sz+1);
      g.drawLine(width-1-sz+1,sz-1,width-1-sz+1,height-1-sz+1);
      break;
    case 7:
      g.setForeground(Color(150,150,150));
      g.drawLine(sz-1,sz-1,width-1-sz+1,sz-1);
      g.drawLine(sz-1,sz-1,sz-1,height-1-sz+1);

      g.setForeground(Color(230,230,230));
      g.drawLine(sz-1,height-1-sz+1,width-1-sz+1,height-1-sz+1);
      g.drawLine(width-1-sz+1,sz-1,width-1-sz+1,height-1-sz+1);
      break;
    default :;
    }
  }



  void BevelBorder::setInnerStyle( int style )
  {
    innerStyle_ = style;
    mapStyle();  
  }

  void BevelBorder::setOuterStyle( int style )
  {
    outerStyle_ = style;
    mapStyle();  
  }

  int BevelBorder::innerStyle( )
  {
    return innerStyle_;
  }

  int BevelBorder::outerStyle( )
  {
    return outerStyle_;
  }

  void BevelBorder::mapStyle( )
  {
    if (innerStyle_==nNone    && outerStyle_==nNone)  style_=0; 
      else
    if (outerStyle_==nRaised  && innerStyle_==nLowered) style_=1;
      else 
    if (outerStyle_==nLowered  && innerStyle_==nRaised) style_=2;
      else
    if (outerStyle_==nRaised  && innerStyle_==nNone)  style_=5;
      else
    if (outerStyle_==nLowered && innerStyle_==nNone)  style_=4;
      else
    if (outerStyle_==nNone && innerStyle_==nRaised) style_=6;
      else
    if (outerStyle_==nNone && innerStyle_==nLowered) style_=7;
  }

  void BevelBorder::setStyle( int outerStyle, int innerStyle, int size )
  {
    innerStyle_ = innerStyle;
    outerStyle_ = outerStyle;
    mapStyle();
    //  size_ = size;
  }

  // usage of Covariant Return Types, a feature that was not originally part of c++. If your //compiler complains at the declaration of BevelBorder* clone() const, you have an old compiler and you'll have to change the return type to Border*.

  BevelBorder * BevelBorder::clone( ) const
  {
    return new BevelBorder(*this);
  }

}
