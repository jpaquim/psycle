// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2004-2010 members of the psycle project http://psycle.pastnotecut.org ; johan boule <bohan@jabber.org>

///\file
/// stuff related with virtual member functions

#pragma once
#include <universalis/detail/project.hpp>
#include "attribute.hpp"

/// pure virtual classes
#if \
	defined DIVERSALIS__COMPILER__GNU || \
	defined DIVERSALIS__COMPILER__MICROSOFT
	#define UNIVERSALIS__COMPILER__PURE_VIRTUAL UNIVERSALIS__COMPILER__ATTRIBUTE(novtable)
#else
	#define UNIVERSALIS__COMPILER__PURE_VIRTUAL
#endif

/// override keyword - overridding of virtual member function
#if \
	defined DIVERSALIS__COMPILER__FEATURE__CXX0X && \
	/* gcc since version 4.7 http://gcc.gnu.org/gcc-4.7/changes.html */ \
	defined DIVERSALIS__COMPILER__GNU && DIVERSALIS__COMPILER__VERSION > 407000
	// override is a keyword
#else
	#define override
#endif
