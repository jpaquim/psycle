// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// Copyright (C) 1999-2006 Johan Boule <bohan@jabber.org>
// Copyright (C) 2004-2006 Psycledelics http://psycle.pastnotecut.org

///\file
/// memory alignment
#pragma once
#include "attribute.hpp"
#if defined DIVERSALIS__COMPILER__GNU
	#define UNIVERSALIS__COMPILER__ALIGNED(bytes) UNIVERSALIS__COMPILER__ATTRIBUTE(aligned(bytes))
	// note: a bit field is packed to 1 bit, not one byte.
#elif defined DIVERSALIS__COMPILER__MICROSOFT
	#define UNIVERSALIS__COMPILER__ALIGNED(bytes) UNIVERSALIS__COMPILER__ATTRIBUTE(align(bytes))
	// see also: #pragma pack(x) in the optimization section
#else
	/// memory alignment.
	///\see packed
	#define UNIVERSALIS__COMPILER__ALIGNED(bytes)
#endif
