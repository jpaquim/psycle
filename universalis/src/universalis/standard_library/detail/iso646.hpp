// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2004-2007 psycle development team http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>

///\file
#pragma once
#include <universalis/detail/project.hpp>
#if defined DIVERSALIS__COMPILER__MICROSOFT
	// _WCHAR_T_DEFINED defined when -Zc:wchar_t is used or if wchar_t was defined by system header include.
	// _NATIVE_WCHAR_T_DEFINED defined when -Zc:wchar_t is used.

	// with regards to alternate keywords for operators, the compiler conforms with C1990 but not C++1998
	#include <ciso646>
#endif
