// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(psy_TRACE_H)
#define psy_TRACE_H

#include "psydef.h"
#include "os.h"

#ifdef DIVERSALIS__OS__MICROSOFT
#include <windows.h>
#include "portable.h"
#define TRACE(str) OutputDebugString(str)
#define TRACE_FLOAT(val) do { char str[128]; psy_snprintf(str, 128, "%f ", (float)val); OutputDebugString(str); } while(0)
#define TRACE_INT(val) do { char str[128]; psy_snprintf(str, 128, "%d ", (int)val); OutputDebugString(str); } while(0)
#else
#include <stdio.h>
#define TRACE(str) do { printf("%s", str); fflush(stdout); } while (0);
#define TRACE_FLOAT(val) do { printf("%f", (float)val); fflush(stdout); } while (0);
#define TRACE_INT(val) do { printf("%d ", (int)val); fflush(stdout);  } while (0);
#endif

#endif
