// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// Copyright (C) 1999-2005 Psycledelics http://psycle.pastnotecut.org : Johan Boule

///\file
#pragma once
#include <universalis/detail/project.hpp>
#if defined DIVERSALIS__COMPILER__GNU
	// asm is a keyword
#elif defined DIVERSALIS__COMPILER__MICROSOFT
	#define asm __asm
#endif

// arch-tag: 1ddbf702-5e53-4ddf-8252-6af5400bbcfd
