/***************************************************************************
 *   Copyright (C) 2005, 2006, 2007 by Stefan Nattkemper   *
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
#ifndef NCOLOR_H
#define NCOLOR_H

#include <map>
#include <iostream>
#include <sstream>
#include <string>

/**
@author Stefan
*/

namespace ngrs {

  class Color{
  public:
    Color();
    Color( unsigned int red, unsigned int green, unsigned int blue );
    Color( long hcolorref );
    Color( const std:: string & rgbStr );

    ~Color();

    unsigned int red() const;
    unsigned int green() const;
    unsigned int blue() const;

    void setRGB( unsigned int red, unsigned int green, unsigned int bblue );
    void setHCOLORREF( long hcolorref );
    void setRGB( const std::string & rgbStr );

    unsigned long colorValue() const;
    long hColorRef() const;

    bool operator==( const Color & color ) const;
    const Color operator-( const Color & rhs ) {
      return Color( red() - rhs.red(), green()- rhs.green(), blue() - rhs.blue() );
    }
    bool operator<( const Color & rhs ) const;

  private:

    unsigned int r,g,b;
    unsigned long value;

  };

}

#endif
