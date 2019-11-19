// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(PORTABLE_H)
#define PORTABLE_H

#include <stddef.h>

#ifndef min
#define min(a, b) (((a < b) ? a : b))
#endif

void psy_snprintf(char* buf, size_t buflen, const char* fmt, ...);

#endif
