// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2007 johan boule <bohan@jabber.org>
// copyright 2004-2007 psycledelics http://psycle.pastnotecut.org

///\file
/// thread local storage.
/// variable stored in a per thread local storage.
#pragma once
#include "attribute.hpp"
/// thread local storage.
/// variable stored in a per thread local storage.
#if defined DIVERSALIS__COMPILER__GNU && \
	!defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT && /* mingw 3.2.4 has no support. */ \
	!defined DIVERSALIS__OPERATING_SYSTEM__CYGWIN // todo needs to check whether cygwin has support.
		#define UNIVERSALIS__COMPILER__THREAD_LOCAL_STORAGE __thread
#elif defined DIVERSALIS__COMPILER__MICROSOFT
	#define UNIVERSALIS__COMPILER__THREAD_LOCAL_STORAGE UNIVERSALIS__COMPILER__ATTRIBUTE(thread)
#elif 0
	#error "Unsupported compiler ; please add support for thread local storage for your compiler in the file where this error is triggered."
	/*
		#elif defined DIVERSALIS__COMPILER__<your_compiler_name>
			#define UNIVERSALIS__COMPILER__THREAD_LOCAL_STORAGE ...
		#endif
	*/
#else ///\todo for now this is made optional but should eventually be fixed (still, don't error for unconcrete compilers)
	#define UNIVERSALIS__COMPILER__THREAD_LOCAL_STORAGE
#endif
