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
#include "fntstring.h"

using namespace std;

namespace ngrs {

  FntString::FntString()
  {
  }


  FntString::~FntString()
  {
  }

  void FntString::append( const std::string & text )
  {
    std::string::append( text );
  }

  void FntString::setFont( const Font & font )
  {
    fonts_.push_back( font );
    positions_.push_back( length() );
  }

  FntString FntString::substr( std::string::size_type pos, std::string::size_type size ) const
  {
    FntString sub;
    sub.setText( std::string::substr( pos, size ) );
    bool insertOldFont = false;

    vector<Font>::const_iterator fntIt = fonts_.begin();
    for (vector<std::string::size_type>::const_iterator it = positions_.begin(); it < positions_.end(); it++) {
      std::string::size_type p = *it;
      if ( p < pos ) insertOldFont = true; else
        if ( p >= pos && p < pos+size ) {
          if ( insertOldFont ) {
            sub.positions_.push_back( 0 );
            sub.fonts_.push_back( *fntIt );
            insertOldFont = false;
          }
          sub.positions_.push_back( p - pos );
          sub.fonts_.push_back( *fntIt );
        }
        fntIt++;
    }
    return sub;
  }

  FntString FntString::substr( std::string::size_type last ) const
  {
    return substr( last, length()-last );
  }

  void FntString::setText( const std::string & text )
  {
    append( text );
    positions_.clear( );
    fonts_.clear( );
  }

  const std::vector< std::string::size_type > & FntString::positions( ) const
  {
    return positions_;
  }

  const std::vector< Font > & FntString::fonts( ) const
  {
    return fonts_;
  }

  void FntString::append( const FntString & text )
  {
    std::string::size_type oldLen = length();
    std::string::append(text);
    vector<Font>::const_iterator fntIt = text.fonts_.begin();
    for (vector<std::string::size_type>::const_iterator it = text.positions_.begin(); it < text.positions_.end(); it++) {
      std::string::size_type p = *it; 
      Font fnt = *fntIt;
      positions_.push_back(p+oldLen);
      fonts_.push_back((*fntIt)); 
      fntIt++;
    }
  }

  std::string FntString::textsubstr( std::string::size_type pos, std::string::size_type size ) const
  {
    return substr( pos, size );
  }

  std::string FntString::textsubstr( std::string::size_type last ) const
  {
    return substr( last );
  }

}
