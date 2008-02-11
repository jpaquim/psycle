/* -*- mode:c++, indent-tabs-mode:t -*- */
// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2007 johan boule <bohan@jabber.org>
// copyright 2004-2007 psycledelics http://psycle.pastnotecut.org

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
