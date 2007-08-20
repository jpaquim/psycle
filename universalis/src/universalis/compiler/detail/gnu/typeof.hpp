// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2007 johan boule <bohan@jabber.org>
// copyright 2004-2007 psycledelics http://psycle.pastnotecut.org

///\file
#pragma once
#include <universalis/detail/project.hpp>
#if defined DIVERSALIS__COMPILER__GNU
	// typeof is a keyword unless strict iso conformance is enabled, in which case, it is made available with the __typeof__ keyword
	#define typeof __typeof__
	#define DIVERSALIS__COMPILER__FEATURE__TYPEOF
#elif defined DIVERSALIS__COMPILER__MICROSOFT
	/// it looks like a reserved keyword, but it's not implemented yet as of version 7.1 (only in c#).
	#define typeof __typeof
#endif
