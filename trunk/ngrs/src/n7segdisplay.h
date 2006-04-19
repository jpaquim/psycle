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
#ifndef N7SEGDISPLAY_H
#define N7SEGDISPLAY_H

#include <npanel.h>

class N7Segment;

/**
@author Stefan Nattkemper
*/
class N7SegDisplay : public NPanel
{
public:
    N7SegDisplay();
    N7SegDisplay(int segmentCount);

    ~N7SegDisplay();

    void setNumber(int number);
    void setColors(const NColor & bg ,const NColor & on,const NColor & off);

private:

    int segCount;
    std::vector<N7Segment*> segs;

    void initSegDisplay();


};

#endif
