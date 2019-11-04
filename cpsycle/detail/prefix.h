// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(PREFIX_H)
#define PREFIX_H

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
