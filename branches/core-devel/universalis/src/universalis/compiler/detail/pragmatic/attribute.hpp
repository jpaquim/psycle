// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2007 johan boule <bohan@jabber.org>
// copyright 2004-2007 psycledelics http://psycle.pastnotecut.org

///\file
/// pragmatic attribute
#pragma once
#include <universalis/detail/project.hpp>
/// pragmatic attribute
#if defined DIVERSALIS__COMPILER__GNU
	#define UNIVERSALIS__COMPILER__ATTRIBUTE(x) __attribute__ ((x))
#elif defined DIVERSALIS__COMPILER__MICROSOFT
	#define UNIVERSALIS__COMPILER__ATTRIBUTE(x) __declspec(x)
#else
	#define UNIVERSALIS__COMPILER__ATTRIBUTE(x)
#endif
