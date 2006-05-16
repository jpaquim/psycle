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
#include "ngrs/nkeyaccelerator.h"

NKeyAccelerator::NKeyAccelerator( int keyMask, char key)
{
  keyMask_ = keyMask;
  key_ = key;
}

NKeyAccelerator::~NKeyAccelerator()
{
}

NKeyAccelerator::NKeyAccelerator( )
{
  keyMask_ = key_ = 0;
}

bool NKeyAccelerator::operator <( const NKeyAccelerator & a ) const
{
  int comp1 = key_    | (keyMask_    << 8);
  int comp2 = a.key() | (a.keyMask() << 8);
  return comp1 < comp2;
}

char NKeyAccelerator::key( ) const
{
  return key_;
}

char NKeyAccelerator::keyMask( ) const
{
  return keyMask_;
}


