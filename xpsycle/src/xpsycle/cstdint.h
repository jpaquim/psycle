/***************************************************************************
*   Copyright (C) 2006 by Stefan   *
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
#pragma once
#include <inttypes.h> // that's for posix/unix.. C1999 has <stdint.h>

namespace std
{
  using ::int8_t;    //  8-bit signed integral type. 
  using ::int16_t;   //  16-bit signed integral type. 
  using ::int32_t;   //  32-bit signed integral type. 
  using ::int64_t;   //  64-bit signed integral type. 
  using ::uint8_t;   //  8-bit unsigned integral type. 
  using ::uint16_t;  //  16-bit unsigned integral type. 
  using ::uint32_t;  //  32-bit unsigned integral type. 
  using ::uint64_t;  //  64-bit unsigned integral type. 
  using ::intptr_t;  //  Signed integral type large enough to hold any pointer. 
  using ::uintptr_t; //  Unsigned integral type large enough to hold any pointer.
}

