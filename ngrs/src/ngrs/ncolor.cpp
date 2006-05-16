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
#include "ngrs/ncolor.h"
#include "ngrs/napp.h"

using namespace std;


NColor::NColor() : r(0),g(0),b(0)
{
  value = NApp::system().getXColorValue(r,g,b);
}

NColor::NColor(int red,int green,int blue) : r(red), g(green), b(blue)
{
  if (r  < 0)  r  = 0;
  if (b  < 0)  b = 0;
  if (g  < 0 ) g = 0;

  if (r  > 255)  r = 255;
  if (b  > 255)  b = 255;
  if (g  > 255 ) g = 255;

  value = NApp::system().getXColorValue(r,g,b);
}

NColor::~NColor()
{
}

int NColor::red( ) const
{
  return r;
}

int NColor::green( ) const
{
  return g;
}

int NColor::blue( ) const
{
  return b;
}

unsigned long NColor::colorValue( ) const
{
  return value;
}

bool NColor::operator ==( const NColor & color )
{
  return ( (color.blue() == blue()) && (color.red() == red()) && (color.green() == green()) );
}

void NColor::setRGB( int red, int green, int blue )
{
  r = red;
  g = green;
  b = blue;
  value = NApp::system().getXColorValue(r,g,b);
}

bool NColor::operator <( const NColor & cl ) const
{
  long key1 = r | g<<8 | b<<16;
  long key2 = cl.red() | cl.green()<<8 | cl.blue()<<16;
  return key1 < key2;
}

void NColor::setRGB( const std::string & rgbStr )
{
  int r = 0; 
  int g = 0;
  int b = 0;

  unsigned int i = 0;
  int start = 0;
  std::string substr;
  int c = 0;
  do {
    i = rgbStr.find(':', i);
    if (i != std::string::npos) {
       substr = rgbStr.substr(start,i-start);
       i+=1;
       start = i;
    } else substr = rgbStr.substr(start);
      std::stringstream str; str << substr; int value = 0; str >> value;
      if (c==0) r = value;
      if (c==1) g = value;
      if (c==2) b = value;
    c++;
  } while (i != std::string::npos);

  setRGB(r,g,b);
}

NColor::NColor( const std::string & rgbStr )
{
  setRGB(rgbStr);
}

void NColor::setHCOLORREF( long int hcolorref )
{
  b = (hcolorref>>16) & 0xff;
  g = (hcolorref>>8 ) & 0xff;
  r = (hcolorref    ) & 0xff;
  value = NApp::system().getXColorValue(r,g,b);
}





