// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// Copyright (C) 1999-2006 Johan Boule <bohan@jabber.org>
// Copyright (C) 2004-2006 Psycledelics http://psycle.pastnotecut.org

///\file
#pragma once
#include <universalis/detail/project.hpp>
//#region UNIVERSALIS
	//#region COMPILER
		#define UNIVERSALIS__COMPILER__PRAGMA(x) _Pragma(#x)

		#if defined DIVERSALIS__COMPILER__GNU
			#define UNIVERSALIS__COMPILER__POISON(x) UNIVERSALIS__COMPILER__PRAGMA("GCC poison " #x)
		#else
			#define UNIVERSALIS__COMPILER__POISON(x)
		#endif
	//#endregion
//#endregion
