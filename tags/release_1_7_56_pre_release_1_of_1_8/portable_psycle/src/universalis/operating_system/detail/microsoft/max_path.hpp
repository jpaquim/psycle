// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// Copyright (C) 1999-2005 Psycledelics http://psycle.pastnotecut.org : Johan Boule

///\file
///\brief microsoft's #define MAX_PATH has too low value for ntfs
#pragma once
#include <universalis/detail/project.hpp>
//#namespace UNIVERSALIS
	//#namespace OPERATING_SYSTEM
		#if defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT
			/// microsoft's #define MAX_PATH has too low value for ntfs
			#define UNIVERSALIS__OPERATING_SYSTEM__MICROSOFT__MAX_PATH (MAX_PATH < (1 << 12) ? (1 << 12) : MAX_PATH)
		#endif
	//#endnamespace
//#endnamespace
