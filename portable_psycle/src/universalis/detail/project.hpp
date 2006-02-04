// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// Copyright (C) 1999-2005 Psycledelics http://psycle.pastnotecut.org : Johan Boule

///\file
#pragma once

#include <universalis/operating_system/detail/microsoft/packageneric.microsoft.never-hardcoded.hpp>

//#define DIVERSALIS__PACKAGENERIC__CONFIGURATION__OPERATING_SYSTEM__MICROSOFT__REQUIRE  UNIVERSALIS__OPERATING_SYSTEM__MICROSOFT
//#include <diversalis/diversalis.hpp>

/*
#define c(a,b) c_(a,b)
#define c_(a,b) a##b
#if c(x,M)
	#if !defined _M
		#define _M c(x,M)
	#elif _M < c(x,M)
		#error
	#elif _M == c(x,M) && c(x,m)
		...
*/

#if defined UNIVERSALIS__OPERATING_SYSTEM__MICROSOFT__PACKAGENERIC__CONFIGURATION__OPERATING_SYSTEM__MICROSOFT__BRANCH__NT
	#if !defined DIVERSALIS__PACKAGENERIC__CONFIGURATION__OPERATING_SYSTEM__MICROSOFT__BRANCH
		#define DIVERSALIS__PACKAGENERIC__CONFIGURATION__OPERATING_SYSTEM__MICROSOFT__BRANCH__NT
	#elif !defined DIVERSALIS__PACKAGENERIC__CONFIGURATION__OPERATING_SYSTEM__MICROSOFT__BRANCH__NT
		#error universalis requires the nt branch
	#endif
#endif

#if defined UNIVERSALIS__OPERATING_SYSTEM__MICROSOFT__PACKAGENERIC__CONFIGURATION__OPERATING_SYSTEM__MICROSOFT__VERSION__MAJOR
	#if !defined DIVERSALIS__PACKAGENERIC__CONFIGURATION__OPERATING_SYSTEM__MICROSOFT__VERSION__MAJOR
		#define DIVERSALIS__PACKAGENERIC__CONFIGURATION__OPERATING_SYSTEM__MICROSOFT__VERSION__MAJOR UNIVERSALIS__OPERATING_SYSTEM__MICROSOFT__PACKAGENERIC__CONFIGURATION__OPERATING_SYSTEM__MICROSOFT__VERSION__MAJOR
		///\todo minor and patch too
	#elif DIVERSALIS__PACKAGENERIC__CONFIGURATION__OPERATING_SYSTEM__MICROSOFT__VERSION__MAJOR < UNIVERSALIS__OPERATING_SYSTEM__MICROSOFT__PACKAGENERIC__CONFIGURATION__OPERATING_SYSTEM__MICROSOFT__VERSION__MAJOR
			#error universalis requires a higher version
	#elif \
	( \
		DIVERSALIS__PACKAGENERIC__CONFIGURATION__OPERATING_SYSTEM__MICROSOFT__VERSION__MAJOR == UNIVERSALIS__OPERATING_SYSTEM__MICROSOFT__PACKAGENERIC__CONFIGURATION__OPERATING_SYSTEM__MICROSOFT__VERSION__MAJOR && \
		defined UNIVERSALIS__OPERATING_SYSTEM__MICROSOFT__PACKAGENERIC__CONFIGURATION__OPERATING_SYSTEM__MICROSOFT__VERSION__MINOR \
	)
		#if !defined DIVERSALIS__PACKAGENERIC__CONFIGURATION__OPERATING_SYSTEM__MICROSOFT__VERSION__MINOR
			#define DIVERSALIS__PACKAGENERIC__CONFIGURATION__OPERATING_SYSTEM__MICROSOFT__VERSION__MINOR UNIVERSALIS__OPERATING_SYSTEM__MICROSOFT__PACKAGENERIC__CONFIGURATION__OPERATING_SYSTEM__MICROSOFT__VERSION__MINOR
		#elif DIVERSALIS__PACKAGENERIC__CONFIGURATION__OPERATING_SYSTEM__MICROSOFT__VERSION__MINOR < UNIVERSALIS__OPERATING_SYSTEM__MICROSOFT__PACKAGENERIC__CONFIGURATION__OPERATING_SYSTEM__MICROSOFT__VERSION__MINOR
			#error universalis requires a higher version
		#elif \
		( \
			DIVERSALIS__PACKAGENERIC__CONFIGURATION__OPERATING_SYSTEM__MICROSOFT__VERSION__MINOR == UNIVERSALIS__OPERATING_SYSTEM__MICROSOFT__PACKAGENERIC__CONFIGURATION__OPERATING_SYSTEM__MICROSOFT__VERSION__MINOR && \
                  defined UNIVERSALIS__OPERATING_SYSTEM__MICROSOFT__PACKAGENERIC__CONFIGURATION__OPERATING_SYSTEM__MICROSOFT__VERSION__PATCH \
		)
			#if !defined DIVERSALIS__PACKAGENERIC__CONFIGURATION__OPERATING_SYSTEM__MICROSOFT__VERSION__PATCH
				#define DIVERSALIS__PACKAGENERIC__CONFIGURATION__OPERATING_SYSTEM__MICROSOFT__VERSION__PATCH UNIVERSALIS__OPERATING_SYSTEM__MICROSOFT__PACKAGENERIC__CONFIGURATION__OPERATING_SYSTEM__MICROSOFT__VERSION__PATCH
			#elif DIVERSALIS__PACKAGENERIC__CONFIGURATION__OPERATING_SYSTEM__MICROSOFT__VERSION__PATCH < UNIVERSALIS__OPERATING_SYSTEM__MICROSOFT__PACKAGENERIC__CONFIGURATION__OPERATING_SYSTEM__MICROSOFT__VERSION__PATCH
				#error universalis requires a higher version
			#endif
		#endif
	#endif
