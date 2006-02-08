// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// Copyright (C) 1999-2006 Johan Boule <bohan@jabber.org>
// Copyright (C) 2004-2006 Psycledelics http://psycle.pastnotecut.org

///\file
///\brief tokenization
#pragma once
#include <universalis/detail/project.hpp>
//#namespace UNIVERSALIS
	//#namespace COMPILER
		/// tokenize.
		/// The indirection in the call lets the macro expansion on the argument be done first.
		#define UNIVERSALIS__COMPILER__TOKEN(token) UNIVERSALIS__COMPILER__TOKEN__DETAIL__NO_EXPANSION(token)
		
		//#namespace DETAIL
			///\internal
			/// Don't call this macro directly ; call UNIVERSALIS__COMPILER__TOKEN, which calls this macro after macro expansion is done on the argument.
			///\relates UNIVERSALIS__COMPILER__TOKEN
			#define UNIVERSALIS__COMPILER__TOKEN__DETAIL__NO_EXPANSION(token) token
		//#endnamespace
	//#endnamespace
//#endnamespace
