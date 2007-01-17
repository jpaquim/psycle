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
#include "font.h"
#include "app.h"

using namespace std;

namespace ngrs {

  Font::Font() : 
#ifdef __unix__
  name_("9x15"),
#else
  name_("Arial"),               
#endif
  size_(10),style_(nMedium | nStraight)
  {
    systemFnt = App::system().getFontValues(*this);
  }

  Font::Font( const std::string& name, int size, int style ) 
    : name_(name), size_(size), style_(style)
  {
    systemFnt = App::system().getFontValues( *this );
  }

  Font::~Font()
  {

  }

  // setter

  void Font::setName( const string& name )
  {
    name_ = name;
    systemFnt = App::system().getFontValues( *this );
  }

  void Font::setSize( int size )
  {
    size_ = size;
    systemFnt = App::system().getFontValues( *this );
  }

  void Font::setStyle( int style )
  {
    style_ = style;
    systemFnt = App::system().getFontValues( *this );
  }

  // getter

  const std::string & Font::name() const
  {
    return name_;
  }

  int Font::size() const
  {
    return size_;
  }

  int Font::style() const
  {
    return style_;
  }

  bool Font::antialias() const
  {
    return ( style_ & nAntiAlias ) != 0;
  }

  std::string Font::fontString() const
  {
    string styleString  = "*";
    string italicString = "i";
    if (style_ &  nBold)   styleString  = "bold"; else
      if (style_ & nMedium)  styleString  = "medium";
    if (style_ & nItalic)  italicString = "r";
    string alias = "";
    if (antialias()) alias = ":antialias"; else alias=":nonantialias";

    std::ostringstream o;
    o << name_ << ":" << size_ << ":" << styleString << ":" << italicString << ":" << alias;
    return o.str();
  }

  bool Font::operator==( const Font& fnt ) const
  {
    return (fnt.name()==name() && fnt.size() == size() && fnt.style()==style());
  }

  bool Font::operator <( const Font& fnt ) const
  {
    if (name_ < fnt.name()) return true;
    if (name_ > fnt.name()) return false;
    if (size_< fnt.size())  return true;
    if (size_ > fnt.size()) return false;
    if (style_ < fnt.style()) return true;
    if (textColor_ < fnt.textColor()) return true;

    return false;
  }

  const FontStructure& Font::platformFontStructure() const
  {
    return systemFnt;
  }

  const Color& Font::textColor() const
  {
    return textColor_;
  }

  void Font::setTextColor( const Color& color )
  {
    textColor_ = color;
    systemFnt.textColor = color;
  }

}