#endif

#if defined UNIVERSALIS__OPERATING_SYSTEM__MICROSOFT__PACKAGENERIC__CONFIGURATION__OPERATING_SYSTEM__MICROSOFT__VERSION__EXTRA_LAYER__INTERNET_EXPLORER__MAJOR
	#if !defined DIVERSALIS__PACKAGENERIC__CONFIGURATION__OPERATING_SYSTEM__MICROSOFT__VERSION__EXTRA_LAYER__INTERNET_EXPLORER__MAJOR
		#define DIVERSALIS__PACKAGENERIC__CONFIGURATION__OPERATING_SYSTEM__MICROSOFT__VERSION__EXTRA_LAYER__INTERNET_EXPLORER__MAJOR UNIVERSALIS__OPERATING_SYSTEM__MICROSOFT__PACKAGENERIC__CONFIGURATION__OPERATING_SYSTEM__MICROSOFT__VERSION__EXTRA_LAYER__INTERNET_EXPLORER__MAJOR
	#elif DIVERSALIS__PACKAGENERIC__CONFIGURATION__OPERATING_SYSTEM__MICROSOFT__VERSION__EXTRA_LAYER__INTERNET_EXPLORER__MAJOR < UNIVERSALIS__OPERATING_SYSTEM__MICROSOFT__PACKAGENERIC__CONFIGURATION__OPERATING_SYSTEM__MICROSOFT__VERSION__EXTRA_LAYER__INTERNET_EXPLORER__MAJOR
			#error universalis requires a higher version
	#elif \
	( \
		DIVERSALIS__PACKAGENERIC__CONFIGURATION__OPERATING_SYSTEM__MICROSOFT__VERSION__EXTRA_LAYER__INTERNET_EXPLORER__MAJOR == UNIVERSALIS__OPERATING_SYSTEM__MICROSOFT__PACKAGENERIC__CONFIGURATION__OPERATING_SYSTEM__MICROSOFT__VERSION__EXTRA_LAYER__INTERNET_EXPLORER__MAJOR && \
		defined UNIVERSALIS__OPERATING_SYSTEM__MICROSOFT__PACKAGENERIC__CONFIGURATION__OPERATING_SYSTEM__MICROSOFT__VERSION__EXTRA_LAYER__INTERNET_EXPLORER__MINOR \
	)
		#if !defined DIVERSALIS__PACKAGENERIC__CONFIGURATION__OPERATING_SYSTEM__MICROSOFT__VERSION__EXTRA_LAYER__INTERNET_EXPLORER__MINOR
			#define DIVERSALIS__PACKAGENERIC__CONFIGURATION__OPERATING_SYSTEM__MICROSOFT__VERSION__EXTRA_LAYER__INTERNET_EXPLORER__MINOR UNIVERSALIS__OPERATING_SYSTEM__MICROSOFT__PACKAGENERIC__CONFIGURATION__OPERATING_SYSTEM__MICROSOFT__VERSION__EXTRA_LAYER__INTERNET_EXPLORER__MINOR
		#elif DIVERSALIS__PACKAGENERIC__CONFIGURATION__OPERATING_SYSTEM__MICROSOFT__VERSION__EXTRA_LAYER__INTERNET_EXPLORER__MINOR < UNIVERSALIS__OPERATING_SYSTEM__MICROSOFT__PACKAGENERIC__CONFIGURATION__OPERATING_SYSTEM__MICROSOFT__VERSION__EXTRA_LAYER__INTERNET_EXPLORER__MINOR
			#error universalis requires a higher version
		#elif \
		( \
			DIVERSALIS__PACKAGENERIC__CONFIGURATION__OPERATING_SYSTEM__MICROSOFT__VERSION__EXTRA_LAYER__INTERNET_EXPLORER__MINOR == UNIVERSALIS__OPERATING_SYSTEM__MICROSOFT__PACKAGENERIC__CONFIGURATION__OPERATING_SYSTEM__MICROSOFT__VERSION__EXTRA_LAYER__INTERNET_EXPLORER__MINOR && \
                  defined UNIVERSALIS__OPERATING_SYSTEM__MICROSOFT__PACKAGENERIC__CONFIGURATION__OPERATING_SYSTEM__MICROSOFT__VERSION__EXTRA_LAYER__INTERNET_EXPLORER__PATCH \
		)
			#if !defined DIVERSALIS__PACKAGENERIC__CONFIGURATION__OPERATING_SYSTEM__MICROSOFT__VERSION__EXTRA_LAYER__INTERNET_EXPLORER__PATCH
				#define DIVERSALIS__PACKAGENERIC__CONFIGURATION__OPERATING_SYSTEM__MICROSOFT__VERSION__EXTRA_LAYER__INTERNET_EXPLORER__PATCH UNIVERSALIS__OPERATING_SYSTEM__MICROSOFT__PACKAGENERIC__CONFIGURATION__OPERATING_SYSTEM__MICROSOFT__VERSION__EXTRA_LAYER__INTERNET_EXPLORER__PATCH
			#elif DIVERSALIS__PACKAGENERIC__CONFIGURATION__OPERATING_SYSTEM__MICROSOFT__VERSION__EXTRA_LAYER__INTERNET_EXPLORER__PATCH < UNIVERSALIS__OPERATING_SYSTEM__MICROSOFT__PACKAGENERIC__CONFIGURATION__OPERATING_SYSTEM__MICROSOFT__VERSION__EXTRA_LAYER__INTERNET_EXPLORER__PATCH
				#error universalis requires a higher version
			#endif
		#endif
	#endif
#endif

#include <diversalis/diversalis.hpp>
