///\file
#pragma once
#include INCLUDE(PROJECT/project.hpp)
#include INCLUDE(PROJECT/compilers.hpp)
// [bohan] this keyword is just syntactic sugar, so, we don't *require* it.
#if defined COMPILER__GNU
	// unsupported?
#elif defined COMPILER__MICROSOFT
	// super is a keyword
#else
	// prolly unsupported
#endif
