///\file
/// pragmatic attributes
#pragma once
#include INCLUDE(PROJECT/project.hpp)
#include INCLUDE(PROJECT/compilers.hpp)
/// pragmatic attributes
#if defined COMPILER__GNU
	#define pragma(x) __attribute__ ((x))
#elif defined COMPILER__MICROSOFT
	#define pragma(x) __declspec(x)
#else
	#define pragma(x)
#endif
