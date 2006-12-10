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
#include "nelbowlineshape.h"
#include <cmath>


NElbowLineShape::NElbowLineShape()
 : NShape()
{
  p1_.setXY(0,0);
  p2_.setXY(10,10);
  pickWidth_ = pickHeight_ = 5;
  distance_ = 5;
  calculateRectArea();
}


NElbowLineShape::~NElbowLineShape()
{
}

NElbowLineShape * NElbowLineShape::clone( ) const
{
  return new NElbowLineShape(*this);
}

NPoint NElbowLineShape::pickerAt( int i )
{
  if (i == 0) return p1_; else return p2_;
}

int NElbowLineShape::pickerSize( )
{
  return 5;
}

void NElbowLineShape::setPosition( int left, int top, int width, int height )
{
  int dx = left - rectArea().left();
  int dy = top  - rectArea().top();
  move(dx,dy);
  resize(width,height);
  calculateRectArea();
}

void NElbowLineShape::resize( int width, int height )
{
  int x = rectArea().left();
  int y = rectArea().top();

  int w = rectArea().width();
  int h = rectArea().height();

  p1_.setX( (int) d2i( ((width) / double ( w) ) * (p1_.x() - x) + x));
  p1_.setY( (int) d2i( ((height) / double ( h) ) * (p1_.y() - y) + y));

  p2_.setX( (int) d2i( ((width) / double ( w) ) * (p2_.x() - x) + x));
  p2_.setY( (int) d2i( ((height) / double ( h) ) * (p2_.y() - y) + y));

}

void NElbowLineShape::calculateRectArea( )
{
   NShape::setPosition ( std::min( p1_.x(), p2_.x() ),  std::min( p1_.y(), 
               p2_.y() ),  std::abs( p1_.x() - p2_.x() ),   std::abs( p1_.y() - 
               p2_.y() ) );
}

void NElbowLineShape::move( int dx, int dy )
{
  p1_.setXY(p1_.x()+dx, p1_.y() +dy);
  p2_.setXY(p2_.x()+dx, p2_.y() +dy);
  calculateRectArea();
}

void NElbowLineShape::drawPicker( NGraphics * g )
{
  g->setForeground(NColor(0,0,0));
  g->fillRect(p1_.x()- pickWidth_/2,p1_.y() - pickHeight_/2, pickWidth_, pickHeight_ );
  g->fillRect(p2_.x()- pickWidth_/2,p2_.y() - pickHeight_/2, pickWidth_, pickHeight_ );
}

void NElbowLineShape::setLeft( int left )
{
  int dx = left - rectArea().left();
  move(dx,0);
}

void NElbowLineShape::setTop( int top )
{
  int dy = top  - rectArea().top();
  move(0,dy);
}

void NElbowLineShape::setWidth( int width )
{
  resize(width,rectArea().height());
  calculateRectArea();
}

void NElbowLineShape::setHeight( int height )
{
  resize(rectArea().width(),height);
  calculateRectArea();
}

const NPoint & NElbowLineShape::p1( ) const
{
  return p1_;
}

const NPoint & NElbowLineShape::p2( ) const
{
  return p2_;
}

NPoint NElbowLineShape::p3() const 
{
        return NPoint(( p1_.x() + p2_.x() ) / 2,  ( p1_.y() + p2_.y() ) 
               / 2 );
}

NPoint NElbowLineShape::p4() const 
{
        return NPoint ( p1().x(), p3().y() );
}

NPoint NElbowLineShape::p5() const 
{
        return NPoint ( p2().x(), p3().y() );
}

void NElbowLineShape::setPoints( NPoint p1, NPoint p2 )
{
  p1_ = p1;
  p2_ = p2;
/*  p3_.setX( (p1.x(), p2.x()) / 2 );
  p3_.setY( (p1.y(), p2.y()) / 2 );
  p4_.setX( p2.x() );
  p4_.setY( p3_.y() );
  p5_.setX( p1.x() );
  p5_.setY( p3_.y() );*/
  calculateRectArea();
}

int NElbowLineShape::overPicker( int x, int y )
{
  if (NRect(p1_.x()-pickWidth_/2,p1_.y()-pickHeight_/2,pickWidth_,pickHeight_).intersects(x,y)) return 0;
  if (NRect(p2_.x()-pickWidth_/2,p2_.y()-pickHeight_/2,pickWidth_,pickHeight_).intersects(x,y)) return 1;
  return -1;
}

void NElbowLineShape::setPicker( int index, int x, int y )
{
  if (index == 0) {
      p1_ = NPoint(x,y);
      calculateRectArea();
  } else 
  if (index == 1) {
      p2_ = NPoint(x,y);
      calculateRectArea();
  }
}

NRegion NElbowLineShape::lineToRegion( )
{
   return NRegion( rectArea() );
}

void NElbowLineShape::setClippingDistance( int d )
{
  distance_ = d;
}

NRegion NElbowLineShape::region( )
{
  return lineToRegion();
}

NRegion NElbowLineShape::spacingRegion( const NSize & spacing )
{
  return lineToRegion();
}


int NElbowLineShape::d2i(double d)
{
   return (int) ( d<0?d-.5:d+.5);
}









