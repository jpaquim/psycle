// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// Copyright (C) 1999-2006 Johan Boule <bohan@jabber.org>
// Copyright (C) 2004-2006 Psycledelics http://psycle.pastnotecut.org

///\file
#pragma once
#include "pragmas.hpp"
//#region UNIVERSALIS
	//#region COMPILER
		#else
		#if defined DIVERSALIS__COMPILER__MICROSOFT
			#if !defined DIVERSALIS__COMPILER__FEATURE__AUTO_LINK
				#error "boggus preprocessor definitions: DIVERSALIS__COMPILER__FEATURE__AUTO_LINK should be defined. please fix the cause not this symptom."
			#endif
			#define UNIVERSALIS__COMPILER__AUTO_LINK(library) UNIVERSALIS__COMPILER__PRAGMA("comment(lib, \"" #library "\"")
		#else
			#define UNIVERSALIS__COMPILER__AUTO_LINK(library)
		#endif
	//#endregion
//#endregion
