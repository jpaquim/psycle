/***************************************************************************
 *   Copyright (C) 2006 by Stefan Nattkemper   *
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
#include "ngrs/nfntstring.h"

using namespace std;

NFntString::NFntString()
{
}


NFntString::~NFntString()
{
}

void NFntString::append( const std::string & text )
{
  std::string::append(text);
}

void NFntString::setFont( const NFont & font )
{
  fonts_.push_back(font);
  positions_.push_back(length());
}

NFntString NFntString::substr( int pos, int size ) const
{
  NFntString sub;
  sub.setText(std::string::substr(pos,size));
  bool insertOldFont = false;

  vector<NFont>::const_iterator fntIt = fonts_.begin();
  for (vector<int>::const_iterator it = positions_.begin(); it < positions_.end(); it++) {
    int p = *it;
    if (p<pos) insertOldFont = true; else
    if (p>=pos && p<pos+size) {
       if (insertOldFont) {
         sub.positions_.push_back(0);
         sub.fonts_.push_back(*fntIt);
         insertOldFont = false;
       }
       sub.positions_.push_back(p-pos);
       sub.fonts_.push_back((*fntIt));
    }
    fntIt++;
  }
  return sub;
}

NFntString NFntString::substr( int last ) const
{
  return substr(last,length()-last);
}

void NFntString::setText( const std::string & text )
{
  append(text);
  positions_.clear();
  fonts_.clear();
}


const std::vector< int > & NFntString::positions( ) const
{
  return positions_;
}

const std::vector< NFont > & NFntString::fonts( ) const
{
  return fonts_;
}

void NFntString::append( const NFntString & text )
{
  int oldLen = length();
  std::string::append(text);
  vector<NFont>::const_iterator fntIt = text.fonts_.begin();
  for (vector<int>::const_iterator it = text.positions_.begin(); it < text.positions_.end(); it++) {
    int p = *it; 
    NFont fnt = *fntIt;
    positions_.push_back(p+oldLen);
    fonts_.push_back((*fntIt)); 
    fntIt++;
  }
}

std::string NFntString::textsubstr( int pos, int size ) const
{
  return substr(pos,size);
}

std::string NFntString::textsubstr( int last ) const
{
  return substr(last);
}

