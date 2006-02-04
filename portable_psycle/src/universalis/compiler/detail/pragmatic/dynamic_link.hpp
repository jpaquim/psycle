// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// Copyright (C) 1999-2005 Psycledelics http://psycle.pastnotecut.org : Johan Boule

///\file
///\brief compiler attributes for dynamic linking
#pragma once
#include "attribute.hpp"
#if defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT
	#define UNIVERSALIS__COMPILER__DYNAMIC_LINK__EXPORT UNIVERSALIS__COMPILER__ATTRIBUTE(dllexport)
	#define UNIVERSALIS__COMPILER__DYNAMIC_LINK__IMPORT UNIVERSALIS__COMPILER__ATTRIBUTE(dllimport)
	#define UNIVERSALIS__COMPILER__DYNAMIC_LINK__HIDDEN
#else
	#define UNIVERSALIS__COMPILER__DYNAMIC_LINK__EXPORT
	#define UNIVERSALIS__COMPILER__DYNAMIC_LINK__IMPORT
	#if defined DIVERSALIS__COMPILER__GNU
		#define UNIVERSALIS__COMPILER__DYNAMIC_LINK__HIDDEN UNIVERSALIS__COMPILER__ATTRIBUTE(visibility("hidden"))
	#else
		#define UNIVERSALIS__COMPILER__DYNAMIC_LINK__HIDDEN
	#endif
#endif

// arch-tag: 4ed20264-7732-4ff4-a032-88369d7eff59
