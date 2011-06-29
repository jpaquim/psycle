// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2011 members of the psycle project http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>

#include "attribute.hpp"

#if defined DIVERSALIS__COMPILER__GNU
	#define UNIVERSALIS__COMPILER__CONST UNIVERSALIS__COMPILER__ATTRIBUTE(const)
#elif defined DIVERSALIS__COMPILER__INTEL
	///\todo supported.. check the doc
	#define UNIVERSALIS__COMPILER__CONST
#else
	#define UNIVERSALIS__COMPILER__CONST
#endif
