// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2008 members of the psycle project http://psycle.pastnotecut.org ; johan boule <bohan@jabber.org>

///\file \brief compiler-independant meta-information about the compiler
#ifndef DIVERSALIS__COMPILER__INCLUDED
#define DIVERSALIS__COMPILER__INCLUDED
#pragma once
#include <diversalis/detail/project.hpp>

#if defined DIVERSALIS__COMPILER__DOXYGEN

	/**********************************************************************************/
	// documentation about what is defined in this file

	///\name meta-information about the compiler's version
	///\{
		/// compiler version, as a string.
		#define DIVERSALIS__COMPILER__VERSION__STRING <string>

		/// compiler version, as an integral number.
		/// This combines the major, minor and patch numbers into a single integral number.
		#define DIVERSALIS__COMPILER__VERSION <number>

		/// compiler version, major number.
		#define DIVERSALIS__COMPILER__VERSION__MAJOR <number>

		/// compiler version, minor number.
		#define DIVERSALIS__COMPILER__VERSION__MINOR <number>

		/// compiler version, patch number.
		#define DIVERSALIS__COMPILER__VERSION__PATCH <number>

		/// compiler version, application binary interface number.
		#define DIVERSALIS__COMPILER__VERSION__ABI <number>
	///\}

	///\name meta-information about the compiler's features
	///\{
		/// indicates the compiler supports pre-compilation.
		#define DIVERSALIS__COMPILER__FEATURE__PRE_COMPILATION
		#undef DIVERSALIS__COMPILER__FEATURE__PRE_COMPILTION // was just defined to insert documentation.
	
		/// indicates the compiler supports auto-linking. e.g.: #pragma comment(lib, "foo")
		#define DIVERSALIS__COMPILER__FEATURE__AUTO_LINK
		#undef DIVERSALIS__COMPILER__FEATURE__AUTO_LINK // was just defined to insert documentation.

		/// indicates the compiler supports the #warning preprocessor directive.
		#define DIVERSALIS__COMPILER__FEATURE__WARNING
		#undef DIVERSALIS__COMPILER__FEATURE__WARNING // was just defined to insert documentation.

		/// indicates the compiler does not generate code for a processor.
		/// e.g.: the eclipse cdt c++ indexer, the doxygen documentation compiler, resource compilers, etc.
		#define DIVERSALIS__COMPILER__FEATURE__NOT_CONCRETE
		#undef DIVERSALIS__COMPILER__FEATURE__NOT_CONCRETE // was just defined to insert documentation.

		/// indicates the compiler supports some assembler language.
		#define DIVERSALIS__COMPILER__FEATURE__ASSEMBLER
		#undef DIVERSALIS__COMPILER__FEATURE__ASSEMBLER // was just defined to insert documentation.

		///\todo doc
		#define DIVERSALIS__COMPILER__FEATURE__XMM_INTRINSICS
		#undef DIVERSALIS__COMPILER__FEATURE__XMM_INTRINSICS // was just defined to insert documentation.
	///\}

	///\name meta-information about the compiler's assembler language syntax
	///\{
		/// indicates the compiler's assembler language has at&t's syntax.
		#define DIVERSALIS__COMPILER__ASSEMBLER__ATT
		#undef DIVERSALIS__COMPILER__ASSEMBLER__ATT // was just defined to insert documentation.

		/// indicates the compiler's assembler language has intel's syntax.
		#define DIVERSALIS__COMPILER__ASSEMBLER__INTEL
		#undef DIVERSALIS__COMPILER__ASSEMBLER__INTEL // was just defined to insert documentation.
	///\}

	///\name meta-information about the compiler's brand
	///\{
		/// gnu g++/gcc, g++, autodetected via __GNUG__ (equivalent to __GNUC__ && __cplusplus).
		/// To see the predefined macros, run: g++ -xc++ -std=c++98 -dM -E /dev/null
		#define DIVERSALIS__COMPILER__GNU
		#undef DIVERSALIS__COMPILER__GNU // was just defined to insert documentation.

		/// bcc, autodetected via __BORLAND__.
		#define DIVERSALIS__COMPILER__BORLAND
		#undef DIVERSALIS__COMPILER__BORLAND // was just defined to insert documentation.

		/// msvc, autodetected via _MSC_VER.
		#define DIVERSALIS__COMPILER__MICROSOFT
		#undef DIVERSALIS__COMPILER__MICROSOFT // was just defined to insert documentation.

		#undef DIVERSALIS__COMPILER__DOXYGEN // redefine to insert documentation.
		/// doxygen documentation compiler. This is not autodetected and has to be manually defined.
		#define DIVERSALIS__COMPILER__DOXYGEN

		/// eclipse cdt c++ indexer. This is not autodetected and has to be manually defined.
		#define DIVERSALIS__COMPILER__ECLIPSE
		#undef DIVERSALIS__COMPILER__ECLIPSE // was just defined to insert documentation.

		/// resource compiler (only relevant on microsoft's operating system), autodetected via RC_INVOKED.
		#define DIVERSALIS__COMPILER__RESOURCE
		#undef DIVERSALIS__COMPILER__RESOURCE // was just defined to insert documentation.
	///\}
#endif



/**********************************************************************************/
// now the real work

