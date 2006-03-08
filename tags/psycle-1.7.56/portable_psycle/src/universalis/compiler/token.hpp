// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// Copyright (C) 1999-2005 Psycledelics http://psycle.pastnotecut.org : Johan Boule

///\file
///\brief tokenization
#pragma once
#include <universalis/detail/project.hpp>
//#namespace UNIVERSALIS
	//#namespace COMPILER
		/// tokenize.
		/// The indirection in the call to # lets the macro expansion on the argument be done first.
		#define UNIVERSALIS__COMPILER__TOKEN(token) UNIVERSALIS__COMPILER__TOKEN__NO_EXPANSION(token)
		/// Don't call this macro directly ; call UNIVERSALIS__COMPILER__TOKEN, which calls this macro after macro expansion is done on the argument.
		///\relates UNIVERSALIS__COMPILER__TOKEN
		#define UNIVERSALIS__COMPILER__TOKEN__NO_EXPANSION(token) token
	//#endnamespace
//#endnamespace
