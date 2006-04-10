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
#ifndef NGRAPHICS_H
#define NGRAPHICS_H

#include "nsystem.h"
#include "nrect.h"
#include "ncolor.h"
#include <stack>
#include "nbitmap.h"
#include "nfntstring.h"
#include "nregion.h"


/**
@author Stefan
*/
class NGraphics{
public:
    NGraphics(Window winID);

    ~NGraphics();

    void setRepaintArea(const NRegion & rect);
    const NRegion & repaintArea();

    void setClipping(const NRegion & region);

    void setRegion(const NRegion & region);
    NRegion region();

    void drawLine(long x,long y,long x1, long y1);
    void drawRect( int x, int y, int width, int height );
    void drawRoundRect(int x, int y, int width, int height, int arcWidth, int arcHeight);
    void drawRect( const NRect & rect );
    void drawArc(int x, int y, int width, int height, int angle1, int angle2);
    void fillArc(int x, int y, int width, int height, int angle1, int angle2);
    void fillRect(int x, int y, int width, int height);
    void fillRoundRect(int x, int y, int width, int height, int arcWidth, int arcHeight);
    void fillRect(const NRect & rect);
    void fillGradient(int x, int y, int width, int height, const NColor & start, const NColor & end , int direction);
    void fillGradient(int x, int y, int width, int height, const NColor & start, const NColor & mid, const NColor & end , int direction, int percent);
    void fillRoundGradient(int x, int y, int width, int height, const NColor & start, const NColor & end , int direction , int arcWidth, int arcHeight);
    void fillRoundGradient(int x, int y, int width, int height, const NColor & start, const NColor & mid, const NColor & end , int direction , int percent, int arcWidth, int arcHeight);



    void setTranslation( long dx, long dy );
    void resize(int width, int height);
    void swap(const NRect & repaintArea);
    long xTranslation();
    long yTranslation();
    void setForeground(const NColor & color);

    void setFont(const NFont & font);
    void drawText(int x, int y, const std::string & text);
    void drawText(int x, int y, const NFntString & text);
    int textWidth(const std::string & text) const;
    int textWidth( const NFntString & text ) const;
    int findWidthMax(long width, const std::string & data, bool wbreak) const;
    int findWidthMax(long width, const NFntString & data, bool wbreak) const;
    int textHeight();
    int textAscent();
    int textDescent();

    void drawPolygon(XPoint* pts, int n);
    void fillPolygon(XPoint* pts, int n);

    void fillTranslucent(int x, int y, int width, int height, NColor color, int percent);

    void putBitmap(int x, int y, NBitmap & bitmap);
    void putBitmap(int destX, int destY, int width, int height, NBitmap & bitmap, int srcX, int srcY );

    void setDoubleBuffer(bool on);

    Pixmap dbPixmap();
    GC dbGC();
    GC gc();


    void copyArea(int src_x,int src_y,unsigned width,unsigned height,int dest_x,int dest_y, bool dblBuffer_ = true);

    int dblWidth() const;
    int dblHeight() const;

private:

   NColor old;
   NRegion repaintArea_;
   NColor oldColor;
   NFont fnt;
   long dx_;
   long dy_;
   bool dblBuffer_;

   int dblWidth_;
   int dblHeight_;

   Window win;
   GC gc_;      // GC from Window
   GC gcp;     // GC from Pixmap for double buffering

   Pixmap doubleBufferPixmap_;

   void createGraphicHandles();
   void destroyGraphicHandles();
   void copyDblBuffer(const NRect &  repaintArea);
   void drawXftString(int x, int y, const char* s);

   NFontStructure fntStruct;
   XftColor fFtColor;
   XftDraw* draw;

   NRegion region_;


};

#endif
