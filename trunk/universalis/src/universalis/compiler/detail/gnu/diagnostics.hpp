// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2008 members of the psycle project http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>

///\file
#pragma once
#include <universalis/detail/project.hpp>
#if defined DIVERSALIS__COMPILER__GNU
	#if DIVERSALIS__COMPILER__VERSION >= 40200 // 4.2.0
		#pragma GCC diagnostic error "-Wsequence-point"
			// Warn about code that may have undefined semantics because of violations of sequence point rules in the C and C++ standards.
			// man gcc 3.4.4 and 4.1.2: The present implementation of this option only works for C programs.  A future implementation may also work for C++ programs.

		#pragma GCC diagnostic error "-Wreturn-type"
			// Warn whenever a function is defined with a return-type that defaults to int. Also warn about any return statement with no return-value in a function whose return-type is not void.

		#pragma GCC diagnostic error "-Wuninitialized"
			// Warn if an automatic variable is used without first being initialized or if a variable may be clobbered by a setjmp call.
	#endif
#endif
