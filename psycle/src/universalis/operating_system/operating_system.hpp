// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2007 psycledelics http://psycle.pastnotecut.org : johan boule

///\file
///\brief meta header
#pragma once
#include "detail/microsoft/max_path.hpp"


/// end-of-line character sequence on the host platform.

#if defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT
	#define UNIVERSALIS__OPERATING_SYSTEM__EOL "\r\n"
#elif defined DIVERSALIS__OPERATING_SYSTEM__APPLE && DIVERSALIS__OPERATING_SYSTEM__VERSION__MAJOR < 10 // before bsd-unix (darwin)
	#define UNIVERSALIS__OPERATING_SYSTEM__EOL "\r"
#else
	#define UNIVERSALIS__OPERATING_SYSTEM__EOL "\n"
#endif
