// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// Copyright (C) 1999-2005 Psycledelics http://psycle.pastnotecut.org : Johan Boule

///\file
#pragma once
#include <universalis/detail/project.hpp>
// [bohan] this keyword is just syntactic sugar, so, we don't *require* it.
#if defined DIVERSALIS__COMPILER__GNU
	// unsupported?
#elif defined DIVERSALIS__COMPILER__MICROSOFT
	// super is a keyword
#else
	// prolly unsupported
#endif

// arch-tag: 829c7780-448a-473a-bc6a-bfa9b12c5fd1
