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
#include "ngrs/nlineshape.h"
#include <cmath>


NLineShape::NLineShape()
 : NShape()
{
  p1_.setXY(0,0);
  p2_.setXY(10,10);
  pickWidth_ = pickHeight_ = 5;
  distance_ = 5;
  region_ = 0;
  calculateRectArea();
}


NLineShape::~NLineShape()
{
}

NPoint NLineShape::pickerAt( int i )
{
  if (i == 0) return p1_; else return p2_;
}

int NLineShape::pickerSize( )
{
  return 5;
}

void NLineShape::setPosition( int left, int top, int width, int height )
{
  int dx = left - rectArea().left();
  int dy = top  - rectArea().top();
  move(dx,dy);
  resize(width,height);
  calculateRectArea();
}

void NLineShape::resize( int width, int height )
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

void NLineShape::calculateRectArea( )
{
   double  ankathede    = (p1().x() - p2().x());
   double  gegenkathede = (p1().y() - p2().y());
   double  hypetenuse   = sqrt( ankathede*ankathede + gegenkathede*gegenkathede);

   double cos = ankathede    / hypetenuse;
   double sin = gegenkathede / hypetenuse;

   int dx = (int) (-sin*distance_);
   int dy = (int) (-cos*distance_);


   XPoint  pts[4];
   pts[0].x = p1_.x()+dx;
   pts[0].y = p1_.y()-dy;
   pts[1].x = p2_.x()+dx;
   pts[1].y = p2_.y()-dy;
   pts[2].x = p2_.x()-dx;
   pts[2].y = p2_.y()+dy;
   pts[3].x = p1_.x()-dx;
   pts[3].y = p1_.y()+dy;

   Region region = XPolygonRegion(pts,4,WindingRule);
   XRectangle r;
   XClipBox(region, &r);
   NShape::setPosition(r.x,r.y,r.width,r.height);
}

void NLineShape::move( int dx, int dy )
{
  p1_.setXY(p1_.x()+dx, p1_.y() +dy);
  p2_.setXY(p2_.x()+dx, p2_.y() +dy);
  calculateRectArea();
}

void NLineShape::drawPicker( NGraphics * g )
{
  g->setForeground(NColor(0,0,0));
  g->fillRect(p1_.x()- pickWidth_/2,p1_.y() - pickHeight_/2, pickWidth_, pickHeight_ );
  g->fillRect(p2_.x()- pickWidth_/2,p2_.y() - pickHeight_/2, pickWidth_, pickHeight_ );
}

void NLineShape::setLeft( int left )
{
  int dx = left - rectArea().left();
  move(dx,0);
}

void NLineShape::setTop( int top )
{
  int dy = top  - rectArea().top();
  move(0,dy);
}

void NLineShape::setWidth( int width )
{
  resize(width,rectArea().height());
  calculateRectArea();
}

void NLineShape::setHeight( int height )
{
  resize(rectArea().width(),height);
  calculateRectArea();
}

const NPoint & NLineShape::p1( )
{
  return p1_;
}

const NPoint & NLineShape::p2( )
{
  return p2_;
}

void NLineShape::setPoints( NPoint p1, NPoint p2 )
{
  p1_ = p1;
  p2_ = p2;
  calculateRectArea();
}

int NLineShape::overPicker( int x, int y )
{
  if (NRect(p1_.x()-pickWidth_/2,p1_.y()-pickHeight_/2,pickWidth_,pickHeight_).intersects(x,y)) return 0;
  if (NRect(p2_.x()-pickWidth_/2,p2_.y()-pickHeight_/2,pickWidth_,pickHeight_).intersects(x,y)) return 1;
  return -1;
}

void NLineShape::setPicker( int index, int x, int y )
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

NRegion NLineShape::lineToRegion( )
{
   double  ankathede    = (p1().x() - p2().x());
   double  gegenkathede = (p1().y() - p2().y());
   double  hypetenuse   = sqrt( ankathede*ankathede + gegenkathede*gegenkathede);

   double cos = ankathede    / hypetenuse;
   double sin = gegenkathede / hypetenuse;

   int dx = (int) ( -sin*distance_);
   int dy = (int) ( -cos*distance_);

 
   XPoint  pts[4];
   pts[0].x = p1_.x()+dx;
   pts[0].y = p1_.y()-dy;
   pts[1].x = p2_.x()+dx;
   pts[1].y = p2_.y()-dy;
   pts[2].x = p2_.x()-dx;
   pts[2].y = p2_.y()+dy;
   pts[3].x = p1_.x()-dx;
   pts[3].y = p1_.y()+dy;

   NRegion region;
   region.setPolygon(pts,4);

   return region;
}

void NLineShape::setClippingDistance( int d )
{
  distance_ = d;
}

NRegion NLineShape::region( )
{
  return lineToRegion();
}

NRegion NLineShape::spacingRegion( const NSize & spacing )
{
  return lineToRegion();
}


int NLineShape::d2i(double d)
 {
   return (int) ( d<0?d-.5:d+.5);
 }







