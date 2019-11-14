// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "portable.h"
#include <stdio.h>
#include <stdarg.h>

void psy_snprintf(char* buf, size_t buflen, const char* fmt, ...)
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