//////////////////////////////////////////////////////////////////////
// resource compilers (only relevant on microsoft's operating system)

// RC_INVOKED is defined by resource compilers (only relevant on microsoft's operating system).
#if defined RC_INVOKED
	// don't #define DIVERSALIS__COMPILER ; we can determine the real preprocessor used for parsing.
	#define DIVERSALIS__COMPILER__RESOURCE
	#define DIVERSALIS__COMPILER__FEATURE__NOT_CONCRETE
#endif

//////////////////////////////////
// doxygen documentation compiler

#if defined DIVERSALIS__COMPILER__DOXYGEN
	#define DIVERSALIS__COMPILER
	#define DIVERSALIS__COMPILER__FEATURE__NOT_CONCRETE
	#if !defined __cplusplus
		#define __cplusplus
	#endif

///////////////////////
// eclipse c++ indexer

#elif defined DIVERSALIS__COMPILER__ECLIPSE
	#define DIVERSALIS__COMPILER
	#define DIVERSALIS__COMPILER__FEATURE__NOT_CONCRETE
	#if !defined __cplusplus
		#define __cplusplus
	#endif

////////////////
// gnu compiler

#elif defined __GNUC__
	#define DIVERSALIS__COMPILER
	#define DIVERSALIS__COMPILER__GNU
	#if (!defined __GNUG__ || !defined __cplusplus) && !defined DIVERSALIS__COMPILER__RESOURCE
		#if defined __GNUG__ || defined __cplusplus
			#error "weird settings... we should have both __GNUG__ and __cplusplus"
		#else
			#error "please invoke gcc with the language option set to c++ (or invoke gcc via the g++ driver)"
		#endif
	#endif
	#define DIVERSALIS__COMPILER__VERSION__STRING __VERSION__
	#define DIVERSALIS__COMPILER__VERSION  (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
	#define DIVERSALIS__COMPILER__VERSION__MAJOR __GNUC__
	#define DIVERSALIS__COMPILER__VERSION__MINOR __GNUC_MINOR__
	#define DIVERSALIS__COMPILER__VERSION__PATCH __GNUC_PATCHLEVEL__
	#define DIVERSALIS__COMPILER__VERSION__ABI __GXX_ABI_VERSION

	#define DIVERSALIS__COMPILER__FEATURE__WARNING

	#define DIVERSALIS__COMPILER__FEATURE__ASSEMBLER
	#define DIVERSALIS__COMPILER__ASSEMBLER__ATT

	#define DIVERSALIS__COMPILER__FEATURE__XMM_INTRINSICS

	// check if version supports pre-compilation. gcc < 3.4 does not support pre-compilation.
	#if __GNUC__ > 3 || (__GNUC__ == 3 && __GNUC_MINOR__ >= 4)
		#define DIVERSALIS__COMPILER__FEATURE__PRE_COMPILATION
	#endif
	
//////////////////////
// borland's compiler

#elif defined __BORLAND__
	#define DIVERSALIS__COMPILER
	#define DIVERSALIS__COMPILER__BORLAND
	#define DIVERSALIS__COMPILER__FEATURE__PRE_COMPILATION
	#define DIVERSALIS__COMPILER__FEATURE__AUTO_LINK

////////////////////////
// microsoft's compiler

#elif defined _MSC_VER
	#define DIVERSALIS__COMPILER
	#define DIVERSALIS__COMPILER__MICROSOFT
	
	#define DIVERSALIS__COMPILER__VERSION _MSC_VER
	#define DIVERSALIS__COMPILER__VERSION__MAJOR (_MSC_VER / 100)
	#define DIVERSALIS__COMPILER__VERSION__MINOR ((_MSC_VER - _MSC_VER / 100 * 100) / 10)
	#define DIVERSALIS__COMPILER__VERSION__PATCH (_MSC_VER - _MSC_VER / 10 * 10)
	#if defined _MSC_FULL_VER
		#define DIVERSALIS__COMPILER__VERSION__FULL _MSC_FULL_VER
	#endif

	#pragma conform(forScope, on) // ISO conformance of the scope of variables declared inside the parenthesis of a loop instruction.
	#define DIVERSALIS__COMPILER__FEATURE__PRE_COMPILATION
	#define DIVERSALIS__COMPILER__FEATURE__AUTO_LINK
	#define DIVERSALIS__COMPILER__FEATURE__ASSEMBLER
	#define DIVERSALIS__COMPILER__ASSEMBLER__INTEL
	#define DIVERSALIS__COMPILER__FEATURE__XMM_INTRINSICS
	
	////////////////////
	// intel's compiler
	// they also define _MSC_VER !
	
	#if defined __INTEL_COMPILER
		#define DIVERSALIS__COMPILER__INTEL
	#endif
#endif



/**********************************************************************************/
// consistency check



#if !defined DIVERSALIS__COMPILER && !defined DIVERSALIS__COMPILER__RESOURCE
	#error "Unkown compiler."
#endif
	
#if !defined __cplusplus && !defined DIVERSALIS__COMPILER__RESOURCE
	#error "Not a c++ compiler. For the gnu compiler, please invoke gcc with the language option set to c++ (or invoke gcc via the g++ driver)."
#endif

#endif // !defined DIVERSALIS__COMPILER__INCLUDED
