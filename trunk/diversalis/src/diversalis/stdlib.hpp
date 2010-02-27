// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2010 members of the psycle project http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>

///\file \brief compiler-independant meta-information about the standard/runtime library.

#ifndef DIVERSALIS__STDLIB__INCLUDED
#define DIVERSALIS__STDLIB__INCLUDED
#pragma once

#include "compiler.hpp"
#include "os.hpp"

#if __STDC__VERSION__ >= 199901 || ( \
	defined DIVERSALIS__COMPILER__GNU && \
	DIVERSALIS__COMPILER__VERSION__MAJOR >= 4 && \
	!defined DIVERSALIS__OS__CYGWIN \
)
	#define DIVERSALIS__STDLIB__MATH 199901
#endif

#if defined DIVERSALIS__COMPILER__MICROSOFT

	#if defined _MT // defined when -MD or -MDd (multithreaded dll) or -MT or -MTd (multithreaded static) is specified.
		#define DIVERSALIS__STDLIB__RUNTIME__MULTI_THREADED
	#endif

	#if defined _DLL // defined when -MD or -MDd (multithread dll) is specified.
		#define DIVERSALIS__STDLIB__RUNTIME__DYN_LINK
	#endif

	#if defined _DEBUG // defined when compiling with -LDd, -MDd, or -MTd.
		#define DIVERSALIS__STDLIB__RUNTIME__DEBUG
	#endif
#endif

#endif // !defined DIVERSALIS__STDLIB__INCLUDED
