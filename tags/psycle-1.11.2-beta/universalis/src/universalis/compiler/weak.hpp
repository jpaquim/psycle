// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2004-2011 members of the psycle project http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>

#pragma once
#include "attribute.hpp"

///\file

#if defined DIVERSALIS__COMPILER__GNU
	#define UNIVERSALIS__COMPILER__WEAK UNIVERSALIS__COMPILER__ATTRIBUTE(__weak__)
#elif defined DIVERSALIS__COMPILER__MICROSOFT
	#define UNIVERSALIS__COMPILER__WEAK UNIVERSALIS__COMPILER__ATTRIBUTE(selectany)
#else
	#define UNIVERSALIS__COMPILER__WEAK
#endif
