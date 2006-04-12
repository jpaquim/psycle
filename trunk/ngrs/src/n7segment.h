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
#ifndef N7SEGMENT_H
#define N7SEGMENT_H

#include <npanel.h>

/**
@author Stefan Nattkemper
*/
class N7Segment : public NPanel
{
public:
    N7Segment();

    ~N7Segment();

    virtual void paint(NGraphics* g);

    void setSeg(bool seg1,bool seg2,bool seg3,bool seg4,bool seg5,bool seg6,bool seg7);
    void resetSeg();

    void setNumber(int z);

private:

    NColor onColor;
    NColor offColor;

    bool seg1_, seg2_ , seg3_, seg4_, seg5_, seg6_, seg7_;

    void drawVerticalSegment(NGraphics*g,int x, int y, int x1, int y1, bool on);
    void drawHorizontalSegment(NGraphics*g,int x, int y, int x1, int y1,bool on);

};

#endif
