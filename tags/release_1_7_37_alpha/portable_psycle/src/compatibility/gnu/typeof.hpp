///\file
#pragma once
#include INCLUDE(PROJECT/project.hpp)
#include INCLUDE(PROJECT/compilers.hpp)
#if defined COMPILER__GNU
	// typeof is a keyword
	#define COMPILER__FEATURE__TYPEOF
#elif defined COMPILER__MICROSOFT
	/// it looks like a reserved keyword, but it's not implemented yet as of version 7.1 (only in c#).
	#define typeof __typeof
#endif
