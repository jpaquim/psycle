// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// Copyright (C) 1999-2005 Psycledelics http://psycle.pastnotecut.org : Johan Boule

///\file
///\brief shared dynamic-link library loader
#pragma once
#include "attribute.hpp"
#if defined DIVERSALIS__COMPILER__GNU
	#define UNIVERSALIS__COMPILER__HIDDEN UNIVERSALIS__COMPILER__ATTRIBUTE(visibility("hidden"))
#endif
#if !defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT
	#define UNIVERSALIS__COMPILER__EXPORT
	#define UNIVERSALIS__COMPILER__IMPORT
#else
	#define UNIVERSALIS__COMPILER__EXPORT UNIVERSALIS__COMPILER__ATTRIBUTE(dllexport)
	#define UNIVERSALIS__COMPILER__IMPORT UNIVERSALIS__COMPILER__ATTRIBUTE(dllimport)
#endif
#if !defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT || defined DIVERSALIS__COMPILER__GNU // gcc handles dll transparently :-)
	/// everything is nice and simple
	/// [bohan] but, see http://people.redhat.com/drepper/dsohowto.pdf (pdf sux ... is there a non pdf version?)
	///\todo /dev/null will fail on mingw outside of msys ... we need an empty file... or an already-included one... maybe #include __FILE__
	#define UNIVERSALIS__COMPILER__LIBRARY__BEGIN  "/dev/null"
	#define UNIVERSALIS__COMPILER__LIBRARY__EXPORT "/dev/null"
	#define UNIVERSALIS__COMPILER__LIBRARY__IMPORT "/dev/null"
	#define UNIVERSALIS__COMPILER__LIBRARY__END    "/dev/null"
#else
	#define UNIVERSALIS__COMPILER__LIBRARY__BEGIN  <universalis/operating_system/library/begin.hpp>
	#define UNIVERSALIS__COMPILER__LIBRARY__EXPORT <universalis/operating_system/library/export.hpp>
	#define UNIVERSALIS__COMPILER__LIBRARY__IMPORT <universalis/operating_system/library/import.hpp>
	#define UNIVERSALIS__COMPILER__LIBRARY__END    <universalis/operating_system/library/end.hpp>
	/**/#if 0
		// This horrible problem that bloats the headers hopefully
		// only occurs with microsoft's dll format with other compilers than GCC :-)
		// If you really insist on using another compiler on the microsoft operating system,
		// see:
			#include <universalis/operating_system/library/export.hpp>
			#include <universalis/operating_system/library/import.hpp>
		// then, starts bloating every header and source files with "appropriate" preprocessor directives.
		// Example of use:
			// in file foo/bar.cpp:
				#define FOO__BAR
				#include "bar.hpp"
			// in file foo/bar.hpp:
				#pragma once
				#if defined FOO__BAR
					#include UNIVERSALIS__COMPILER__LIBRARY__EXPORT
				#else
					#include UNIVERSALIS__COMPILER__LIBRARY__IMPORT
				#endif
				namespace foo
				{
					class bar
					{
					};
				}
		// Alternative way is to use:
			#include <universalis/operating_system/library/begin.hpp>
			#include <universalis/operating_system/library/end.hpp>
		// which attempt to be smarter by doing,
				#define class class UNIVERSALIS__COMPILER__ATTRIBUTE(dllexport)
		// or,
				#define class class UNIVERSALIS__COMPILER__ATTRIBUTE(dllimport)
		// so that everything is handled transparently.
		// Example of use:
			// in file foo/bar.cpp:
				#define FOO__BAR 1
				#include "bar.hpp"
			// in file foo/bar.hpp:
				#pragma once
				#define UNIVERSALIS__COMPILER__LIBRARY FOO__BAR
				#include UNIVERSALIS__COMPILER__LIBRARY__BEGIN
				namespace foo
				{
					class bar
					{
					};
				}
				#include UNIVERSALIS__COMPILER__LIBRARY__END
	/**/#endif
#endif

// arch-tag: 4ed20264-7732-4ff4-a032-88369d7eff59
