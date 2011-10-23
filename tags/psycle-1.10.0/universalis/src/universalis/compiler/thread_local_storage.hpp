// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2004-2011 members of the psycle project http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>

///\file
///\brief thread_local keyword - thread local storage

#pragma once
#include <universalis/detail/project.hpp>

#if defined DIVERSALIS__COMPILER__FEATURE__CXX0X && !defined DIVERSALIS__STDLIB__CXX0X__BROKEN__THREAD
	// thread_local is a keyword
#else
	#include "attribute.hpp"

	#if \
		defined DIVERSALIS__COMPILER__GNU && ( \
			/* TLS support added on windows only in version 4.3.0 */ \
			!defined DIVERSALIS__OS__MICROSOFT || \
			DIVERSALIS__COMPILER__VERSION >= 40300 \
		)

		#define thread_local __thread
	#elif defined DIVERSALIS__COMPILER__MICROSOFT
		#define thread_local UNIVERSALIS__COMPILER__ATTRIBUTE(thread)
	#else
		/// variable stored in a per thread local storage.
		#define thread_local

		#if !defined DIVERSALIS__COMPILER__FEATURE__NOT_CONCRETE && defined DIVERSALIS__COMPILER__FEATURE__WARNING
			#warning "Cannot generate thread-safe code for this compiler ; please add support for thread-local storage for your compiler in the file where this warning is triggered."
		#endif
	#endif
#endif
