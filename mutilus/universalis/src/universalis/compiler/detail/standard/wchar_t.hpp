// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2007 johan boule <bohan@jabber.org>
// copyright 2004-2007 psycledelics http://psycle.pastnotecut.org

///\file
#pragma once
#include <universalis/compiler/numeric.hpp>
#if defined DIVERSALIS__COMPILER__MICROSOFT && 0 // [bohan] disabled... i can't help microsoft with this... let them fix that flaw.
	// the microsoft compiler has an old implementation of wchar_t: it is 16 bits while it actually must be 32 bits nowadays.
	// see also: _WCHAR_T_DEFINED _NATIVE_WCHAR_T_DEFINED
	#if defined wchar_t
		#undef wchar_t
		typedef universalis::compiler::numeric<32>::unsigned_int __identifier(wchar_t);
	#else
		#define wchar_t universalis::compiler::numeric<32>::unsigned_int
		// we cannot use a typedef since wchar_t is already a type, and we cannot #undef a type.
	#endif
#endif
