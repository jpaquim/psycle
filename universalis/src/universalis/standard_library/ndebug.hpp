// -*- mode:c++; indent-tabs-mode:t -*-
// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2008 members of the psycle project http://psycle.pastnotecut.org ; johan boule <bohan@jabber.org>

///\file circumvant usual lack of standardisation for the NDEBUG preprocessor definition on microsoft-based projects
#pragma once
#define UNIVERSALIS__STANDARD_LIBRARY__NDEBUG__INCLUDED
#include <universalis/detail/project.hpp>
#if defined DIVERSALIS__COMPILER__MICROSOFT
	// microsoft uses its own vendor #define _DEBUG (for debug), instead of the iso #define NDEBUG (for no debug)
	// so, we recover the iso way
	#if defined NDEBUG
		#if defined _DEBUG
			#error "We have both ISO #define NDEBUG (no debug) and microsoft's vendor #define _DEBUG (debug) ... Is this intended?"
		#endif
	#elif defined _DEBUG
	#if defined UNIVERSALIS__COMPILER__VERBOSE
		#pragma message("universalis::standard_library:: debug ; ("__FILE__ ")")
	#endif
	#else
		#define NDEBUG
	#endif
	#if defined UNIVERSALIS__COMPILER__VERBOSE && defined NDEBUG
		#pragma message("universalis::standard_library:: no debug (NDEBUG) ; ("__FILE__ ")")
	#endif
#elif defined DIVERSALIS__COMPILER__DOXYGEN
	/// parts of the source code that contain instructions only useful for debugging purpose
	/// shall be encapsulated inside:
	///\code
	/// #if !defined NDEBUG
	///  some debugging stuff
	/// #endif
	///\endcode
	#define NDEBUG
#endif
