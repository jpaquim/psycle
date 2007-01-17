/***************************************************************************
*   Copyright (C) 2005, 2006, 2007 by  Stefan Nattkemper                         *
*   Made in Germany                                                       *
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
#include "graphics.h"
#include "app.h"
#include "visualcomponent.h"
#include <cmath>
#include <iostream>

namespace ngrs{

#ifdef __unix__
#else

#define PI 3.14159265358979
#define XAngleToRadians(a) ((double)(a) / 64 * PI / 180);

#endif

  Graphics::Graphics( WinHandle winHandle ) :
win(winHandle), dx_(0), dy_(0), dblWidth_(0), dblHeight_(0), dblBuffer_(0)
{   
#ifdef __unix__
  fFtColor.color.red   = 0x0000;
  fFtColor.color.green = 0x0000;
  fFtColor.color.blue  = 0x0000;
  fFtColor.color.alpha = 0xFFFF; // Alpha blending
#else
  brush = ::CreateSolidBrush(RGB(0,0,0));
  // Set brush to hollow
  ::LOGBRUSH logbrush;
  logbrush.lbStyle = BS_HOLLOW;
  hollow = ::CreateBrushIndirect(&logbrush);
  hPen  = ::CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
#endif
#ifdef __unix__
  gc_     = ::XCreateGC( App::system().dpy(),win, 0, 0);
  drawWin = ::XftDrawCreate( App::system().dpy(), win, App::system().visual(), App::system().colormap());
  doubleBufferPixmap_=0;
  gcp = 0;
  drawDbl = 0;
  currentDrawable_ = win;
#else
  gc_ = ::GetDC( win );
  doubleBufferBitmap_=0;
  gcp = 0;
#endif
  currentGc_ = gc_;
  visible_ = false;
  setFont(fnt);
  setForeground(oldColor);
}

Graphics::~Graphics()
{
#ifdef __unix__
  ::XftDrawDestroy(drawWin);
#else  
  ::ReleaseDC( win , gc_ );
  ::DeleteObject( brush );
  ::DeleteObject( hollow );
  ::DeleteObject( hPen );
#endif

  destroyDblBufferHandles();
}

void Graphics::fillRect( int x, int y, int width, int height )
{
  if ( !currentGc_ ) return;
  // Note : XFillRectangle has the same
  // width as Rectangle, but XDrawRectangle is one pixel wider
  // for the same co-ordinates.   
#ifdef __unix__
  ::XFillRectangle( App::system().dpy(), currentDrawable_, currentGc_, x+dx_, y+dy_, width, height );
#else
  ::Rectangle( currentGc_, x + dx_,y + dy_, x + dx_ + width, y + dy_ +height);
#endif
}

void Graphics::createDblBufferHandles( )
{
#ifdef __unix__
  if (!gcp) {
    ::XWindowAttributes attr;
    ::XGetWindowAttributes( App::system().dpy(), win, &attr );

    doubleBufferPixmap_ = XCreatePixmap(App::system().dpy(), win, attr.width, attr.height, App::system().depth());
    gcp = XCreateGC(App::system().dpy(),doubleBufferPixmap_,0,0);
    dblWidth_  = attr.width;
    dblHeight_ = attr.height;
    XSetForeground( App::system().dpy(), gcp, oldColor.colorValue() );
    drawDbl = XftDrawCreate(App::system().dpy(), doubleBufferPixmap_, App::system().visual(), App::system().colormap());  
  }
#else
  if (!gcp) {
    RECT r;
    GetWindowRect( win, &r );

    gcp = CreateCompatibleDC( gc_ );
    doubleBufferBitmap_ = CreateCompatibleBitmap( gc_, r.right - r.left, r.bottom - r.top);
    SelectObject( gcp, doubleBufferBitmap_ );
  }
#endif
}

void Graphics::destroyDblBufferHandles( )
{     
  if ( gcp ) {
     if ( currentGc_ == gcp ) currentGc_ = gc_;
#ifdef __unix__
    XFreeGC(App::system().dpy(), gcp);
    XFreePixmap(App::system().dpy(), doubleBufferPixmap_);
    XftDrawDestroy( drawDbl );
    gcp = 0;
    doubleBufferPixmap_=0;
    drawDbl = 0;
    XSetForeground( App::system().dpy(), gc_, oldColor.colorValue() );
#else
    DeleteDC( gcp);
    DeleteObject( doubleBufferBitmap_ );
    doubleBufferBitmap_ = 0;
    gcp = 0;
#endif
  }
}

void Graphics::updateCurrentGc() {
  if ( dblBuffer_ && gcp) {
   currentGc_ = gcp;
#ifdef __unix__
    currentDrawable_ = doubleBufferPixmap_;
#endif
  } else {
    currentGc_ = gc_;
#ifdef __unix__
    currentDrawable_ = win;
#endif      
  }
}

void Graphics::setTranslation( long dx, long dy )
{
  dx_ = dx;
  dy_ = dy;
}

void Graphics::resize( int width, int height )
{
  if ( dblBuffer_ ) {
    destroyDblBufferHandles();
    createDblBufferHandles();
    updateCurrentGc();    
  }
}

void Graphics::copyDblBuffer( const Rect & repaintArea )
{
  if ( !gcp ) return;
#ifdef __unix__
  XCopyArea(App::system().dpy(), doubleBufferPixmap_, win, gc_,repaintArea.left(), repaintArea.top(),repaintArea.width(), repaintArea.height(),repaintArea.left(), repaintArea.top());
#else
  // blit the gcp to gc_
  BitBlt( gc_, repaintArea.left(), repaintArea.top(), repaintArea.width(), repaintArea.height(), gcp, repaintArea.left(), repaintArea.top(), SRCCOPY);
#endif     
}

void Graphics::swap( const Rect& repaintArea )
{
  if ( dblBuffer_ ) 
    copyDblBuffer( repaintArea );
}

long Graphics::xTranslation( ) const
{
  return dx_;
}

long Graphics::yTranslation( ) const
{
  return dy_;
}

void Graphics::setForeground( const Color& color )
{
  if ( !currentGc_ ) return;

  if ( !( oldColor==color) ) {
#ifdef __unix__                 
    XSetForeground( App::system().dpy(), currentGc_, color.colorValue() );
#else
    ::DeleteObject( brush );
    brush =  ::CreateSolidBrush(RGB(color.red(), color.green(), color.blue() ));      
    ::SelectObject( currentGc_, brush );      

    ::LOGPEN logPen;
    ::GetObject( hPen, sizeof(logPen), &logPen );
    ::DeleteObject( hPen );
    hPen = ::CreatePen( logPen.lopnStyle, logPen.lopnWidth.x, RGB( color.red(), color.green(), color.blue()));

    ::SelectObject( currentGc_, hPen );
#endif               
    oldColor.setRGB( color.red(), color.green(), color.blue() );
  }
}

void Graphics::setFont( const Font& font )
{
  if ( !currentGc_ ) return;

  fntStruct = font.platformFontStructure();   
#ifdef __unix__
  if ( !fntStruct.antialias ) {
    XSetFont( App::system().dpy(), currentGc_, fntStruct.xFnt->fid );
  }
#else
  SelectObject( currentGc_, fntStruct.hFnt ); 
#endif
}

void Graphics::drawXftString( int x, int y, const char * s )
{
#ifdef __unix__
  if (dblBuffer_)
    XftDrawString8(drawDbl, &fFtColor, fntStruct.xftFnt , x, y,reinterpret_cast<const FcChar8 *>(s), strlen(s));
  else
    XftDrawString8(drawWin, &fFtColor, fntStruct.xftFnt , x, y,reinterpret_cast<const FcChar8 *>(s), strlen(s));
#endif
}

void Graphics::drawText( int x, int y, const std::string & text )
{
  if ( !currentGc_ ) return;

#ifdef __unix__
  if (!fntStruct.antialias)  {
    if (!(fntStruct.textColor==oldColor))
      XSetForeground( App::system().dpy(), currentGc_, fntStruct.textColor.colorValue() );
    XDrawString(App::system().dpy(), currentDrawable_, currentGc_, x+dx_, y+dy_, text.c_str(),strlen(text.c_str()) );
    setForeground(old);
  } else {
    Color color;
    color = fntStruct.textColor;
    fFtColor.color.red   = color.red() * ( 0xFFFF/255);
    fFtColor.color.green = color.green() * ( 0xFFFF/255);
    fFtColor.color.blue  = color.blue() * ( 0xFFFF/255);
    drawXftString( x+dx_, y+dy_, text.c_str() );
  }
#else    
  SetBkMode( currentGc_, TRANSPARENT );
  SetTextAlign( currentGc_, TA_BASELINE );
  SetTextColor( currentGc_, fntStruct.textColor.hColorRef() );
  TextOut( currentGc_, x + dx_, y+ dy_, text.c_str(), static_cast<int>( text.length() ) );
#endif
}

void Graphics::drawText(int x, int y, const std::string & text, const Color& color ) {
  if ( !currentGc_ ) return;

#ifdef __unix__
  if (!fntStruct.antialias)  {
    XSetForeground( App::system().dpy(), currentGc_, color.colorValue() );
    XDrawString( App::system().dpy(), currentDrawable_, currentGc_, x+dx_,y+dy_,text.c_str(),strlen(text.c_str()));
  } else
  {
    fFtColor.color.red   = color.red() * ( 0xFFFF/255);
    fFtColor.color.green = color.green() * ( 0xFFFF/255);
    fFtColor.color.blue  = color.blue() * ( 0xFFFF/255);
    drawXftString( x+dx_, y+dy_, text.c_str() );
  }
#else
  SetBkMode( currentGc_, TRANSPARENT );
  SetTextAlign( currentGc_, TA_BASELINE );
  SetTextColor( currentGc_, color.hColorRef() );
  TextOut( currentGc_, x + dx_, y+ dy_, text.c_str(), static_cast<int>( text.length() ) );
#endif
}

void Graphics::drawRect( int x, int y, int width, int height )
{
  if ( !currentGc_ ) return;

  // Note : XFillRectangle has the same
  // width as Rectangle, but XDrawRectangle is one pixel wider
  // for the same co-ordinates.   
#ifdef __unix__
  XDrawRectangle( App::system().dpy(), currentDrawable_, currentGc_, x+dx_, y+dy_, width, height );
#else
  HBRUSH holdbrush = (HBRUSH) SelectObject( currentGc_, hollow );
  Rectangle( currentGc_, x + dx_,y + dy_, x + dx_ + width + 1, y + dy_ +height + 1 );
  SelectObject( currentGc_, holdbrush );
#endif
}

void Graphics::drawRect( const Rect& rect )
{
  if ( !currentGc_ ) return;

  drawRect( rect.left(), rect.top(), rect.width(), rect.height() );
}

void Graphics::drawLine( long x, long y, long x1, long y1 )
{
  if ( !currentGc_ ) return;

#ifdef __unix__
  XDrawLine(App::system().dpy(), currentDrawable_, currentGc_, x+dx_, y+dy_, x1+dx_, y1+dy_);
#else
  MoveToEx( currentGc_, x + dx_, y + dy_, NULL);
  LineTo( currentGc_, x1 + dx_, y1 + dy_ );
#endif
}

void Graphics::drawPolygon( NPoint* pts , int n )
{
  if ( !currentGc_ ) return;

  int p2x = 0;
  int p2y = 0;
  for (int i = 0; i < n; i++) {
    int p1x = pts[i].x();
    int p1y = pts[i].y();
    if (i<n-1)  {
      p2x = pts[i+1].x(); 
      p2y = pts[i+1].y();
    } else { p2x = pts[0].x(); p2y = pts[0].y(); }
    drawLine(p1x,p1y,p2x,p2y);
  }
}

void Graphics::fillPolygon( NPoint * pts, int n )
{
  if ( !currentGc_ ) return;

#ifdef __unix__
  XPoint* pt = new XPoint[n];
#else
  POINT* pt = new POINT[n];
#endif
  for (int i = 0; i< n; i++) {
    pt[i].x = pts[i].x() +  dx_;
    pt[i].y = pts[i].y() +  dy_;
  }
#ifdef __unix__
  XFillPolygon(App::system().dpy(), currentDrawable_, currentGc_, pt, n, Complex, CoordModeOrigin );
#else   
  Polygon( currentGc_, pt, n );
#endif

  delete[] pt;
}

ngrs::Region Graphics::region( )
{
  return region_;
}

void Graphics::setRegion( const ngrs::Region& region )
{
  region_ = region;
}

void Graphics::setClipping( const ngrs::Region& region )
{
  if ( !currentGc_ ) return;

#ifdef __unix__
  XSetRegion( App::system().dpy(), currentGc_, region.asPlatformRegionHandle() );
  if (dblBuffer_) {
    XftDrawSetClip( drawDbl, region.asPlatformRegionHandle() );
  } else {
    XftDrawSetClip( drawWin, region.asPlatformRegionHandle() );
  }
#else
  ::SelectClipRgn( currentGc_, region.asPlatformRegionHandle() );
#endif
}

void Graphics::fillTranslucent( int x, int y, int width, int height, Color color, int percent )
{
  if ( !currentGc_ ) return;

#ifdef __unix__
  XImage* xi = XGetImage(App::system().dpy(), currentDrawable_, x+dx_,y+dy_,width,height,AllPlanes,ZPixmap );
  unsigned char* data = (unsigned char*) xi->data;
  int pixelsize = App::system().pixelSize( App::system().depth() );
  double anteil = percent / 100.0f;
  if (pixelsize == 4)
    for (int i = 0; i < xi->width*xi->height*pixelsize;i=i+pixelsize) {
      unsigned char r = data[i];
      unsigned char g = data[i+1];
      unsigned char b = data[i+2];

      data[i]   =  (int) ((anteil*r + (1 - anteil) * color.red()));
      data[i+1] =  (int) ((anteil*g + (1 - anteil) * color.green()));
      data[i+2] =  (int) ((anteil*b + (1 - anteil) * color.blue()));
    }
    XPutImage( App::system().dpy(), currentDrawable_, currentGc_, xi, 0, 0, x+dx_, y+dy_, xi->width,xi->height );
    XDestroyImage(xi);
#else
  //todo 
#endif

}

int Graphics::textWidth( const std::string& text ) const
{
  if ( !currentGc_ ) return 0;

#ifdef __unix__
  const char* s = text.c_str();
  if (!fntStruct.antialias) {
    return XTextWidth(fntStruct.xFnt,s,strlen(s));
  } else
  {
    XGlyphInfo info;
    XftTextExtents8(App::system().dpy(),fntStruct.xftFnt
      ,reinterpret_cast<const FcChar8 *>(s),strlen(s),&info);
    return info.xOff;
  }
#else 
  SIZE size;
  GetTextExtentPoint32( currentGc_, text.c_str(), static_cast<int>( text.length() ), &size);
  return size.cx;
#endif
}

int Graphics::textHeight() const
{
  if ( !currentGc_ ) return 0;

#ifdef __unix__
  if (!fntStruct.antialias)
    return (fntStruct.xFnt->max_bounds.ascent+ fntStruct.xFnt->max_bounds.descent);
  else
    return fntStruct.xftFnt->ascent + fntStruct.xftFnt->descent + 1;
#else
  TEXTMETRIC metrics;
  GetTextMetrics( currentGc_, &metrics );
  return metrics.tmHeight; 
#endif
}

int Graphics::textAscent( ) const
{ 
  if ( !currentGc_ ) return 0;

#ifdef __unix__
  if (!fntStruct.antialias)
    return ( fntStruct.xFnt->max_bounds.ascent );
  else 
    return fntStruct.xftFnt->ascent;
#else
  TEXTMETRIC metrics;
  GetTextMetrics( currentGc_, &metrics );
  return metrics.tmAscent; 
#endif
}

int Graphics::textDescent( ) const
{
  if ( !currentGc_ ) return 0;

#ifdef __unix__
  if ( !fntStruct.antialias)
    return ( fntStruct.xFnt->max_bounds.descent );
  else 
    return fntStruct.xftFnt->descent;
#else
  TEXTMETRIC metrics;
  GetTextMetrics( currentGc_, &metrics );
  return metrics.tmDescent; 
#endif
}

void Graphics::putStretchBitmap(int x, int y, const Bitmap& bitmap, int width, int height ) {
  // todo
}

void Graphics::putBitmap( int x, int y, const Bitmap& bitmap )
{
  if ( !currentGc_ ) return;
#ifdef __unix__
  if (bitmap.sysData() != 0) {
    if (bitmap.clpData()==0) {
      XPutImage(App::system().dpy(), currentDrawable_, currentGc_, bitmap.sysData(),0, 0, x+dx_,y+dy_, bitmap.width(),bitmap.height());
    } else
    {
      // transparent bitmap;
      XImage* clp = bitmap.clpData();
      ::Pixmap pix;
      pix = XCreatePixmap(App::system().dpy(), currentDrawable_, clp->width, clp->height, clp->depth);

      GC gc1 = XCreateGC (App::system().dpy(), pix, 0, NULL);
      XPutImage( App::system().dpy(), pix, gc1, clp, 0, 0, 0, 0, clp->width, clp->height);

      XSetClipMask(App::system().dpy(), currentGc_, pix);
      XSetClipOrigin(App::system().dpy(), currentGc_, x+dx_, y+dy_);
      // todo valgrind check error on some images
      XPutImage(App::system().dpy(), currentDrawable_, currentGc_, bitmap.sysData(), 0, 0, x+dx_, y+dy_, bitmap.width(), bitmap.height() );
      // valgrind check error end
      XSetClipMask(App::system().dpy(), currentGc_, None);
      XFreeGC( App::system().dpy(), currentGc_ );
      XFreePixmap( App::system().dpy(), pix );
    }      
  }
#else
  if ( bitmap.clpData() ) {  
    // bitmap is transparent      
    HDC hdcMem = CreateCompatibleDC( currentGc_ );
    HBITMAP hbmOld = (HBITMAP) SelectObject(hdcMem, bitmap.clpData() );
    SelectObject( hdcMem, bitmap.clpData() );
    BitBlt( currentGc_, x+dx_, y+dy_, bitmap.width(), bitmap.height(), hdcMem , 0,0, SRCAND);    
    SelectObject( hdcMem, bitmap.sysData() );
    BitBlt( currentGc_, x+dx_, y+dy_, bitmap.width(), bitmap.height(), hdcMem , 0,0, SRCPAINT);
    SelectObject(hdcMem, hbmOld);
    DeleteDC(hdcMem);
  } else
    if ( bitmap.sysData() ) {    
      HDC hdcMem = CreateCompatibleDC( currentGc_ );
      HBITMAP hbmOld = (HBITMAP) SelectObject(hdcMem, bitmap.sysData() );
      BitBlt( currentGc_, x+dx_, y+dy_, bitmap.width(), bitmap.height(), hdcMem , 0,0, SRCCOPY);
      SelectObject(hdcMem, hbmOld);
      DeleteDC(hdcMem);
    }  
#endif
}

void Graphics::putBitmap( int destX, int destY, int width, int height, const Bitmap & bitmap, int srcX, int srcY )
{
  if ( !currentGc_ ) return;
#ifdef __unix__
  if ( bitmap.sysData() ) {
    XPutImage(App::system().dpy(), currentDrawable_, currentGc_, bitmap.sysData(),
      srcX, srcY, destX+dx_,destY+dy_, width, height);
  }
#else
  if ( bitmap.sysData() ) {    
    HDC hdcMem = CreateCompatibleDC( currentGc_ );
    HBITMAP hbmOld = (HBITMAP) SelectObject( hdcMem, bitmap.sysData() );
    BitBlt( currentGc_, destX+dx_, destY+dy_, width, height, hdcMem , srcX, srcY, SRCCOPY);
    SelectObject( hdcMem, hbmOld );
    DeleteDC(hdcMem);
  } 
#endif
}

void Graphics::copyArea( int src_x, int src_y, unsigned width, unsigned height, int dest_x, int dest_y )
{
#ifdef __unix__
  if (width  && height) {
    XCopyArea( App::system().dpy(), currentDrawable_, currentDrawable_, currentGc_,
      src_x, src_y, width, height, dest_x, dest_y ); 
  }
#else
  if (width && height) {      
    BitBlt( currentGc_, dest_x+dx_, dest_y+dy_, width, height, currentGc_, src_x, src_y, SRCCOPY );
  }          
#endif
}

void Graphics::setDoubleBuffer( bool on )
{
  if (dblBuffer_ == on) return;
  if (dblBuffer_ && !on) {
    dblBuffer_ = on;
  } else
    if (!dblBuffer_ && on) {
      dblBuffer_ = on;
    }    
  updateCurrentGc();
}

#ifdef __unix__
::Pixmap Graphics::dbPixmap( )
{
  return doubleBufferPixmap_;
}
#endif

GC Graphics::dbGC( )
{
  return gcp;
}

GC Graphics::gc( )
{
  return gc_;
}


void Graphics::fillGradient(int x, int y, int width, int height, const Color & start, const  Color & end , int direction) {
  if ( !currentGc_ ) return;

  int middle = (direction == nHorizontal) ? width : height;

  int r1 = start.red();
  int g1 = start.green();
  int b1 = start.blue();

  int r2 = end.red();
  int g2 = end.green();
  int b2 = end.blue();

  double dr = (r2-r1) / (double) middle;
  double dg = (g2-g1) / (double) middle;
  double db = (b2-b1) / (double) middle;


  if (direction == nHorizontal)
    for (int i = 0; i < middle; i++) {
      setForeground(Color((int) (r1 + i*dr),(int) (g1 + i*dg),(int) (b1 + i*db)));
      fillRect(x+i,y,1,height);
    }
  else
    for (int i = 0; i < middle; i++) {
      setForeground(Color((int) (r1 + i*dr),(int) (g1 + i*dg), (int) (b1 + i*db)));
      fillRect(x,y+i,width,1);
    }
}


const ngrs::Region & Graphics::repaintArea( ) const
{
  return repaintArea_;
}

void Graphics::setRepaintArea( const ngrs::Region & region )
{
  repaintArea_ = region;
}

int Graphics::dblWidth( ) const
{
  return dblWidth_;
}

int Graphics::dblHeight( ) const
{
  return dblHeight_;
}


void Graphics::drawRoundRect( int x, int y, int width, int height, int arcWidth, int arcHeight ) {
if ( !currentGc_ ) return;

#ifdef __unix__
  int nx = x;
  int ny = y;
  int nw = width;
  int nh = height;
  int naw = arcWidth;
  int nah = arcHeight;

  if (nw < 0) { 
    nw = 0 - nw;
    nx = nx - nw;
  }
  if (nh < 0) {
    nh = 0 - nh;
    ny = ny - nh;
  }
  if (naw < 0)  naw = 0 - naw;
  if (nah < 0)  nah = 0 - nah;

  int naw2 = naw / 2;
  int nah2 = nah / 2;

  if (nw > naw) {
    if (nh > nah) {
      drawArc(nx, ny, naw, nah, 5760, 5760);
      drawLine(nx + naw2, ny, nx + nw - naw2, ny);
      drawArc(nx + nw - naw, ny, naw, nah, 0, 5760);
      drawLine(nx + nw, ny + nah2, nx + nw, ny + nh - nah2);
      drawArc(nx + nw - naw, ny + nh - nah, naw, nah, 17280, 5760);
      drawLine(nx + naw2, ny + nh, nx + nw - naw2, ny + nh);
      drawArc(nx, ny + nh - nah, naw, nah, 11520, 5760);
      drawLine(nx, ny + nah2, nx, ny + nh - nah2);
    } else {
      drawArc(nx, ny, naw, nh, 5760, 11520);
      drawLine(nx + naw2, ny, nx + nw - naw2, ny);
      drawArc(nx + nw - naw, ny, naw, nh, 17280, 11520);
      drawLine(nx + naw2, ny + nh, nx + nw - naw2, ny + nh);
    }
  } else {
    if (nh > nah) {
      drawArc(nx, ny, nw, nah, 0, 11520);
      drawLine(nx + nw, ny + nah2, nx + nw, ny + nh - nah2);
      drawArc(nx, ny + nh - nah, nw, nah, 11520, 11520);
      drawLine(nx, ny + nah2, nx, ny + nh - nah2);
    } else {
      drawArc(nx, ny, nw, nh, 0, 23040);
    }
  }
#else
  HBRUSH holdbrush = (HBRUSH) SelectObject( currentGc_, hollow );
  RoundRect( currentGc_, x + dx_, y + dy_ , x + dx_ + width + 1 , y + dy_ + height + 1, arcWidth, arcHeight );
  SelectObject( currentGc_, holdbrush );
#endif        
}

static double fTwoPi = 2.0 * 3.14; 


#ifdef __unix__
#else
void Graphics::drawArcX( int x, int y, int width, int height, int start, int extent, bool fill ) {
  if ( !currentGc_ ) return;

  int clockwise = (extent < 0); /* non-zero if clockwise */
  int xstart, ystart, xend, yend;
  double radian_start, radian_end, xr, yr;

  //
  // Compute the absolute starting and ending angles in normalized radians.
  // Swap the start and end if drawing clockwise.
  //

  start = start % (64*360);
  if (start < 0) {
    start += (64*360);
  }
  extent = (start+extent) % (64*360);
  if (extent < 0) {
    extent += (64*360);
  }
  if (clockwise) {
    int tmp = start;
    start = extent;
    extent = tmp;
  }
  radian_start = XAngleToRadians(start);
  radian_end = XAngleToRadians(extent);

  //
  // Now compute points on the radial lines that define the starting and
  // ending angles.  Be sure to take into account that the y-coordinate
  // system is inverted.
  //

  xr = x + width / 2.0;
  yr = y + height / 2.0;
  xstart = (int)((xr + cos(radian_start)*width/2.0) + 0.5);
  ystart = (int)((yr + sin(-radian_start)*height/2.0) + 0.5);

  xend = (int)((xr + cos(radian_end)*width/2.0) + 0.5);
  yend = (int)((yr + sin(-radian_end)*height/2.0) + 0.5);

  if ( !fill ) {
    Arc( currentGc_, x , y  , x + width +1, y + height+1, xstart, ystart, xend, yend );
  } else {
    Chord( currentGc_, x, y, x+width+1, y+height+1, xstart, ystart, xend, yend);
  }    
}
#endif

