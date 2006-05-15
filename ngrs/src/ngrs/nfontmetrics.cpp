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
#include "nfontmetrics.h"
#include "napp.h"

using namespace std;

NFontMetrics::NFontMetrics( )
{
  fntStruct = NApp::system().getXFontValues(NFont());
}

NFontMetrics::NFontMetrics(const NFont & font)
{
  fntStruct = font.systemFont();
}


NFontMetrics::~NFontMetrics()
{
}

int NFontMetrics::textWidth( const string & text )
{
   const char* s = text.c_str();
   if (!fntStruct.antialias) {
     return XTextWidth(fntStruct.xFnt,s,strlen(s));
   } else
   {
    XGlyphInfo info;
    XftTextExtents8(NApp::system().dpy(),fntStruct.xftFnt
     ,reinterpret_cast<const FcChar8 *>(s),strlen(s),&info);
    return info.xOff;
   }
}

int NFontMetrics::textHeight( )
{
 if (!fntStruct.antialias)
 {
   return (fntStruct.xFnt->max_bounds.ascent+ fntStruct.xFnt->max_bounds.descent);
 } else {
  int a = fntStruct.xftFnt->ascent;
  int d = fntStruct.xftFnt->descent;
  return a + d;
}
}

int NFontMetrics::textAscent( )
{
  if (!fntStruct.antialias)
  {
    return (fntStruct.xFnt->max_bounds.ascent);
  } else {
   int a = fntStruct.xftFnt->ascent;
   return a;
  }
}

int NFontMetrics::textDescent( )
{
 if (!fntStruct.antialias)
 {
   return (fntStruct.xFnt->max_bounds.descent);
 } else {
  int d = fntStruct.xftFnt->descent;
  return d;
 }
}

void NFontMetrics::setFont( const NFont & font )
{
  fntStruct = font.systemFont();
}



