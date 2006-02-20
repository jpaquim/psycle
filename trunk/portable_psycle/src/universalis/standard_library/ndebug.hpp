// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// Copyright (C) 1999-2006 Johan Boule <bohan@jabber.org>
// Copyright (C) 2004-2006 Psycledelics http://psycle.pastnotecut.org

///\file
#pragma once
#include <universalis/detail/project.hpp>
#if defined DIVERSALIS__COMPILER__MICROSOFT
	// microsoft uses its own vendor #define _DEBUG (for debug), instead of the iso #define NDEBUG (for no debug)
	// so, we recover the iso way
	#if defined NDEBUG
		#if defined _DEBUG
			#error "We have both ISO #define NDEBUG (no debug) and microsoft's vendor #define _DEBUG (debug) ... Is this intended?"
		#endif
	#elif defined _DEBUG
		//#pragma message(__FILE__ ": debug")
	#else
		#define NDEBUG
		//#pragma message(__FILE__ ": no debug")
	#endif
#elif defined DIVERSALIS__COMPILER__DOXYGEN
	/// parts of the source code that contain instructions only useful for debugging purpose
	/// shall be encapsulated inside:
	///\code
	/// #if !defined NDEBUG
	/// 	some debugging stuff
	/// #endif
	///\endcode
	#define NDEBUG
#endif
