// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// Copyright (C) 1999-2005 Psycledelics http://psycle.pastnotecut.org : Johan Boule

///\file
#pragma once
#include "attribute.hpp"

#if defined DIVERSALIS__COMPILER__GNU
	#define UNIVERSALIS__COMPILER__WEAK UNIVERSALIS__COMPILER__ATTRIBUTE(weak)
#elif defined DIVERSALIS__COMPILER__MICROSOFT
	#define UNIVERSALIS__COMPILER__WEAK UNIVERSALIS__COMPILER__ATTRIBUTE(selectany)
#else
	#define UNIVERSALIS__COMPILER__WEAK
#endif
