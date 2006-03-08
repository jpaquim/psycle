///\file
#pragma once
#include INCLUDE(PROJECT/project.hpp)
#include INCLUDE(PROJECT/compilers.hpp)
/// [bohan] i don't quite understand what microsoft says about this keyword...
#if defined COMPILER__GNU
	// unsupported?
	#define assume(x)
#elif defined COMPILER__MICROSOFT
	#define assume(x) __assume(x)
#else
	#define assume(x)
#endif
