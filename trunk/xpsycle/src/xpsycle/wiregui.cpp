/***************************************************************************
 *   Copyright (C) 2006 by Stefan   *
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
#include "wiregui.h"
#include "global.h"
#include "configuration.h"
#include <cmath>
#include <cstdio>

template<typename X, typename Y>
inline double min (X x,Y y) {
   return std::min((double)(x),(double)(y));
}

template<typename X, typename Y>
inline double max (X x,Y y) { 
   return std::max((double)(x),(double)(y));
}


WireGUI::WireGUI()
 : NLine()
{
  deltaColR = (Global::pConfig()->mv_polycolour.red() / 510.0) + .45;
  deltaColG = (Global::pConfig()->mv_polycolour.green() / 510.0) + .45;
  deltaColB = (Global::pConfig()->mv_polycolour.blue() / 510.0) + .45;

  triangle_size_tall = Global::pConfig()->mv_triangle_size+((23*Global::pConfig()->mv_wirewidth)/16);

  triangle_size_center = triangle_size_tall/2;
  triangle_size_wide = triangle_size_tall/2;
  triangle_size_indent = triangle_size_tall/6;

}


WireGUI::~WireGUI()
{
}

void WireGUI::paint( NGraphics * g )
{
  NLine::paint(g);

  g->setTranslation(g->xTranslation()-left(),g->yTranslation()-top());
  drawArrow(g);
  g->setTranslation(g->xTranslation()+left(),g->yTranslation()+top());
}

void WireGUI::drawArrow( NGraphics * g )
{
  // Spaces between the end and startPoint of the Line

  double  ankathede    = (p1().x() - p2().x());
  double  gegenkathede = (p1().y() - p2().y());
  double  hypetenuse   = std::sqrt( ankathede*ankathede + gegenkathede*gegenkathede);


  double cos = ankathede / hypetenuse;
  double sin = gegenkathede / hypetenuse;

  int middleX = (p1().x() + p2().x()) / 2;
  int middleY = (p1().y() + p2().y()) / 2;

  double slope = atan2(middleX,middleY);
  int rtcol = 140+abs((int)(slope*32));

  double altslope=slope;
  if(altslope<-1.05)  altslope -= 2 * (altslope + 1.05);
  if(altslope>2.10) altslope -= 2 * (altslope - 2.10);
  int ltcol = 140 + abs((int)((altslope - 2.10) * 32));
  altslope=slope;
  if(altslope>0.79)  altslope -= 2 * (altslope - 0.79);
  if(altslope<-2.36)  altslope -= 2 * (altslope + 2.36);
  int btcol = 240 - abs((int)((altslope-0.79) * 32));


  NColor rtBrush(max(0, min(255, rtcol * deltaColR)),
                 max(0, min(255, rtcol * deltaColG)),
                 max(0, min(255, rtcol * deltaColB)));
  NColor ltBrush(max(0, min(255, ltcol * deltaColR)),
                 max(0, min(255, ltcol * deltaColG)),
                 max(0, min(255, ltcol * deltaColB)));
  NColor btBrush(max(0, min(255, btcol * deltaColR)),
                 max(0, min(255, btcol * deltaColG)),
                 max(0, min(255, btcol * deltaColB)));


  XPoint pol[5];

  pol[0].x = middleX -  (int) (cos    * triangle_size_center);
  pol[0].y = middleY -  (int) (sin    * triangle_size_center);
  pol[1].x = pol[0].x + (int) (cos    * triangle_size_tall);
  pol[1].y = pol[0].y + (int) (sin    * triangle_size_tall);
  pol[2].x = pol[0].x - (int) (sin    * triangle_size_wide);
  pol[2].y = pol[0].y + (int) (cos    * triangle_size_wide);
  pol[3].x = pol[0].x + (int) (cos    * triangle_size_indent);
  pol[3].y = pol[0].y + (int) (sin    * triangle_size_indent);
  pol[4].x = pol[0].x + (int) (sin    * triangle_size_wide);
  pol[4].y = pol[0].y - (int) (cos    * triangle_size_wide);

  XPoint fillPoly[7];

  fillPoly[2].x = pol[0].x + (int) (2* cos * triangle_size_indent);
  fillPoly[2].y = pol[0].y + (int) (2* sin * triangle_size_indent);
  fillPoly[6].x = fillPoly[2].x;
  fillPoly[6].y = fillPoly[2].y;
  fillPoly[1].x = pol[1].x;
  fillPoly[1].y = pol[1].y;
  fillPoly[0].x = pol[2].x;
  fillPoly[0].y = pol[2].y;
  fillPoly[5].x = pol[2].x;
  fillPoly[5].y = pol[2].y;
  fillPoly[4].x = pol[3].x;
  fillPoly[4].y = pol[3].y;
  fillPoly[3].x = pol[4].x;
  fillPoly[3].y = pol[4].y;

  g->setForeground(rtBrush);
  g->fillPolygon(fillPoly,3);
  g->setForeground(ltBrush);
  g->fillPolygon(&fillPoly[1],3);
  g->setForeground(btBrush);
  g->fillPolygon(&fillPoly[3], 4);

  g->setForeground(NColor(0,0,0));
  g->drawPolygon(fillPoly,3);
  g->setForeground(NColor(0,0,0));
  g->drawPolygon(&fillPoly[1],3);
  g->setForeground(NColor(0,0,0));
  g->drawPolygon(&fillPoly[3], 4);
}


