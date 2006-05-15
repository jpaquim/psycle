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
#include "n7segment.h"

N7Segment::N7Segment()
 : NPanel()
{
   setBackground(NColor(170,0,0));
   setTransparent(false);

   offColor.setRGB(180,0,0);
   onColor.setRGB(255,0,0);
   seg1_=seg2_=seg3_=seg4_=seg5_=seg6_=seg7_=false;

   setWidth(12);
   setHeight(20);
}


N7Segment::~N7Segment()
{
}


void N7Segment::paint( NGraphics * g )
{
  int borderDistH = height() / 8;
  int middleH     = height() / 2;
  int segW        = width() / 6;

  int borderDistWO = width() / 8;
  int borderDistWU = 0;
  int segH        = width() / 8;

  drawVerticalSegment(g,borderDistWO,borderDistH,borderDistWO+segW,middleH,seg1_);
  drawVerticalSegment(g,borderDistWU,middleH,borderDistWU+segW,height()-borderDistH,seg2_);

  drawVerticalSegment(g,width()-borderDistWU-segW,borderDistH,width()-borderDistWU,middleH,seg3_);
  drawVerticalSegment(g,width()-borderDistWO-segW,middleH,width()-borderDistWO,height()-borderDistH,seg4_);

  drawHorizontalSegment(g,borderDistWU+segW,middleH,width()-borderDistWO-segW,middleH+segH,seg5_);

  drawHorizontalSegment(g,borderDistWO+segW,borderDistH,width()-borderDistWU-segW,borderDistH+segH,seg6_);
  drawHorizontalSegment(g,borderDistWU+segW,height()-borderDistH-segH,width()-borderDistWO-segW,height()-borderDistH,seg7_);
}

void N7Segment::drawVerticalSegment( NGraphics * g, int x , int y ,int  x1 , int y1, bool on )
{
 if (on) g->setForeground(onColor);
    else g->setForeground(offColor);

 int h  = (y1-y) / 8;
 int sc = (x1-x)/3;
 for (int i=x; i<x1-sc; i++) {
  g->drawLine(i,y1-h,sc+i,y+h);
 }
}

void N7Segment::drawHorizontalSegment( NGraphics * g, int x, int y, int x1, int y1, bool on )
{
 if (on) g->setForeground(onColor);
    else g->setForeground(offColor);

 int w  = (x1-x) / 8;
 for (int i=x+w; i<x1-w; i++)
  g->drawLine(i,y1, i,y);
}

void N7Segment::setSeg( bool seg1, bool seg2, bool seg3, bool seg4, bool seg5, bool seg6, bool seg7 )
{
  seg1_ = seg1;
  seg2_ = seg1;
  seg3_ = seg1;
  seg4_ = seg1;
  seg5_ = seg1;
  seg6_ = seg1;
  seg7_ = seg1;
}

void N7Segment::setNumber( int z )
{
 if (z>0) resetSeg();
 if (z==0) {
      resetSeg();
      seg1_=seg2_=seg3_=seg4_=seg6_=seg7_=true;
      } else
 if (z==1) {
      resetSeg();
      seg3_=seg4_=true;
     } else
 if (z==2) {
      resetSeg();
      seg6_=seg3_=seg5_=seg2_=seg7_=true;
     } else
 if (z==3) {
    resetSeg();
    seg6_=seg3_=seg5_=seg4_=seg7_=true;
 } else
 if (z==4) {
    resetSeg();
    seg1_=seg5_=seg3_=seg4_=true;
 } else
  if (z==5) {
    resetSeg();
    seg6_=seg1_=seg5_=seg4_=seg7_=true;
 } else
  if (z==6) {
    resetSeg();
    seg6_=seg1_=seg2_=seg7_=seg4_=seg5_=true; 
 } else
 if (z==7) {
    resetSeg();
    seg6_=seg3_=seg4_=true;
 } else
 if (z==8) {
    resetSeg();
    seg1_=seg2_=seg3_=seg4_=seg5_=seg6_=seg7_=true; 
 } else
 if (z==9) {
    resetSeg();
    seg1_=seg3_=seg4_=seg5_=seg6_=seg7_=true; 
 }
}

void N7Segment::resetSeg( )
{
  seg1_=seg2_=seg3_=seg4_=seg5_=seg6_=seg7_=false;
}

void N7Segment::setColors( const NColor & on, const NColor & off )
{
  onColor  = on;
  offColor = off;
}
