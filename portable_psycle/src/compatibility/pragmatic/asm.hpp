///\file
#pragma once
#include INCLUDE(PROJECT/project.hpp)
#include INCLUDE(PROJECT/compilers.hpp)
#if defined COMPILER__GNU
	// asm is a keyword
#elif defined COMPILER__MICROSOFT
	#define asm __asm
#endif
