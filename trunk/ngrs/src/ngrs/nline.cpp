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
#include "ngrs/nline.h"
#include <cmath>

NLine::NLine()
 : NVisualComponent()
{
  lineShape = new NLineShape();
  setGeometry(lineShape);
  lineShape->setClippingDistance(30);
}


NLine::~NLine()
{
}

void NLine::paint( NGraphics * g )
{
  g->setTranslation(g->xTranslation()-left(),g->yTranslation()-top());
  g->drawLine(lineShape->p1().x(),  lineShape->p1().y(), lineShape->p2().x(), lineShape->p2().y());
  g->setTranslation(g->xTranslation()+left(),g->yTranslation()+top());
}

void NLine::setPoints( NPoint p1, NPoint p2 )
{
  lineShape->setPoints(p1,p2);
}



const NPoint & NLine::p1( )
{
  return lineShape->p1();
}

const NPoint & NLine::p2( )
{
  return lineShape->p2();
}



