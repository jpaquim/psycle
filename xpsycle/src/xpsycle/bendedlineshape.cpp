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
#include "bendedlineshape.h"
#include <cmath>


BendedLineShape::BendedLineShape()
 : NShape()
{
  p1_.setXY(0,0);
  p2_.setXY(10,10);
  pickWidth_ = pickHeight_ = 5;
  distance_ = 5;
  calculateRectArea();
}


BendedLineShape::~BendedLineShape()
{
}

BendedLineShape * BendedLineShape::clone( ) const
{
  return new BendedLineShape(*this);
}

NPoint BendedLineShape::pickerAt( int i )
{
  if (i == 0) return p1_; else return p2_;
}

int BendedLineShape::pickerSize( )
{
  return 5;
}

void BendedLineShape::setPosition( int left, int top, int width, int height )
{
  int dx = left - rectArea().left();
  int dy = top  - rectArea().top();
  move(dx,dy);
  resize(width,height);
  calculateRectArea();
}

void BendedLineShape::resize( int width, int height )
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

void BendedLineShape::calculateRectArea( )
{
  double  ankathede    = (p1().x() - p2().x());
  double  gegenkathede = (p1().y() - p2().y());
  double  hypetenuse   = sqrt( ankathede*ankathede + gegenkathede*gegenkathede);
      
  double cos = ankathede    / hypetenuse;
  double sin = gegenkathede / hypetenuse;
            
  int dx = (int) (-sin*distance_);
  int dy = (int) (-cos*distance_);
                  
                  
  NPoint  pts[4];
  pts[0].setX ( p1_.x() + dx );
  pts[0].setY ( p1_.y() - dy );
  pts[1].setX ( p2_.x() + dx );
  pts[1].setY ( p2_.y() - dy );
  pts[2].setX ( p2_.x() - dx );
  pts[2].setY ( p2_.y() + dy );
  pts[3].setX ( p1_.x() - dx );
  pts[3].setY ( p1_.y() + dy );
           
  NRegion region;
  region.setPolygon(pts,4);
                                                   
  NRect r = region.rectClipBox();
  NShape::setPosition( r.left(), r.top(), r.width(), r.height() );                                                         
}

void BendedLineShape::move( int dx, int dy )
{
  p1_.setXY(p1_.x()+dx, p1_.y() +dy);
  p2_.setXY(p2_.x()+dx, p2_.y() +dy);
  calculateRectArea();
}

void BendedLineShape::drawPicker( NGraphics * g )
{
  g->setForeground(NColor(100,100,100));
  g->fillRect(p1_.x()- pickWidth_/2,p1_.y() - pickHeight_/2, pickWidth_, pickHeight_ );
  g->fillRect(p2_.x()- pickWidth_/2,p2_.y() - pickHeight_/2, pickWidth_, pickHeight_ );
  
  std::vector<NPoint>::const_iterator it = bendPts().begin();
  for ( ; it < bendPts().end(); it++ ) {
    NPoint pt = *it;
    g->fillRect(pt.x()- pickWidth_/2, pt.y() - pickHeight_/2, pickWidth_, pickHeight_ );
  }
}

void BendedLineShape::setLeft( int left )
{
  int dx = left - rectArea().left();
  move(dx,0);
}

void BendedLineShape::setTop( int top )
{
  int dy = top  - rectArea().top();
  move(0,dy);
}

void BendedLineShape::setWidth( int width )
{
  resize(width,rectArea().height());
  calculateRectArea();
}

void BendedLineShape::setHeight( int height )
{
  resize(rectArea().width(),height);
  calculateRectArea();
}

const NPoint & BendedLineShape::p1( ) const
{
  return p1_;
}

const NPoint & BendedLineShape::p2( ) const
{
  return p2_;
}

NPoint BendedLineShape::p3() const 
{
  return NPoint(( p1_.x() + p2_.x() ) / 2,  ( p1_.y() + p2_.y() ) 
               / 2 );
}

NPoint BendedLineShape::p4() const 
{
  return NPoint ( p2_.x() , p3().y() );
}

NPoint BendedLineShape::p5() const 
{
  return NPoint ( p1_.x(), p3().y() );
}

void BendedLineShape::setPoints( const NPoint & p1, const NPoint & p2 )
{
  p1_ = p1;
  p2_ = p2;
  calculateRectArea();
}

void BendedLineShape::addBend( const NPoint & pts ) {
  bendPts_.push_back( pts );
}

const std::vector<NPoint> & BendedLineShape::bendPts() const {
  return bendPts_;
}

