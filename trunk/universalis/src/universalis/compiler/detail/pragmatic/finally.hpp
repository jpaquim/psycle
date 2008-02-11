/* -*- mode:c++, indent-tabs-mode:t -*- */
// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2007 johan boule <bohan@jabber.org>
// copyright 2004-2007 psycledelics http://psycle.pastnotecut.org

///\file
///\brief try-finally statements
#pragma once
#include <universalis/detail/project.hpp>
#if 0 // disabled ; as of january 2005, we still can't use try-finally statements easily but with borland's compiler.
	#if defined DIVERSALIS__COMPILER__FEATURE__NOT_CONCRETE
		#define try_finally
		#define finally
	#elif defined DIVERSALIS__COMPILER__GNU
		#define try_finally
		#define finally
	#elif defined DIVERSALIS__COMPILER__BORLAND
		#define try_finally try
		#define finally __finally
	#elif defined DIVERSALIS__COMPILER__MICROSOFT
		// msvc's try-finally statements are useless because they cannot be mixed with try-catch statements in the same function
		// [bohan] this was with msvc 6. it might have been fixed with msvc 7.1, but there's only little hope.
		//#define try_finally __try
		//#define finally __finally
	#else
		#error "Unsupported compiler ; please add support for try-finally statements for your compiler in the file where this error is triggered."
		/*
			#elif defined DIVERSALIS__COMPILER__<your_compiler_name>
				#define try_finally ...
				#define finally ...
			#endif
		*/
	#endif
#endif // 0