void Graphics::drawArc( int x, int y, int width, int height, int start, int extent )
{
  if ( !currentGc_ ) return;

#ifdef __unix__
  XDrawArc( App::system().dpy(), currentDrawable_, currentGc_, x+dx_, y+dy_, width, height, start, extent );
#else
  drawArcX( x +dx_, y+ dy_, width, height, start, extent , 0 );
#endif     
}

void Graphics::fillArc( int x, int y, int width, int height, int angle1, int angle2 )
{
  if ( !currentGc_ ) return;

#ifdef __unix__   
  XFillArc( App::system().dpy(), currentDrawable_, currentGc_, x+dx_, y+dy_, width, height, angle1, angle2);
#else
  drawArcX( x + dx_, y + dy_, width, height, angle1, angle2, 1 );
#endif
}

void Graphics::fillRect( const Rect& rect )
{
  fillRect( rect.left(), rect.top(), rect.width(), rect.height() );
}

void Graphics::fillRoundRect( int x, int y, int width, int height, int arcWidth, int arcHeight )
{
  if ( !currentGc_ ) return;

  int nx = x;
  int ny = y;
  int nw = width;
  int nh = height;
  int naw = arcWidth;
  int nah = arcHeight;

  if (nw < 0) { 
    nw = 0 - nw;
    nx = nx - nw;
  }
  if (nh < 0) {
    nh = 0 - nh;
    ny = ny - nh;
  }
  if (naw < 0)  naw = 0 - naw;
  if (nah < 0)  nah = 0 - nah;

  int naw2 = naw / 2;
  int nah2 = nah / 2;

  if (nw > naw) {
    if (nh > nah) {
      fillArc(nx, ny, naw, nah, 5760, 5760);
      fillArc(nx + nw - naw, ny, naw, nah, 0, 5760);
      fillArc(nx + nw - naw, ny + nh - nah, naw, nah, 17280,5760);
      fillArc(nx, ny + nh - nah, naw, nah, 11520, 5760);
      fillRect(nx,ny+nah2,nw,nh- 2*nah2);
      fillRect(nx+naw2,ny,nw-2*naw2,nah2);
      fillRect(nx+naw2,ny+nh-nah2,nw-2*naw2,nah2);
    } else {
      fillArc(nx, ny, naw, nh, 5760, 11520);
      fillArc(nx + nw - naw, ny, naw, nh, 17280, 11520);
      fillRect(nx+naw2,ny,nw-2*naw2,nah2);
    }
  } else {
    if (nh > nah) {
      fillArc(nx, ny, nw, nah, 0, 11520);
      fillArc(nx, ny + nh - nah, nw, nah, 11520, 11520);
      fillRect(nx+naw2,ny+nh-nah2,nw-2*naw2,nah2);
    } else {
      fillArc(nx, ny, nw, nh, 0, 23040);
    }
  }
}

