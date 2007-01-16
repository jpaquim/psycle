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
#ifndef BENDEDLINESHAPE_H
#define BENDEDLINESHAPE_H

#include <ngrs/shape.h>

/**
@author Stefan
*/
class BendedLineShape : public ngrs::Shape
{
public:
  BendedLineShape();

  ~BendedLineShape();

  virtual BendedLineShape* clone() const;   // Uses the copy constructor

  virtual void setPosition( int left, int top, int width, int height );
  virtual void setLeft( int left );
  virtual void setTop( int top );
  virtual void setWidth( int width );
  virtual void setHeight( int height );
  virtual ngrs::NPoint pickerAt( int i );
  virtual int pickerSize();
  virtual void drawPicker( ngrs::Graphics& g );

  const ngrs::NPoint & p1() const;
  const ngrs::NPoint & p2() const;
  ngrs::NPoint p3() const;
  ngrs::NPoint p4() const;
  ngrs::NPoint p5() const;

  void setPoints( const ngrs::NPoint & p1, const ngrs::NPoint & p2 );
  void insertBend( const ngrs::NPoint & bendPt );
  void setClippingDistance( int d );

  const std::vector<ngrs::NPoint> & bendPts() const;

  virtual int overPicker( int x, int y );
  virtual void setPicker( int index, int x, int y );

  virtual ngrs::Region region();
  virtual ngrs::Region spacingRegion( const ngrs::Size & spacing );

  int d2i( double d );

private:

  ngrs::NPoint p1_;
  ngrs::NPoint p2_;
  ngrs::NPoint p3_;
  ngrs::NPoint p4_;
  ngrs::NPoint p5_;

  std::vector< ngrs::NPoint > bendPts_;

  int pickWidth_ ;
  int pickHeight_;
  int distance_;

  ngrs::Region lineToRegion();
  void resize( int width, int height );
  void calculateRectArea();
  void move( int dx, int dy );

};

#endif
