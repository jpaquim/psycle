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
#ifndef FONT_H
#define FONT_H

#include <string>
#include "color.h"
#include "runtime.h"
#include "fontstructure.h"


/**
@author  Stefan
*/

namespace ngrs {

  enum FontStyleEnum { nBold=1,nMedium=2, nItalic=4, nStraight=8, nAntiAlias=16};

  class Font{
  public:
    Font();
    Font( const std::string& name, int size=10, int style=nMedium | nStraight | nAntiAlias);

    ~Font();

    void setName( const std::string& name);
    void setSize( int size );
    void setStyle( int style );

    const std::string & name() const;
    int size() const;
    int style() const;
    bool antialias() const;
    std::string fontString( ) const;
    bool operator==(const Font& fnt) const;
    bool operator<(const Font& fnt) const;

    void setTextColor(const Color& color);
    const Color& textColor() const;

    const FontStructure& platformFontStructure() const;

  private:

    std::string name_;
    int size_, style_;
    Color textColor_;

    FontStructure systemFnt;

  };

}

#endif
