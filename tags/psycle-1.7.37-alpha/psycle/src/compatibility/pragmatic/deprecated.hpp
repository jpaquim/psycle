///\file
/// deprecated warning.
/// declares a symbol as deprecated.
#pragma once
#include INCLUDE(PROJECT/project.hpp)
#include INCLUDE(PROJECT/compilers.hpp)
#include "pragma.hpp"
/// deprecated warning.
/// declares a symbol as deprecated.
#if defined COMPILER__GNU
	#define deprecated pragma(deprecated)
#elif defined COMPILER__MICROSOFT
	// deprecated is a keyword, not an attribute?
#else
	#define deprecated
#endif
