// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2007 johan boule <bohan@jabber.org>
// copyright 2004-2007 psycledelics http://psycle.pastnotecut.org

///\file
///\brief stringization
#pragma once
#include <universalis/detail/project.hpp>
//#region UNIVERSALIS
	//#region COMPILER
		/// Interprets argument as a string litteral.
		#if !defined UNIVERSALIS__QUAQUAVERSALIS || !defined DIVERSALIS__COMPILER__GNU || !defined DIVERSALIS__COMPILER__MICROSOFT
			// boost has workarounds for broken preprocessors
			#define UNIVERSALIS__COMPILER__STRINGIZED(tokens) BOOST_PP_STRINGIZE(tokens)
			#include <boost/preprocessor/stringize.hpp>
		#else
			#define UNIVERSALIS__COMPILER__STRINGIZED(tokens) UNIVERSALIS__COMPILER__STRINGIZED__DETAIL__NO_EXPANSION(tokens)
			#define UNIVERSALIS__COMPILER__STRINGIZED__DETAIL__NO_EXPANSION(tokens) #tokens
		#endif
	//#endregion
//#endregion
