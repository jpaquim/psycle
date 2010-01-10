// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2004-2010 members of the psycle project http://psycle.sourceforge.net

// steinberg's headers are unable to correctly detect the platform,
// so we need to detect the platform ourselves,
// and declare steinberg's unstandard/specific options: WIN32/MAC
#include <diversalis/os.hpp>

#if defined DIVERSALIS__OS__MICROSOFT
	#if defined _WIN64
		#error internal steinberg error (actually, it might work)
	#elif defined _WIN32
		#define WIN32 // steinberg's asio build option
	#else
		#error internal steinberg error
	#endif

	// steinberg's headers also lack some #include,
	// so we include the missing headers ourselves
	#include <objbase.h>

#elif defined DIVERSALIS__OS__APPLE
	#define MAC // steinberg's asio build option

#else
	#error internal steinberg error
#endif
