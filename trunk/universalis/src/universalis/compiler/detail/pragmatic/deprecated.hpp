/* -*- mode:c++, indent-tabs-mode:t -*- */
// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2007 johan boule <bohan@jabber.org>
// copyright 2004-2007 psycledelics http://psycle.pastnotecut.org

///\file
/// deprecated warning.
/// declares a symbol as deprecated.
#pragma once
#include "attribute.hpp"
/// deprecated warning.
/// declares a symbol as deprecated.
#if defined DIVERSALIS__COMPILER__GNU
	#define UNIVERSALIS__COMPILER__DEPRECATED(message) UNIVERSALIS__COMPILER__ATTRIBUTE(deprecated)
#elif defined DIVERSALIS__COMPILER__MICROSOFT
	#if DIVERSALIS__COMPILER__VERSION < 130000000
		#define UNIVERSALIS__COMPILER__DEPRECATED(message)
	#elif DIVERSALIS__COMPILER__VERSION < 140050320
		#define UNIVERSALIS__COMPILER__DEPRECATED(message) UNIVERSALIS__COMPILER__ATTRIBUTE(deprecated)
	#else
		#define UNIVERSALIS__COMPILER__DEPRECATED(message) UNIVERSALIS__COMPILER__ATTRIBUTE(deprecated(message))
	#endif
#else
	#define UNIVERSALIS__COMPILER__DEPRECATED
#endif
