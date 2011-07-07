// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2011 members of the psycle project http://psycle.pastnotecut.org ; johan boule <bohan@jabber.org>

#pragma once
#include <diversalis.hpp>

// consistency check
#if defined NDEBUG && defined DIVERSALIS__STDLIB__RUNTIME__DEBUG
	#ifdef DIVERSALIS__COMPILER__FEATURE__WARNING
		#warning assertions are off and we are using a debug runtime. Is this intended?
	#else
		#pragma message(__FILE__ "(" __LINE__ ") : warning: assertions are off and we are using a debug runtime. Is this intended?")
	#endif
#elif !defined NDEBUG && !defined DIVERSALIS__STDLIB__RUNTIME__DEBUG && \
	/* debug runtime is currently only detected on msvc */ defined DIVERSALIS__COMPILER__MICROSOFT
	#ifdef DIVERSALIS__COMPILER__FEATURE__WARNING
		#warning assertions are on and we are not using a debug runtime. Is this intended?
	#else
		#pragma message(__FILE__ "(" __LINE__ ") : warning: assertions are on and we are not using a debug runtime. Is this intended?")
	#endif
#endif

#ifdef NDEBUG // if no dedug
	#ifdef DIVERSALIS__COMPILER__MICROSOFT
		#if defined UNIVERSALIS__COMPILER__VERBOSE
			#pragma message(__FILE__ "(" __LINE__ ") : universalis::compiler:: setting optimizations on")
		#endif

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
		#pragma optimize("y", off) // generates frame pointers on the program stack
		#pragma optimize("g", on) // global optimization

		//no #pragma for /GL == enable whole program, accross .obj, optimization
			// [bohan] we have to disable that optimization in some projects because it's boggus.
			// [bohan] because it's only performed at link time it doesn't matter if the precompiled headers were done using it or not.
			// [bohan] static initialization and termination is done/ordered weirdly when enabled
			// [bohan] so, if you experience any weird bug,
			// [bohan] first thing to try is to disable this /GL option in the command line to cl, or
			// [bohan] in the project settings, it's hidden under General Properties / Whole Program Optimization.

		//#pragma inline_depth(255)
		//#pragma inline_recursion(on)
		//#define inline __forceinline
		
		#define _SECURE_SCL 0 // disable checked iterators. see http://msdn.microsoft.com/en-us/library/aa985896.aspx
		#define _HAS_ITERATOR_DEBUGGING 0 // see http://msdn.microsoft.com/en-us/library/aa985939.aspx
	#endif
#endif
