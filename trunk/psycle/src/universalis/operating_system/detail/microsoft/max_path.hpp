// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// Copyright (C) 1999-2006 Johan Boule <bohan@jabber.org>
// Copyright (C) 2004-2006 Psycledelics http://psycle.pastnotecut.org

///\file
///\brief microsoft's #define MAX_PATH has too low value for ntfs
#pragma once
#include <universalis/detail/project.hpp>
//#region UNIVERSALIS
	//#region OPERATING_SYSTEM
		#if defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT
			#include <windows.h>
			/// microsoft's #define MAX_PATH has too low value for ntfs
			///\see <cstdio>'s FILENAME_MAX
			#define UNIVERSALIS__OPERATING_SYSTEM__MICROSOFT__MAX_PATH (MAX_PATH < (1 << 12) ? (1 << 12) : MAX_PATH)
		#endif
	//#endregion
//#endregion
