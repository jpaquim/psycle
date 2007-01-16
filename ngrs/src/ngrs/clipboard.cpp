/***************************************************************************
 *   Copyright (C) 2006, 2007 by Stefan Nattkemper   *
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
#include "clipboard.h"

namespace ngrs {

  // clipboard data class

  ClipBoard::ClipBoardData::ClipBoardData( )
  {
  }

  ClipBoard::ClipBoardData::~ ClipBoardData( )
  {
  }

  void ClipBoard::ClipBoardData::setFormat( unsigned char format )
  {
    if ( size() == 0 ) {
      push_back(format);
    } else {
      (*this)[0] = format;
    }
  }

  unsigned char ClipBoard::ClipBoardData::format( ) const
  {
    if ( size() == 0 )
      return 0;
    else
      return *begin();
  }

  // start clipboard

  ClipBoard::ClipBoard()
  {
  }


  ClipBoard::~ClipBoard()
  {
  }

  void ClipBoard::setAsText( const std::string & text )
  {
    data_.clear();
    data_.setFormat( 1 );

    std::string::const_iterator it = text.begin();
    for ( ; it != text.end(); it++) {
      data_.push_back(*it);
    }

    data_.push_back(0);
  }

  std::string ClipBoard::asText( ) const
  {
    std::string text = "";
    if (data_.format() == 1) {
      text = std::string( (const char*) &data_[1] );
    }

    return text;
  }

}