void Graphics::fillRoundGradient( int x, int y, int width, int height, const Color & start, const Color & end, int direction, int arcWidth, int arcHeight )
{
  if ( !currentGc_ ) return;

  int nx = x;
  int ny = y;
  int nw = width;
  int nh = height;
  int naw = arcWidth;
  int nah = arcHeight;

  if (nw < 0) { 
    nw = 0 - nw;
    nx = nx - nw;
  }
  if (nh < 0) {
    nh = 0 - nh;
    ny = ny - nh;
  }
  if (naw < 0)  naw = 0 - naw;
  if (nah < 0)  nah = 0 - nah;

  int naw2 = naw / 2;
  int nah2 = nah / 2;

  if (nw > naw) {
    if (nh > nah) {
      if (direction == nHorizontal) {
        setForeground(start);
        fillArc(nx, ny, naw, nah, 5760, 5760);
        fillArc(nx, ny + nh - nah, naw, nah, 11520, 5760);
        setForeground(end);
        fillArc(nx + nw - naw, ny, naw, nah, 0, 5760);
        fillArc(nx + nw - naw, ny + nh - nah, naw, nah, 17280,5760);
      } else {
        fillArc(nx, ny, naw, nah, 5760, 5760);
        fillArc(nx + nw - naw, ny, naw, nah, 0, 5760);
        setForeground(end);
        fillArc(nx, ny + nh - nah, naw, nah, 11520, 5760);
        fillArc(nx + nw - naw, ny + nh - nah, naw, nah, 17280,5760);
      }
      fillGradient(nx,ny+nah2,nw,nh- 2*nah2,start,end,direction);
      fillGradient(nx+naw2,ny,nw-2*naw2,nah2,start,end,direction);
      fillGradient(nx+naw2,ny+nh-nah2,nw-2*naw2,nah2,start,end,direction);
    } else {
      fillArc(nx, ny, naw, nh, 5760, 11520);
      fillArc(nx + nw - naw, ny, naw, nh, 17280, 11520);
      fillGradient(nx+naw2,ny,nw-2*naw2,nah2,start,end,direction);
    }
  } else {
    if (nh > nah) {
      fillArc(nx, ny, nw, nah, 0, 11520);
      fillArc(nx, ny + nh - nah, nw, nah, 11520, 11520);
      fillGradient(nx+naw2,ny+nh-nah2,nw-2*naw2,nah2,start,end,direction);
    } else {
      fillArc(nx, ny, nw, nh, 0, 23040);
    }
  }
}

