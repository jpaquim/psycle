// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// Copyright (C) 1999-2005 Psycledelics http://psycle.pastnotecut.org : Johan Boule

///\file
///\brief stringization
#pragma once
#include <universalis/detail/project.hpp>
//#namespace UNIVERSALIS
	//#namespace COMPILER
		/// Interprets argument as a string litteral.
		/// The indirection in the call to # lets the macro expansion on the argument be done first.
		#define UNIVERSALIS__COMPILER__STRINGIZED(tokens) UNIVERSALIS__COMPILER__STRINGIZED__DETAIL__NO_EXPANSION(tokens)

		//#namespace DETAIL		
			///\internal
			/// Don't call this macro directly ; call UNIVERSALIS__COMPILER__STRINGIZED, which calls this macro after macro expansion is done on the argument.
			///\relates UNIVERSALIS__COMPILER__STRINGIZED
			#define UNIVERSALIS__COMPILER__STRINGIZED__DETAIL__NO_EXPANSION(tokens) #tokens
		//#endnamespace
	//#endnamespace
//#endnamespace

// arch-tag: 78b96abf-0b8a-411e-ae0a-924b667ac63b
