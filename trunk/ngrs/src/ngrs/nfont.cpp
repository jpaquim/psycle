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
#include "nfont.h"
#include "napp.h"

using namespace std;


NFont::NFont() : name_("Suse sans"),size_(10),style_(nMedium | nStraight | nAntiAlias)
{
  systemFnt = NApp::system().getXFontValues(*this);
}

NFont::NFont( std::string name, int size,int style ) : name_(name),size_(size),style_(style)
{
  systemFnt = NApp::system().getXFontValues(*this);
}

NFont::~NFont()
{

}

// setter

void NFont::setName( string name )
{
  name_ = name;
  systemFnt = NApp::system().getXFontValues(*this);
}

void NFont::setSize( int size )
{
  size_ = size;
  systemFnt = NApp::system().getXFontValues(*this);
}

void NFont::setStyle( int style )
{
  style_ = style;
  systemFnt = NApp::system().getXFontValues(*this);
}

// getter

std::string NFont::name( ) const
{
  return name_;
}

int NFont::size( ) const
{
  return size_;
}

int NFont::style( ) const
{
  return style_;
}

bool NFont::antialias( ) const
{
  return style_ & nAntiAlias;
}

std::string NFont::fontString( ) const
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


bool NFont::operator ==( const NFont & fnt ) const
{
  return (fnt.name()==name() && fnt.size() == size() && fnt.style()==style());
}


bool NFont::operator <( const NFont & fnt ) const
{
    if (name_ < fnt.name()) return true;
    if (name_ > fnt.name()) return false;
    if (size_< fnt.size())  return true;
    if (size_ > fnt.size()) return false;
    if (style_ < fnt.style()) return true;
    if (textColor_ < fnt.textColor()) return true;

    return false;
}

const NFontStructure & NFont::systemFont( ) const
{
  return systemFnt;
}

const NColor & NFont::textColor( ) const
{
  return textColor_;
}

void NFont::setTextColor( const NColor & color )
{
  textColor_ = color;
  systemFnt.textColor = color;
}




