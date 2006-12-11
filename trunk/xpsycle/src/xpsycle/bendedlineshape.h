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

#include <ngrs/nshape.h>

/**
@author Stefan
*/
class BendedLineShape : public NShape
{
public:
    BendedLineShape();

    ~BendedLineShape();

    virtual BendedLineShape* clone() const;   // Uses the copy constructor

    virtual void setPosition(int left, int top, int width, int height);
    virtual void setLeft(int left);
    virtual void setTop(int top);
    virtual void setWidth(int width);
    virtual void setHeight(int height);
    virtual NPoint pickerAt(int i);
    virtual int pickerSize();
    virtual void drawPicker(NGraphics* g);

    const NPoint & p1() const;
    const NPoint & p2() const;
    NPoint p3() const;
    NPoint p4() const;
    NPoint p5() const;

    void setPoints( const NPoint & p1, const NPoint & p2 );
    void setClippingDistance(int d);

    virtual int overPicker(int x, int y);
    virtual void setPicker(int index, int x, int y);

    virtual NRegion region();
    virtual NRegion spacingRegion(const NSize & spacing);

    int d2i(double d);

private:

   NPoint p1_;
   NPoint p2_;
   NPoint p3_;
   NPoint p4_;
   NPoint p5_;

   void resize(int width, int height);
   void calculateRectArea();
   void move(int dx, int dy);

   int pickWidth_ ;
   int pickHeight_;

   NRegion lineToRegion();

   int distance_;

};

#endif
