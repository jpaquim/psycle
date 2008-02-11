/* -*- mode:c++, indent-tabs-mode:t -*- */
// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2007 johan boule <bohan@jabber.org>
// copyright 2004-2007 psycledelics http://psycle.pastnotecut.org

///\file
/// the restrict keyword has been introduced in the iso 1999 standard.
/// it is a hint for the compiler telling it several references or pointers will *always* holds different memory addresses.
/// hence it can optimize more, knowing that writting to memory via one reference cannot alias another.
/// example:
	/// void f(int & restrict r1, int & restrict r2, int * restrict p1, int restrict p2[]);
	/// here the compiler is told that &r1 != &r2 != p1 != p2
#pragma once
#include <universalis/detail/project.hpp>
///\todo test __STDC_VERSION
#if defined DIVERSALIS__COMPILER__GNU
	// restrict is a keyword
	#define restrict __restrict__
#elif defined DIVERSALIS__COMPILER__MICROSOFT
	#if DIVERSALIS__COMPILER__VERSION__MAJOR >= 8
		// restrict is a keyword
	#else
		// unsupported?
		// see also: #pragma optimize("a", on) // assumes no aliasing
		#define restrict
	#endif
#else
	#define restrict
#endif
