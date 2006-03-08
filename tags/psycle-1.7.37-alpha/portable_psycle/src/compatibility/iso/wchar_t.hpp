///\file
#pragma once
#include INCLUDE(PROJECT/project.hpp)
#include INCLUDE(PROJECT/compilers.hpp)
#if defined COMPILER__MICROSOFT && 0 // [bohan] disabled... i can't help microsoft with this... let them fix that flaw.
	// the microsoft compiler has a broken implementation of wchar_t: it is 16 bits while it actually must be 32 bits.
	// see also: _WCHAR_T_DEFINED _NATIVE_WCHAR_T_DEFINED
	#if defined wchar_t
		#undef wchar_t
		typedef compiler::uint32 __identifier(wchar_t);
	#else
		#define wchar_t compiler::uint32
		// we cannot use a typedef since wchar_t is already a type, and we cannot #undef a type.
	#endif
#endif
