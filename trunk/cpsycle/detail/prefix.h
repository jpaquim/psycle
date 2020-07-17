// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(PREFIX_H)
#define PREFIX_H

/*
** Windows stuff
*/
#if defined(_WIN32) 	/* { */

// For leak detection in VC decomment the next two lines, rebuild the solution
// and start with F5. Now study after programm termination the output window.
// Each leak has a memory allocation number in curly braces e.g {14672}.
// Copy this line to the clipboard.
// Set a breakpoint in psycle.c at the first possible line in main and start
// again with F5. At break open a Watch Window (Debugger, Window, Watch) and
// insert _crtBreakAlloc and press Return. Replace -1 with the leak id from
// the clipboard and continue the execution with F5. At break study the
// callstack to determine the allocation of the leak
// https://docs.microsoft.com/en-us/visualstudio/debugger/finding-memory-leaks-using-the-crt-library?view=vs-2019
// (Set breakpoints on a memory allocation number)

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>

#if !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS  /* avoid warnings about ISO C functions */
#define _CRT_NONSTDC_NO_DEPRECATE
#endif

#endif			/* } */

#endif
