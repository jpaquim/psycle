// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2013-2013 members of the psycle project http://psycle.pastnotecut.org ; johan boule <bohan@jabber.org>

#pragma once
#include "stringize.hpp"

// see http://stackoverflow.com/questions/471935/user-warnings-on-msvc-and-gcc

#ifdef DIVERSALIS__COMPILER__MICROSOFT
	#if DIVERSALIS__COMPILER__VERSION__MAJOR >= 15
		#define DIVERSALIS__COMPILER__MESSAGE(str) \
			__pragma(message(__FILE__ "(" DIVERSALIS__COMPILER__STRINGIZE(__LINE__) ") : " str))
	#else
		#define DIVERSALIS__COMPILER__MESSAGE(str)
	#endif
#else
	/// ISO C-1999 _Pragma operator
	#define DIVERSALIS__COMPILER__MESSAGE(str) \
		_Pragma(DIVERSALIS__COMPILER__STRINGIZE(message str))
#endif

#define DIVERSALIS__COMPILER__WARNING(str) DIVERSALIS__COMPILER__MESSAGE("warning: " str)