void Graphics::fillGradient( int x, int y, int width, int height, const Color& start, const Color& mid, const Color& end, int direction, int percent )
{
  if ( !currentGc_ ) return;

  int middle  = 0;
  int length  = 0;
  if (direction == nHorizontal) {
    middle = (int) (width  * percent/100.0f);
    length = width  - middle;
  } else {
    middle = (int) (height * (percent/100.0f));
    length = height - middle;
  }
  //frist part to middle
  if (direction == nHorizontal)
    fillGradient(x,y,middle,height, start,mid,nHorizontal);
  else 
    fillGradient(x,y,width,middle,start,mid,nVertical);
  // second part from middle to end
  if (direction == nHorizontal)
    fillGradient(x+middle,y,width-middle,height,mid,end,nHorizontal);
  else
    fillGradient(x,y+middle,width,height-middle,mid,end,nVertical);

}

void Graphics::fillRoundGradient( int x, int y, int width, int height, const Color & start, const Color & mid, const Color & end, int direction, int percent , int arcWidth, int arcHeight)
{
  if ( !currentGc_ ) return;

  int nx = x;
  int ny = y;
  int nw = width;
  int nh = height;
  int naw = arcWidth;
  int nah = arcHeight;

  if (nw < 0) { 
    nw = 0 - nw;
    nx = nx - nw;
  }
  if (nh < 0) {
    nh = 0 - nh;
    ny = ny - nh;
  }
  if (naw < 0)  naw = 0 - naw;
  if (nah < 0)  nah = 0 - nah;

  int naw2 = naw / 2;
  int nah2 = nah / 2;

  if (nw > naw) {
    if (nh > nah) {
      if (direction == nHorizontal) {
        setForeground(start);
        fillArc(nx, ny, naw, nah, 5760, 5760);
        fillArc(nx, ny + nh - nah, naw, nah, 11520, 5760);
        setForeground(end);
        fillArc(nx + nw - naw, ny, naw, nah, 0, 5760);
        fillArc(nx + nw - naw, ny + nh - nah, naw, nah, 17280,5760);
        fillGradient(nx+naw2,ny,nw-2*naw2,nah2,start,mid,end,direction,percent);
        fillGradient(nx+naw2,ny+nh-nah2,nw-2*naw2,nah2,start,mid,end,direction,percent);
      } else {
        setForeground(start);
        fillArc(nx, ny, naw, nah, 5760, 5760);
        fillArc(nx + nw - naw, ny, naw, nah, 0, 5760);
        fillRect(nx+naw2,ny,nw-2*naw2,nah2);
        setForeground(end);
        fillArc(nx, ny + nh - nah, naw, nah, 11520, 5760);
        fillArc(nx + nw - naw, ny + nh - nah, naw, nah, 17280,5760);
        fillRect(nx+naw2,ny+nh-nah2,nw-2*naw2,nah2);
      }
      fillGradient(nx,ny+nah2,nw,nh- 2*nah2,start,mid,end,direction,percent);
    } else {
      fillArc(nx, ny, naw, nh, 5760, 11520);
      fillArc(nx + nw - naw, ny, naw, nh, 17280, 11520);
      fillGradient(nx+naw2,ny,nw-2*naw2,nah2,start,mid,end,direction,percent);
    }
  } else {
    if (nh > nah) {
      fillArc(nx, ny, nw, nah, 0, 11520);
      fillArc(nx, ny + nh - nah, nw, nah, 11520, 11520);
      fillGradient(nx+naw2,ny+nh-nah2,nw-2*naw2,nah2,start,mid,end,direction,percent);
    } else {
      fillArc(nx, ny, nw, nh, 0, 23040);
    }
  }
}

