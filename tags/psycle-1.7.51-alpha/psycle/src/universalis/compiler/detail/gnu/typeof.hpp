// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// Copyright (C) 1999-2005 Psycledelics http://psycle.pastnotecut.org : Johan Boule

///\file
#pragma once
#include <universalis/detail/project.hpp>
#if defined DIVERSALIS__COMPILER__GNU
	// typeof is a keyword
	#define DIVERSALIS__COMPILER__FEATURE__TYPEOF
#elif defined DIVERSALIS__COMPILER__MICROSOFT
	/// it looks like a reserved keyword, but it's not implemented yet as of version 7.1 (only in c#).
	#define typeof __typeof
#endif

// arch-tag: 5051ba29-2615-42cc-a4d7-6905a94b9b75
