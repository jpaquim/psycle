// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(PORTABLE_H)
#define PORTABLE_H

#include "../../detail/psydef.h"

#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>

#ifndef min
#define min(a, b) (((a < b) ? a : b))
#endif

#ifndef max
#define max(a, b)  (((a) > (b)) ? (a) : (b)) 
#endif

INLINE void psy_snprintf(char* buf, size_t buflen, const char* fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
#ifdef _WIN32
	if (buflen > 0) {
		_vsnprintf(buf, buflen, fmt, ap);
		buf[buflen - 1] = '\0';
	}
#else
	vsnprintf(buf, buflen, fmt, ap);	
#endif	
	va_end(ap);
}

/*-------------------------------------------------------------------------
   psy_strlwr is included from https://sourceforge.net/projects/sdcc/
   strlwr.c - part of string library functions

   Written by Vangelis Rokas, 2004 <vrokas AT otenet.gr>

   This library is free software; you can redistribute it and/or modify it
   under the terms of the GNU Library General Public License as published by the
   Free Software Foundation; either version 2, or (at your option) any
   later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this program; if not, write to the Free Software
   Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

   In other words, you are welcome to use, share and improve this program.
   You are forbidden to forbid anyone else to use, share and improve
   what you give them.   Help stamp out software-hoarding!
-------------------------------------------------------------------------*/
INLINE char* psy_strlwr(char* str)
{
    char* ret = str;

    while (*str) {
        *str = tolower(*str);
        str++;
    }

    return (ret);
}


#endif
