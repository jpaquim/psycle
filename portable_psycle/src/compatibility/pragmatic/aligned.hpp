///\file
/// memory alignment
#pragma once
#include INCLUDE(PROJECT/project.hpp)
#include INCLUDE(PROJECT/compilers.hpp)
#include "pragma.hpp"
#if defined COMPILER__GNU
	#define aligned(x) pragma(aligned(x))
	//#define packed pragma(packed)
	// note: a bit field is packed to 1 bit, not one byte.
#elif defined COMPILER__MICROSOFT
	#define aligned(x) pragma(align(x))
	//#define packed pragma(packed)
	// see also: #pragma pack(x) in the optimization section
#else
	/// memory alignment.
	///\see packed
	#define aligned(x)
	/// memory alignment.
	///\see aligned()
	#define packed
#endif
