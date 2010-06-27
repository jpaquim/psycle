// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// Copyright (C) 1999-2005 Psycledelics http://psycle.pastnotecut.org : Johan Boule

///\file
#pragma once
#include <universalis/standard_library/ndebug.hpp>
#include "../pragmatic/pack.hpp"
#if defined DIVERSALIS__COMPILER__MICROSOFT
	#if defined NDEBUG // if no dedug
		#pragma runtime_checks("c", off) // reports when a value is assigned to a smaller data type that results in a data loss
		#pragma runtime_checks("s", off) // stack (frame) verification
		#pragma runtime_checks("u", off) // reports when a variable is used before it is defined
			// Run-time error checks is a way for you to find problems in your running code.
			// The run-time error checks feature is enabled and disabled by the /RTC group of compiler options and the runtime_checks pragma.
			// Note:
			// If you compile your program at the command line using any of the /RTC compiler options,
			// any pragma optimize instructions in your code will silently fail.
			// This is because run-time error checks are not valid in a release (optimized) build.
			// [bohan] this means we must not have the /RTC option enabled to enable optimizations, even if we disable runtime checks here.
		#pragma optimize("s", off) // favors small code
		#pragma optimize("t", on) // favors fast code
		#pragma optimize("p", off) // improves floating-point consistency (this is for iso conformance, slower and less precise)
		#pragma optimize("a", on) // assumes no aliasing, see also: the restrict iso keyword
		#pragma optimize("w", off) // assumes that aliasing can occur across function calls
		#pragma optimize("y", off) // generates frame pointers on the program stack
		#pragma optimize("g", on) // global optimization
			// [bohan] note: there is a bug in msvc 6 concerning global optimization ... disable it locally where necessary
			// [bohan] it hopefully generates a warning->error, otherwize, we would have a runtime bug
			// [bohan] warning C4702: unreachable code
			// [bohan] error C2220: warning treated as error - no object file generated
			// [bohan] this bug was with msvc 6. it seems to have been fixed with msvc 7.1,
			// [bohan] but it still appears with msvc 7.1 when using synchronous exception hanlding model.
		//no #pragma for /GL == enable whole program, accross .obj, optimization
			// [bohan] we have to disable that optimization in some projects because it's boggus.
			// [bohan] because it's only performed at link time it doesn't matter if the precompiled headers were done using it or not.
			// [bohan] static initialization and termination is done/ordered weirdly when enabled
			// [bohan] so, if you experience any weird bug,
			// [bohan] first thing to try is to disable this /GL option in the command line to cl, or
			// [bohan] in the project settings, it's hidden under General Properties / Whole Program Optimization.
		#pragma inline_depth(255)
		#pragma inline_recursion(on)
		#define inline __forceinline
	#endif
#endif

// arch-tag: 7225ee47-b6b2-4115-83e5-edd5a42e4f9d