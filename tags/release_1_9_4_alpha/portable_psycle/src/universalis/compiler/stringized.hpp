// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// Copyright (C) 1999-2006 Johan Boule <bohan@jabber.org>
// Copyright (C) 2004-2006 Psycledelics http://psycle.pastnotecut.org

///\file
///\brief stringization
#pragma once
#include <universalis/detail/project.hpp>
#include <boost/preprocessor/stringize.hpp>
//#region UNIVERSALIS
	//#region COMPILER
		/// Interprets argument as a string litteral.
		#define UNIVERSALIS__COMPILER__STRINGIZED(tokens) BOOST_PP_STRINGIZE(tokens)
	//#endregion
//#endregion