int BendedLineShape::overPicker( int x, int y )
{
  if (NRect(p1_.x()-pickWidth_/2,p1_.y()-pickHeight_/2,pickWidth_,pickHeight_).intersects(x,y)) return 0;
  if (NRect(p2_.x()-pickWidth_/2,p2_.y()-pickHeight_/2,pickWidth_,pickHeight_).intersects(x,y)) return 1;
  
  std::vector<NPoint>::const_iterator it = bendPts().begin();
  int i = 2;
  for ( ; it < bendPts().end(); it++, i++ ) {
    NPoint pt = *it;
    if (NRect(pt.x()-pickWidth_/2,pt.y()-pickHeight_/2,pickWidth_,pickHeight_).intersects(x,y)) return i;
  }
  
  return -1;
}

void BendedLineShape::setPicker( int index, int x, int y )
{
  if (index == 0) {
      p1_ = NPoint(x,y);
      calculateRectArea();
  } else 
  if (index == 1) {
      p2_ = NPoint(x,y);
      calculateRectArea();
  } else
  if ( bendPts_.size() > index - 2 ) {
    NPoint & pt = bendPts_.at( index - 2);
    pt.setXY ( x ,y );
  }
}


NRegion BendedLineShape::lineToRegion( )
{
//
//  int d = distance_;
//  NRect r1( p2().x() - d, std::min( p2().y(), p3().y() ), 2*d, std::abs( p2().y() - p3().y() ) );
//  NRect r2( std::min( p1().x(), p2().x() ) - d, p3().y() - d, std::abs( p1().x() - p2().x() ) + 2*d, 2*d );
//  NRect r3( p1().x() - d, std::min( p1().y(), p3().y() ), 2*d, std::abs( p1().y() - p3().y() ) );
  
//  NRegion region = NRegion( r2 ) | NRegion( r1 ) | NRegion ( r3 );
//  return region;

  NRegion region;

  NPoint startPt = p1();
  std::vector<NPoint>::const_iterator it = bendPts().begin();
  for ( ; it < bendPts().end(); it++ ) {
    NPoint pt = *it;
                                                                                                                                                                               
    double  ankathede    = ( startPt.x() - pt.x() );
    double  gegenkathede = ( startPt.y() - pt.y() );
    double  hypetenuse   = sqrt( ankathede*ankathede + gegenkathede*gegenkathede );
         
    double cos = ankathede    / hypetenuse;
    double sin = gegenkathede / hypetenuse;
               
    int dx = (int) ( -sin*distance_);
    int dy = (int) ( -cos*distance_);
                                          
    NPoint  pts[4];
    pts[0].setX ( startPt.x() + dx );
    pts[0].setY ( startPt.y() - dy );
    pts[1].setX ( pt.x() + dx );
    pts[1].setY ( pt.y() - dy );
    pts[2].setX ( pt.x() - dx );
    pts[2].setY ( pt.y() + dy );
    pts[3].setX ( startPt.x() - dx );
    pts[3].setY ( startPt.y() + dy );
                                      
    NRegion r;
    r.setPolygon(pts,4);
     
    region |= r;
    startPt = pt;
  }

  NPoint pt = p2();
  
  double  ankathede    = ( startPt.x() - pt.x() );
  double  gegenkathede = ( startPt.y() - pt.y() );
  double  hypetenuse   = sqrt( ankathede*ankathede + gegenkathede*gegenkathede );
         
  double cos = ankathede    / hypetenuse;
  double sin = gegenkathede / hypetenuse;
               
  int dx = (int) ( -sin*distance_);
  int dy = (int) ( -cos*distance_);
                                          
  NPoint  pts[4];
  pts[0].setX ( startPt.x() + dx );
  pts[0].setY ( startPt.y() - dy );
  pts[1].setX ( pt.x() + dx );
  pts[1].setY ( pt.y() - dy );
  pts[2].setX ( pt.x() - dx );
  pts[2].setY ( pt.y() + dy );
  pts[3].setX ( startPt.x() - dx );
  pts[3].setY ( startPt.y() + dy );
                                      
  NRegion r;
  r.setPolygon(pts,4);

  region |= r;
  
  return region;                                                         
}

void BendedLineShape::setClippingDistance( int d )
{
  distance_ = d;
}

NRegion BendedLineShape::region( )
{
  return lineToRegion();
}

NRegion BendedLineShape::spacingRegion( const NSize & spacing )
{
  return lineToRegion();
}


int BendedLineShape::d2i(double d)
{
   return (int) ( d<0?d-.5:d+.5);
}
