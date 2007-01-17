/***************************************************************************
 *   Copyright (C) 2005, 2006, 2007 by  Stefan Nattkemper   *
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
#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "system.h"
#include "rect.h"
#include "color.h"
#include <stack>
#include "bitmap.h"
#include "fntstring.h"
#include "region.h"
#include "pixmap.h"
#include "pen.h"
#ifdef __unix__
#else
  #include "size.h"
#endif

/**
@author  Stefan
*/

namespace ngrs {

#ifdef __unix__
#else
  typedef HDC GC;
#endif

  const int nBltNormal = 0;
  const int nBltStretch = 1;

  class Graphics{
  public:
    Graphics(WinHandle winID);

    ~Graphics();

    void setRepaintArea(const ngrs::Region & rect);
    const ngrs::Region & repaintArea();

    void setClipping(const ngrs::Region & region);

    void setRegion(const ngrs::Region & region);
    ngrs::Region region();

    void setPen(const Pen & pen);
    const Pen & pen() const;
    void resetPen();

    void drawLine(long x,long y,long x1, long y1);
    void drawRect( int x, int y, int width, int height );
    void drawRoundRect(int x, int y, int width, int height, int arcWidth, int arcHeight);
    void drawRect( const Rect & rect );
    void drawArc(int x, int y, int width, int height, int angle1, int angle2);
    void fillArc(int x, int y, int width, int height, int angle1, int angle2);
    void fillRect(int x, int y, int width, int height);
    void fillRoundRect(int x, int y, int width, int height, int arcWidth, int arcHeight);
    void fillRect(const Rect & rect);
    void fillGradient(int x, int y, int width, int height, const Color & start, const Color & end , int direction);
    void fillGradient(int x, int y, int width, int height, const Color & start, const Color & mid, const Color & end , int direction, int percent);
    void fillRoundGradient(int x, int y, int width, int height, const Color & start, const Color & end , int direction , int arcWidth, int arcHeight);
    void fillRoundGradient(int x, int y, int width, int height, const Color & start, const Color & mid, const Color & end , int direction , int percent, int arcWidth, int arcHeight);



    void setTranslation( long dx, long dy );
    void resize(int width, int height);
    void swap( const Rect & repaintArea );
    long xTranslation();
    long yTranslation();
    void setForeground( const Color & color );

    void setFont( const Font & font );
    void drawText( int x, int y, const std::string & text );
    void drawText( int x, int y, const std::string & text, const Color & color );
    void drawText( int x, int y, const FntString & text );

    int textWidth(const std::string & text) const;
    int textWidth( const FntString & text ) const;

    int textHeight();
    int textAscent();
    int textDescent();

    void drawPolygon(NPoint* pts, int n);
    void fillPolygon(NPoint* pts, int n);

    void fillTranslucent(int x, int y, int width, int height, Color color, int percent);

    void putBitmap(int x, int y, const Bitmap & bitmap );
    void putStretchBitmap(int x, int y, const Bitmap & bitmap, int width, int height );
    void putBitmap(int destX, int destY, int width, int height, const Bitmap & bitmap, int srcX, int srcY );
    void putPixmap(int destX, int destY, int width, int height, Pixmap & pixmap, int srcX, int srcY );

    void setDoubleBuffer(bool on);

#ifdef __unix__
    ::Pixmap dbPixmap();
#endif

    GC dbGC();
    GC gc();

    void copyArea(int src_x,int src_y,unsigned width,unsigned height,int dest_x,int dest_y, bool dblBuffer_ = true);

    int dblWidth() const;
    int dblHeight() const;

    void setVisible(bool on);

  private:

    Color old;
    ngrs::Region repaintArea_;
    Color oldColor;
    Font fnt;
    long dx_;
    long dy_;
    bool dblBuffer_;

    int dblWidth_;
    int dblHeight_;

    WinHandle win;
    GC gc_;      // GC from Window
    GC gcp;     // GC from Pixmap for double buffering

#ifdef __unix__
    ::Pixmap doubleBufferPixmap_;
#else
    HBITMAP doubleBufferBitmap_;
#endif

    void createDblBufferHandles();
    void destroyDblBufferHandles();
    void copyDblBuffer(const Rect &  repaintArea);
    void drawXftString(int x, int y, const char* s);

    FontStructure fntStruct;
#ifdef __unix__
    XftColor fFtColor;
    XftDraw* drawDbl;
    XftDraw* drawWin;
#else
    HBRUSH brush;
    HBRUSH hollow;
    HPEN hPen;
    void drawArcX( int x, int y, int width, int height, int start, int extent, bool fill );
#endif

    ngrs::Region region_;

    Pen pen_;

    bool visible_;
  };

 }

#endif
