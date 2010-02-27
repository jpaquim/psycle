// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2004-2010 members of the psycle project http://psycle.pastnotecut.org ; johan boule <bohan@jabber.org>

///\file \brief thread local storage.

#ifndef UNIVERSALIS__COMPILER__THREAD_LOCAL_STORAGE__INCLUDED
#define UNIVERSALIS__COMPILER__THREAD_LOCAL_STORAGE__INCLUDED
#pragma once

#include "attribute.hpp"

#if defined DIVERSALIS__COMPILER__GNU && ( \
		!defined DIVERSALIS__OS__MICROSOFT || \
		DIVERSALIS__COMPILER__VERSION >= 40300 /* TLS support added on windows in version 4.3.0 */ \
	)

	#define UNIVERSALIS__COMPILER__THREAD_LOCAL_STORAGE  __thread

#elif defined DIVERSALIS__COMPILER__MICROSOFT

	#define UNIVERSALIS__COMPILER__THREAD_LOCAL_STORAGE  UNIVERSALIS__COMPILER__ATTRIBUTE(thread)

#else
	/// variable stored in a per thread local storage.
	#define UNIVERSALIS__COMPILER__THREAD_LOCAL_STORAGE

	#if !defined DIVERSALIS__COMPILER__FEATURE__NOT_CONCRETE && defined DIVERSALIS__COMPILER__FEATURE__WARNING
		#warning "Cannot generate thread-safe code for this compiler ; please add support for thread-local storage for your compiler in the file where this warning is triggered."
	#endif

#endif

#endif
