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
#include "color.h"
#include "app.h"
#include <algorithm>

namespace ngrs {

  Color::Color() : r( 0 ), g( 0 ), b( 0 )
  {
    value = App::system().getXColorValue( r, g, b );
  }

  Color::Color( unsigned int red, unsigned int green, unsigned int blue ) 
    : r( red < 256   ? red : 0 ), 
    g( green < 256 ? green : 0 ),
    b( blue < 256  ? blue : 0 )
  {  
    value = App::system().getXColorValue( r, g, b );
  }

  Color::Color( const std::string & rgbStr )
  {
    setRGB( rgbStr );
  }

  Color::Color( long hcolorref )
  {
    b = (hcolorref>>16) & 0xff;
    g = (hcolorref>>8 ) & 0xff;
    r = (hcolorref    ) & 0xff;
    value = App::system().getXColorValue( r, g, b );
  }

  Color::~Color()
  {
  }

  unsigned int Color::red( ) const
  {
    return r;
  }

  unsigned int Color::green( ) const
  {
    return g;
  }

  unsigned int Color::blue( ) const
  {
    return b;
  }

  unsigned long Color::colorValue( ) const
  {
    return value;
  }

  bool Color::operator ==( const Color & color ) const
  {
    return ( (color.blue() == blue()) && (color.red() == red()) && (color.green() == green()) );
  }

  bool Color::operator <( const Color & rhs ) const
  {
    return ( r | g<<8 | b<<16 )  <  ( rhs.red() | rhs.green()<<8 | rhs.blue()<<16 );
  }

  void Color::setRGB( unsigned int red, unsigned int green, unsigned int blue )
  {  
    value = App::system().getXColorValue( r = red, g = green, b = blue );
  }

  void Color::setRGB( const std::string & rgbStr )
  {
    std::string tok = rgbStr;
    replace( tok.begin(), tok.end(), ':', ' ' ); 
    std::istringstream( tok ) >> r >> g >> b;
    value = App::system().getXColorValue( r, g, b );
  }

  void Color::setHCOLORREF( long int hcolorref )
  {
    b = ( hcolorref>>16 ) & 0xff;
    g = ( hcolorref>>8  ) & 0xff;
    r = ( hcolorref     ) & 0xff;
    value = App::system().getXColorValue(r,g,b);
  }

  long Color::hColorRef( ) const
  {
    return ( b << 16 ) | ( g << 8 ) | r;
  }

}
