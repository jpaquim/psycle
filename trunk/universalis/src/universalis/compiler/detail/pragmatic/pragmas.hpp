/* -*- mode:c++, indent-tabs-mode:t -*- */
// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2007 johan boule <bohan@jabber.org>
// copyright 2004-2007 psycledelics http://psycle.pastnotecut.org

///\file
#pragma once
#include <universalis/detail/project.hpp>
//#region UNIVERSALIS
	//#region COMPILER
		#define UNIVERSALIS__COMPILER__PRAGMA(x) _Pragma(#x)

		#if defined DIVERSALIS__COMPILER__GNU
			#define UNIVERSALIS__COMPILER__POISON(x) UNIVERSALIS__COMPILER__PRAGMA("GCC poison " #x)
		#elif defined DIVERSALIS__COMPILER__MICROSOFT
			#define UNIVERSALIS__COMPILER__POISON(x) UNIVERSALIS__COMPILER__PRAGMA("deprecated(\"#x\")")
		#else
			#define UNIVERSALIS__COMPILER__POISON(x)
		#endif
	//#endregion
//#endregion
