// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// Copyright (C) 1999-2005 Psycledelics http://psycle.pastnotecut.org : Johan Boule

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

// arch-tag: 256b7a9a-0831-4c10-8cf2-86305fb40c24