int Graphics::textWidth( const FntString& text ) const
{
  if ( !currentGc_ ) return 0;

#ifdef __unix__
  FontStructure newFntStruct = fntStruct;
  int pos = 0; int w = 0;
  std::vector<Font>::const_iterator fntIt = text.fonts().begin();
  for (std::vector<std::string::size_type>::const_iterator it = text.positions().begin(); it < text.positions().end(); it++) {
    std::string::size_type old = pos;
    pos = *it;
    const char* s = text.textsubstr( old, pos-old ).c_str();
    if ( !newFntStruct.antialias )
      w+=XTextWidth(newFntStruct.xFnt,s,strlen(s));
    else {
      XGlyphInfo info;
      XftTextExtents8(App::system().dpy(),newFntStruct.xftFnt
        ,reinterpret_cast<const FcChar8 *>(s),strlen(s),&info);
      w+= info.xOff;
    }

    newFntStruct = (*fntIt).platformFontStructure();
    fntIt++;
  }
  const char* s = text.textsubstr(pos).c_str();

  if (!newFntStruct.antialias)
    w+=XTextWidth(newFntStruct.xFnt,s,strlen(s));
  else {
    XGlyphInfo info;
    XftTextExtents8(App::system().dpy(),newFntStruct.xftFnt
      ,reinterpret_cast<const FcChar8 *>(s),strlen(s),&info);
    w+= info.xOff;
  }

  return w;
#else
  ///\todo port
  return 0;
#endif
}

