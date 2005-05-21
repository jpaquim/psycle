// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// Copyright (C) 1999-2005 Psycledelics http://psycle.pastnotecut.org : Johan Boule

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

// arch-tag: 190c2c2d-869e-45c7-a5cd-5be00089a7a5
