/***************************************************************************
 *   Copyright (C) 2006, 2007 by Stefan Nattkemper   *
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
#include "seg7display.h"
#include "flowlayout.h"
#include "segment7.h"

namespace ngrs {

  Seg7Display::Seg7Display( )
  {
    segCount=4;
    initSegDisplay();
  }


  Seg7Display::Seg7Display(int segmentCount)
    : Panel()
  {
    segCount=segmentCount;
    initSegDisplay();
  }


  Seg7Display::~Seg7Display()
  {

  }

  void Seg7Display::setNumber( int number )
  {
    int digit[1000];

    for (int i=0; i<1000; i++) digit[i]=-1;

    if (number==0) digit[0]=0; else {
      int n = 1;
      for(int i = 0, n = 1; n <= number; ++i, n *= 10) digit[i] = (number / n) % 10;
    }

    int j = 0;
    for (std::vector<Segment7*>::iterator it =  segs.begin(); it < segs.end(); it++)
    {
      Segment7* segment = *it;
      segment->resetSeg();
      segment->setNumber( digit[segs.size()-j-1] );
      j++;
    }
  }

  void Seg7Display::initSegDisplay( )
  {
    setLayout(FlowLayout(nAlLeft,0,0));

    for (int i=0; i<segCount; i++) {
      Segment7* seg = new Segment7();
      add(seg);
      segs.push_back(seg);
    }

    setWidth (preferredWidth());
    setHeight(preferredHeight());
  }

  void Seg7Display::setColors(const Color & bg, const Color & on, const Color & off )
  {
    for (std::vector<Segment7*>::iterator it =  segs.begin(); it < segs.end(); it++)
    {
      Segment7* segment = *it;
      segment->setColors(on,off);
      segment->setBackground(bg);
    }
  }

}
