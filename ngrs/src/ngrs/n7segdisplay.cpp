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
#include "ngrs/n7segdisplay.h"
#include "ngrs/nflowlayout.h"
#include "ngrs/n7segment.h"

N7SegDisplay::N7SegDisplay( )
{
  segCount=4;
  initSegDisplay();
}


N7SegDisplay::N7SegDisplay(int segmentCount)
 : NPanel()
{
  segCount=segmentCount;
  initSegDisplay();
}


N7SegDisplay::~N7SegDisplay()
{

}

void N7SegDisplay::setNumber( int number )
{
  int digit[1000];

  for (int i=0; i<1000; i++) digit[i]=-1;

  if (number==0) digit[0]=0; else {
    int n;
    for(int i = 0, n = 1; n <= number; ++i, n *= 10) digit[i] = (number / n) % 10;
  }

  int i = 0;
  for (std::vector<N7Segment*>::iterator it =  segs.begin(); it < segs.end(); it++)
  {
    N7Segment* segment = *it;
    segment->resetSeg();
    segment->setNumber( digit[segs.size()-i-1] );
    i++;
  }
}

void N7SegDisplay::initSegDisplay( )
{
 setLayout(NFlowLayout(nAlLeft,0,0));

 for (int i=0; i<segCount; i++) {
   N7Segment* seg = new N7Segment();
   add(seg);
   segs.push_back(seg);
 }

 setWidth (preferredWidth());
 setHeight(preferredHeight());
}

void N7SegDisplay::setColors(const NColor & bg, const NColor & on, const NColor & off )
{
  for (std::vector<N7Segment*>::iterator it =  segs.begin(); it < segs.end(); it++)
  {
    N7Segment* segment = *it;
    segment->setColors(on,off);
    segment->setBackground(bg);
  }
}


