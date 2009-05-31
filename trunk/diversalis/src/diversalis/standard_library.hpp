// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2008 members of the psycle project http://psycle.pastnotecut.org ; johan boule <bohan@jabber.org>

///\file \brief project-wide standard library tweaks.

#ifndef DIVERSALIS__STANDARD_LIBRARY__INCLUDED
#define DIVERSALIS__STANDARD_LIBRARY__INCLUDED
#pragma once

#include "compiler.hpp"
#include "operating_system.hpp"

#if __STDC__VERSION__ >= 199901 || ( \
	defined DIVERSALIS__COMPILER__GNU && \
	DIVERSALIS__COMPILER__VERSION__MAJOR >= 4 && \
	!defined DIVERSALIS__OPERATING_SYSTEM__CYGWIN \
)
	#define DIVERSALIS__STANDARD_LIBRARY__MATH 199901
#endif

#if defined DIVERSALIS__COMPILER__MICROSOFT

	#if defined _MT // defined when -MD or -MDd (multithreaded dll) or -MT or -MTd (multithreaded static) is specified.
		#define DIVERSALIS__STANDARD_LIBRARY__RUNTIME__MULTITHREADED
	#endif

	#if defined _DLL // defined when -MD or -MDd (multithread dll) is specified.
		#define DIVERSALIS__STANDARD_LIBRARY__RUNTIME__DYNAMIC_LINK
	#endif

	#if defined _DEBUG // defined when compiling with -LDd, -MDd, or -MTd.
		#define DIVERSALIS__STANDARD_LIBRARY__RUNTIME__DEBUG
	#endif
#endif

#endif // !defined DIVERSALIS__STANDARD_LIBRARY__INCLUDED
