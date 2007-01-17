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
#include "keyaccelerator.h"

namespace ngrs {

  KeyAccelerator::KeyAccelerator( int keyMask, char key)
  {
    keyMask_ = keyMask;
    key_ = key;
  }

  KeyAccelerator::~KeyAccelerator()
  {
  }

  KeyAccelerator::KeyAccelerator( )
  {
    keyMask_ = key_ = 0;
  }

  bool KeyAccelerator::operator <( const KeyAccelerator & a ) const
  {
    int comp1 = key_    | (keyMask_    << 8);
    int comp2 = a.key() | (a.keyMask() << 8);
    return comp1 < comp2;
  }

  char KeyAccelerator::key( ) const
  {
    return key_;
  }

  char KeyAccelerator::keyMask( ) const
  {
    return keyMask_;
  }

}
