// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2008 members of the psycle project http://psycle.pastnotecut.org ; johan boule <bohan@jabber.org>

///\file
#pragma once
#include <universalis/detail/project.hpp>
#if defined DIVERSALIS__COMPILER__MICROSOFT

	#pragma warning(push, 4) // generate level-4 (i.e. all) warnings
		// [bohan] note: we never pops this one... but there's no syntax without push.

	//////////////////////////////
	// warnings treated as errors
	//////////////////////////////
	#pragma warning(error:4662) // explicit instantiation; template-class 'X' has no definition from which to specialize X<...>
	#pragma warning(error:4150) // deletion of pointer to incomplete type 'X<...>'; no destructor called
	#pragma warning(error:4518) // storage-class or type specifier(s) unexpected here; ignored

	/////////////
	// weirdness
	/////////////
	//#pragma warning(disable:4702) // unreachable code
		// see comments on exception handling (and also global optimization on msvc 6)
		// [bohan] actually, here is what i did:
		// [bohan] the problem appeared in <vector>, but seems to be due to code in <stdexcept>, which is #included by <vector>.
		// [bohan] i've just disabled the warning/error in precompiled_headers.h,
		// [bohan] around the #include <stdexcept> (which is #included just for the fix)
		// [bohan] maybe there will be more problematic headers appearing later, i'll do the same as with <stdexcept>.

	//////////
	// shiatz
	//////////
	#pragma warning(disable:4258) // definition from the for loop is ignored; the definition from the enclosing scope is used
	#pragma warning(disable:4673) // thrown exception type not catched
	#pragma warning(disable:4290) // c++ exception specification ignored (not yet implemented) except to indicate a function is not __declspec(nothrow)
		// "A function is declared using exception specification, which Visual C++ accepts but does not implement.
		// Code with exception specifications that are ignored during compilation may need to be
		// recompiled and linked to be reused in future versions supporting exception specifications."

	//////////
	// stupid
	//////////
	#pragma warning(disable:4251) // class 'X' needs to have dll-interface to be used by clients of class 'Y'
	#pragma warning(disable:4275) // non dll-interface class 'X' used as base for dll-interface class 'Y'"

	/////////
	// style
	/////////
	#pragma warning(disable:4554) // check operator precedence for possible error; use parentheses to clarify precedence
	#pragma warning(disable:4706) // assignment within conditional expression
	#pragma warning(disable:4127) // conditional expression is constant
	#pragma warning(disable:4100) // unreferenced formal parameter

	////////
	// cast
	////////
	#pragma warning(disable:4800) // forcing value to bool 'true' or 'false' (performance warning)
	#pragma warning(disable:4244) // conversion from 'numeric type A' to 'numeric type B', possible loss of data

	//////////
	// inline
	//////////
	#pragma warning(disable:4711) // selected for automatic inline expansion
	#pragma warning(disable:4710) // function was not inlined
	#pragma warning(disable:4714) // function marked as __forceinline not inlined

	/////////////////////////
	// implicit constructors
	/////////////////////////
	#pragma warning(disable:4512) // assignment operator could not be generated
	#pragma warning(disable:4511) // copy constructor could not be generated

	///////////////
	// yes, yes...
	///////////////
	#pragma warning(disable:4355) // 'this' : used in base member initializer list
	//#pragma warning(disable:4096) // '__cdecl' must be used with '...'
	#pragma warning(disable:4652) // compiler option 'link-time code generation (/GL)' inconsistent with precompiled header; current command-line option will override that defined in the precompiled header
		// see the comments about the /GL option in the optimization section
		// note:
		// since the warning is issued before the first line of code is even parsed,
		// it is not possible to disable it using a #pragma.
		// i just put it here for consistency and documentation, but you should disable it
		// using the /Wd4652 command line option to cl, or
		// in the project settings under C++ / Advanced / Disable Specific Warnings.
	#pragma warning(disable:4651) // '/DXXX' specified for pre-compiled header but not for current compilation
		// beware with this ... it is disabled because of /D_WINDLL ... otherwise it could be relevant
#endif
