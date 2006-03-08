// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// Copyright (C) 1999-2005 Psycledelics http://psycle.pastnotecut.org : Johan Boule

///\file
/// thread local storage.
/// variable stored in a per thread local storage.
#pragma once
#include "attribute.hpp"
/// thread local storage.
/// variable stored in a per thread local storage.
#if defined DIVERSALIS__COMPILER__GNU || defined DIVERSALIS__COMPILER__MICROSOFT
	#define UNIVERSALIS__COMPILER__THREAD_LOCAL_STORAGE UNIVERSALIS__COMPILER__ATTRIBUTE(thread)
#else
	#error "Unsupported compiler ; please add support for thread local storage for your compiler in the file where this error is triggered."
	/*
		#elif defined DIVERSALIS__COMPILER__<your_compiler_name>
			#define UNIVERSALIS__COMPILER__THREAD_LOCAL_STORAGE ...
		#endif
	*/
#endif

// arch-tag: 4c82d1ad-ff2d-4846-b23b-0de074e9fcbe
