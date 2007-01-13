/***************************************************************************
 *   Copyright (C) 2005, 2006, 2007 by Stefan Nattkemper  *
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
#include "ncolor.h"
#include "napp.h"
#include <algorithm>

namespace ngrs {

  NColor::NColor() : r( 0 ), g( 0 ), b( 0 )
  {
    value = NApp::system().getXColorValue( r, g, b );
  }

  NColor::NColor( unsigned int red, unsigned int green, unsigned int blue ) 
    : r( red < 256   ? red : 0 ), 
    g( green < 256 ? green : 0 ),
    b( blue < 256  ? blue : 0 )
  {  
    value = NApp::system().getXColorValue( r, g, b );
  }

  NColor::NColor( const std::string & rgbStr )
  {
    setRGB( rgbStr );
  }

  NColor::NColor( long hcolorref )
  {
    b = (hcolorref>>16) & 0xff;
    g = (hcolorref>>8 ) & 0xff;
    r = (hcolorref    ) & 0xff;
    value = NApp::system().getXColorValue( r, g, b );
  }

  NColor::~NColor()
  {
  }

  unsigned int NColor::red( ) const
  {
    return r;
  }

  unsigned int NColor::green( ) const
  {
    return g;
  }

  unsigned int NColor::blue( ) const
  {
    return b;
  }

  unsigned long NColor::colorValue( ) const
  {
    return value;
  }

  bool NColor::operator ==( const NColor & color ) const
  {
    return ( (color.blue() == blue()) && (color.red() == red()) && (color.green() == green()) );
  }

  bool NColor::operator <( const NColor & rhs ) const
  {
    return ( r | g<<8 | b<<16 )  <  ( rhs.red() | rhs.green()<<8 | rhs.blue()<<16 );
  }

  void NColor::setRGB( unsigned int red, unsigned int green, unsigned int blue )
  {  
    value = NApp::system().getXColorValue( r = red, g = green, b = blue );
  }

  void NColor::setRGB( const std::string & rgbStr )
  {
    std::string tok = rgbStr;
    replace( tok.begin(), tok.end(), ':', ' ' ); 
    std::istringstream( tok ) >> r >> g >> b;
    value = NApp::system().getXColorValue( r, g, b );
  }

  void NColor::setHCOLORREF( long int hcolorref )
  {
    b = ( hcolorref>>16 ) & 0xff;
    g = ( hcolorref>>8  ) & 0xff;
    r = ( hcolorref     ) & 0xff;
    value = NApp::system().getXColorValue(r,g,b);
  }

  long NColor::hColorRef( ) const
  {
    return ( b << 16 ) | ( g << 8 ) | r;
  }

}
