// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(PREFIX_H)
#define PREFIX_H

// psycle status

#define PSY_OK 0
#define PSY_ERRFILE 1

#if defined _M_X64
#define SSE
#else
#if defined _M_IX86 // according to the only doc, it doesn't seem to go above 600 (i686)
#if _M_IX86_FP // expands to a value indicating which -arch compiler option was used:
	// 0 if -arch was not used.
	// 1 if -arch:SSE was used.
	// 2 if -arch:SSE2 was used.
	///\todo detect SSE3 (prescott, nocona)
#define SSE _M_IX86_FP
#endif
#endif
#endif

/*
** Windows stuff
*/
#if defined(_WIN32) 	/* { */

// #define _CRTDBG_MAP_ALLOC
// #include <crtdbg.h>

#if !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS  /* avoid warnings about ISO C functions */
#define _CRT_NONSTDC_NO_DEPRECATE
#endif

#endif			/* } */

#endif
