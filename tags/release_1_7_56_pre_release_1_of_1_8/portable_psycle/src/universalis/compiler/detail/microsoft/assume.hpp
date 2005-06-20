// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// Copyright (C) 1999-2005 Psycledelics http://psycle.pastnotecut.org : Johan Boule

///\file
#pragma once
#include <universalis/detail/project.hpp>
/// [bohan] i don't quite understand what microsoft says about this keyword...
#if defined DIVERSALIS__COMPILER__GNU
	// unsupported?
	#define assume(x)
#elif defined DIVERSALIS__COMPILER__MICROSOFT
	#define assume(x) __assume(x)
#else
	#define assume(x)
#endif

// arch-tag: cdf6c3a9-9db3-4680-90ed-2d0c560a4307
