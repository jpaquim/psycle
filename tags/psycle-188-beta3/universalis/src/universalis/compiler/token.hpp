// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2007 johan boule <bohan@jabber.org>
// copyright 2004-2007 psycledelics http://psycle.pastnotecut.org

///\file
///\brief tokenization

#ifndef UNIVERSALIS__COMPILER__TOKEN__INCLUDED
#define UNIVERSALIS__COMPILER__TOKEN__INCLUDED
#pragma once

#include <universalis/detail/project.hpp>

/// tokenize.
/// The indirection in the call lets the macro expansion on the argument be done first.
#define UNIVERSALIS__COMPILER__TOKEN(token) UNIVERSALIS__COMPILER__TOKEN__DETAIL__NO_EXPANSION(token)
		
///\internal
/// Don't call this macro directly ; call UNIVERSALIS__COMPILER__TOKEN, which calls this macro after macro expansion is done on the argument.
///\relates UNIVERSALIS__COMPILER__TOKEN
#define UNIVERSALIS__COMPILER__TOKEN__DETAIL__NO_EXPANSION(token) token

#endif