void Graphics::drawText( int x, int y, const FntString& text )
{
  if ( !currentGc_ ) return;

  std::string::size_type pos = 0; 
  int w = 0;
  std::vector<Font>::const_iterator fntIt = text.fonts().begin();
  for ( std::vector<std::string::size_type>::const_iterator it = text.positions().begin(); it < text.positions().end(); it++) {
    std::string::size_type old = pos;
    pos = *it;
    drawText( x+w, y, text.textsubstr( old, pos - old ));
    w += textWidth(text.textsubstr( old, pos-old ));
    setFont( *fntIt );
    fntIt++;
  }
  drawText( x+w, y, text.textsubstr( pos ) );
}

/*

int Graphics::findWidthMax(long width, const FntString & data, bool wbreak) const
{
int Low = 0; int High = data.length();  int Mid=High;
while( Low <= High ) {
Mid = ( Low + High ) / 2;
FntString s     = data.substr(0,Mid);
FntString snext;
if (Mid>0) snext  = data.substr(0,Mid+1); else snext = s;
int w     = textWidth(s);
if(  w < width  ) {
int wnext = textWidth(snext);
if (wnext  >= width ) break;
Low = Mid + 1; 
} else
{
High = Mid - 1;
}
}
if (!wbreak || data.textsubstr(0,Mid).find(" ")==std::string::npos || Mid == 0 || Mid>=data.length()) return Mid; else
{
std::string::size_type p = data.rfind(" ",Mid);
if ( p != std::string::npos ) return p+1;
}
return Mid;

}*/

