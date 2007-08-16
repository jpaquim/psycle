// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2007 johan boule <bohan@jabber.org>
// copyright 2004-2007 psycledelics http://psycle.pastnotecut.org

///\file
#pragma once
#include "attribute.hpp"

#if defined DIVERSALIS__COMPILER__GNU
	#define UNIVERSALIS__COMPILER__CONST UNIVERSALIS__COMPILER__ATTRIBUTE(const)
#elif defined DIVERSALIS__COMPILER__INTEL
	#define UNIVERSALIS__COMPILER__CONST // supported.. check the doc
#else
	#define UNIVERSALIS__COMPILER__CONST
#endif
