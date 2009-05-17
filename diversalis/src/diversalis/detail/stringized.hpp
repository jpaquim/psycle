// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2007 johan boule <bohan@jabber.org>
// copyright 2004-2007 psycledelics http://psycle.pastnotecut.org

///\file

#ifndef DIVERSALIS__DETAIL__STRINGIZED__INCLUDED
#define DIVERSALIS__DETAIL__STRINGIZED__INCLUDED
#pragma once

/// Interprets argument as a string litteral.
/// The indirection in the call to # lets the macro expansion on the argument be done first.
#define DIVERSALIS__STRINGIZED(tokens) DIVERSALIS__STRINGIZED__DETAIL__NO_EXPANSION(tokens)

///\internal
/// Don't call this macro directly ; call DIVERSALIS__STRINGIZED, which calls this macro after macro expansion is done on the argument.
///\relates DIVERSALIS__STRINGIZED
#define DIVERSALIS__STRINGIZED__DETAIL__NO_EXPANSION(tokens) #tokens

#endif
