///\file
/// the restrict keyword has been introduced in the iso 1999 standard.
/// it is a hint for the compiler telling it several references or pointers will *always* holds different memory addresses.
/// hence it can optimize more, knowing that writting to memory via one reference cannot alias another.
/// example:
	/// void f(int & restrict r1, int & restrict r2, int * restrict p1, int restrict p2[]);
	/// here the compiler is told that &r1 != &r2 != p1 != p2
#pragma once
#include INCLUDE(PROJECT/project.hpp)
#include INCLUDE(PROJECT/compilers.hpp)
#if defined COMPILER__GNU
	// restrict is a keyword
	//#define restrict __restrict__
#elif defined COMPILER__MICROSOFT
	#define restrict
	// unsupported?
	// see also: #pragma optimize("a", on) // assumes no aliasing
#else
	#define restrict
#endif