void Graphics::setVisible( bool on )
{
  visible_ = on;
  if (!on) {
    destroyDblBufferHandles();    
    dblBuffer_ = false;
    updateCurrentGc();
  } else {
    dblBuffer_ = true;
    createDblBufferHandles();
    updateCurrentGc();
  }
}

void Graphics::putPixmap( int destX, int destY, int width, int height, Pixmap & pixmap, int srcX, int srcY )
{
  if ( !currentGc_ ) return;

#ifdef __unix__
  if ( pixmap.X11Pixmap() ) {
    XCopyArea( App::system().dpy(),  pixmap.X11Pixmap(), currentDrawable_, currentGc_,
      srcX, srcY, width, height, destX+dx_,destY+dy_);
  }
#else
  putBitmap( destX, destY, width, height, ( Bitmap& ) pixmap, srcX, srcY )  ;
#endif
}

// sets and gets the pen (line style etc ..)
void Graphics::setPen( const Pen& pen )
{
  if ( !currentGc_ ) return;

#ifdef __unix__
  XSetLineAttributes(App::system().dpy(), currentGc_, pen.lineWidth(), (int) pen.lineStyle(), pen.capStyle(), pen.joinStyle() );
  XSetFillStyle(App::system().dpy(), currentGc_, pen.fillStyle() );
  XSetFunction(App::system().dpy(), currentGc_, pen.function() );
#else
  LOGPEN logPen;
  GetObject( hPen, sizeof(logPen), &logPen );
  logPen.lopnWidth.x = pen.lineWidth();
  logPen.lopnStyle = pen.lineStyle();
  DeleteObject( hPen ) ;
  hPen = CreatePenIndirect( &logPen );
  SelectObject( currentGc_, hPen );
#endif
}

void Graphics::resetPen( )
{
  setPen( Pen() );
}

}
