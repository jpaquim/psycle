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
#include "fontmetrics.h"
#include "app.h"

using namespace std;

namespace ngrs {

  FontMetrics::FontMetrics( )
  {
    fntStruct = App::system().getFontValues(Font());
  }

  FontMetrics::FontMetrics(const Font & font)
  {
    fntStruct = font.platformFontStructure();
  }


  FontMetrics::~FontMetrics()
  {
  }

  int FontMetrics::textWidth( const string & text ) const
  {
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

    HDC dc = GetDC( NULL );
    HFONT oldFont = (HFONT)SelectObject( dc, fntStruct.hFnt );  

    SIZE size;
    GetTextExtentPoint32(
      dc,            // handle to DC
      text.c_str(),  // text string
      static_cast<int>( text.length() ), // characters in string
      &size          // string size
      );
	SelectObject( dc, oldFont );  
    ReleaseDC ( NULL, dc );

    return size.cx;

#endif
  }

  int FontMetrics::textHeight( ) const
  {
#ifdef __unix__
    if (!fntStruct.antialias)
    {
      return (fntStruct.xFnt->max_bounds.ascent+ fntStruct.xFnt->max_bounds.descent);
    } else {
      int a = fntStruct.xftFnt->ascent;
      int d = fntStruct.xftFnt->descent;
      return a + d;
    }
#else
    TEXTMETRIC metrics;
    HDC dc = GetDC( NULL );
    HFONT oldFont = (HFONT)SelectObject( dc, fntStruct.hFnt );  
    GetTextMetrics(
      dc ,      // handle to DC
      &metrics  // text metrics
      );
	SelectObject( dc, oldFont );  
    ReleaseDC( NULL , dc );
    return metrics.tmHeight;
#endif
  }

  int FontMetrics::textAscent( ) const
  {
#ifdef __unix__
    if (!fntStruct.antialias)
    {
      return (fntStruct.xFnt->max_bounds.ascent);
    } else {
      int a = fntStruct.xftFnt->ascent;
      return a;
    }
#else
    TEXTMETRIC metrics;
    HDC dc = GetDC(NULL); 
    HFONT oldFont = (HFONT)SelectObject( dc, fntStruct.hFnt );  
    GetTextMetrics(
      dc,       // handle to DC
      &metrics  // text metrics
      );  
	SelectObject( dc, oldFont ); 
    ReleaseDC( NULL , dc );
    return metrics.tmAscent;
#endif
  }

  int FontMetrics::textDescent( ) const
  {
#ifdef __unix__
    if (!fntStruct.antialias)
    {
      return (fntStruct.xFnt->max_bounds.descent);
    } else {
      int d = fntStruct.xftFnt->descent;
      return d;
    }
#else
    TEXTMETRIC metrics;
    HDC dc = GetDC(NULL);
    HFONT oldFont = (HFONT)SelectObject( dc, fntStruct.hFnt );  
    GetTextMetrics(
      dc,       // handle to DC
      &metrics  // text metrics
      );
	SelectObject( dc, oldFont ); 
    ReleaseDC( NULL , dc );
    return metrics.tmDescent;
#endif
  }

  void FontMetrics::setFont( const Font & font )
  {
    fntStruct = font.platformFontStructure();
  }

  int FontMetrics::maxCharWidth() const {
#ifdef __unix__
    ///\todo needs to be implement
    return 10;
#else
    TEXTMETRIC metrics;
    HDC dc = GetDC(NULL); 
    HFONT oldFont = (HFONT)SelectObject( dc, fntStruct.hFnt );  
    GetTextMetrics(
      dc,       // handle to DC
      &metrics  // text metrics
      );  
	SelectObject( dc, oldFont );
    ReleaseDC( NULL , dc );
    return metrics.tmMaxCharWidth;;
#endif
  }

  std::string::size_type FontMetrics::findWidthMax( long width, const std::string & data ) const
  {
    std::string::size_type low  = 0;
    std::string::size_type high = data.length();

    while( low < high ) {
      std::string::size_type mid = low + ( high - low ) / 2; 
      if(  textWidth( data.substr( 0, mid ) ) < width  ) {						 
        low = mid + 1; 
      } else
      {
        high = mid;
      }
    }  

    return low;
  }

}
