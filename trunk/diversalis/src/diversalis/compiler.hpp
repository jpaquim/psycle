// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2007 johan boule <bohan@jabber.org>
// copyright 2004-2007 psycledelics http://psycle.pastnotecut.org

///\file
///\brief project-wide compiler-specific tweaks.
///\meta generic
#ifndef DIVERSALIS__COMPILER__INCLUDED
#define DIVERSALIS__COMPILER__INCLUDED
#pragma once
#include <diversalis/detail/project.hpp>
//#region DIVERSALIS
	//#region COMPILER



		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// documentation about what is defined by this file



		#if defined DIVERSALIS__COMPILER__DOXYGEN
			#undef DIVERSALIS__COMPILER__DOXYGEN // redefine to insert documentation.
			/// doxygen documentation compiler, manually defined in configuration file.
			#define DIVERSALIS__COMPILER__DOXYGEN

			/// compiler version, major number.
			#define DIVERSALIS__COMPILER__VERSION__MAJOR 1
			/// compiler version, minor number.
			#define DIVERSALIS__COMPILER__VERSION__MINOR 4
			/// compiler version, patch number.
			#define DIVERSALIS__COMPILER__VERSION__PATCH 0
			/// compiler version, application binary interface number.
			#define DIVERSALIS__COMPILER__VERSION__ABI 0

			/// resource compiler (only relevant on microsoft's operating system), autodetected via RC_INVOKED.
			#define DIVERSALIS__COMPILER__RESOURCE
			#undef DIVERSALIS__COMPILER__RESOURCE // was just defined to insert documentation.

			/// eclipse cdt c++ indexer, manually defined in project settings.
			#define DIVERSALIS__COMPILER__ECLIPSE
			#undef DIVERSALIS__COMPILER__ECLIPSE // was just defined to insert documentation.

			/// a compiler feature description, set for compilers which don't generate code for a processor
			/// (e.g. the eclipse cdt c++ indexer, the doxygen documentation compiler, resource compilers, etc ...).
			/// [bohan] msvc7.1's resource compiler freaked out: warning RC4011: identifier truncated to 'DIVERSALIS__COMPILER__FEATURE__'
			#define DIVERSALIS__COMPILER__FEATURE__NOT_CONCRETE
			#undef DIVERSALIS__COMPILER__FEATURE__NOT_CONCRETE // was just defined to insert documentation.
			
			/// a compiler feature description, set for compilers which support pre-compilation.
			#define DIVERSALIS__COMPILER__FEATURE__PRE_COMPILATION
			#undef DIVERSALIS__COMPILER__FEATURE__PRE_COMPILTION // was just defined to insert documentation.
			
			/// a compiler feature description, set for compilers which support auto-link.
			#define DIVERSALIS__COMPILER__FEATURE__AUTO_LINK
			#undef DIVERSALIS__COMPILER__FEATURE__AUTO_LINK // was just defined to insert documentation.

			/// gnu g++/gcc, g++, autodetected via __GNUG__ (equivalent to __GNUC__ && __cplusplus).
			// http://gcc.gnu.org/onlinedocs/gcc
			/// to see the predefined macros, run:
			///\verbatim
			/// g++ -xc++ -std=c++98 -dM -E /dev/null
			///\endverbatim
			/// version autodetected.
			#define DIVERSALIS__COMPILER__GNU
			#undef DIVERSALIS__COMPILER__GNU // was just defined to insert documentation.

			/// bcc (c++builder or zero-cost version).
			/// version 5.6
			#define DIVERSALIS__COMPILER__BORLAND
			#undef DIVERSALIS__COMPILER__BORLAND // was just defined to insert documentation.

			/// msvc, autodetected via _MSC_VER.\n
			/// msvc 6.? detected via _MSC_VER < 1300, we don't test for service pack 5 nor processor pack.\n
			/// msvc 7.0 detected via _MSC_VER < 1310.\n
			/// msvc 7.1 detected via _MSC_VER < 1400.\n
			/// msvc 8.0 detected via _MSC_VER >= 1400.
			///
			/// command line options: http://msdn.microsoft.com/library/default.asp?url=/library/en-us/vccore/html/vcrefcompilerOptionsListedAlphabetically.asp
			///
			/// attributes: http://msdn.microsoft.com/library/default.asp?url=/library/en-us/vclang/html/_pluslang_Extended_Attribute_Syntax.asp
			///
			/// pragma: http://msdn.microsoft.com/library/default.asp?url=/library/en-us/vclang/html/_predir_pragma_directives.asp
			///
			/// exception handling: http://msdn.microsoft.com/library/default.asp?url=/library/en-us/vclang/html/_core_exception_handling_topics_.28.c.2b2b29.asp
			#define DIVERSALIS__COMPILER__MICROSOFT
			#undef DIVERSALIS__COMPILER__MICROSOFT // was just defined to insert documentation.
		#endif



		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// now the real work



		///////////////////////////////////////////////////////////////////////
		// resource compilers (only relevant on microsoft's operating system).
		///////////////////////////////////////////////////////////////////////

		// RC_INVOKED is defined by resource compilers (only relevant on microsoft's operating system).
		#if defined RC_INVOKED
			//#define DIVERSALIS__COMPILER
				// don't #define DIVERSALIS__COMPILER ; we can determine the real preprocessor used for parsing.
			#define DIVERSALIS__COMPILER__RESOURCE
			#define DIVERSALIS__COMPILER__FEATURE__NOT_CONCRETE
		#endif // DIVERSALIS__COMPILER is not #defined

		//////////////////////////////////
		// doxygen documentation compiler
		//////////////////////////////////

		#if defined DIVERSALIS__COMPILER__DOXYGEN
			#define DIVERSALIS__COMPILER
			#define DIVERSALIS__COMPILER__FEATURE__NOT_CONCRETE
			// version is defined above.
			#if !defined __cplusplus
				#define __cplusplus
			#endif

		///////////////////////
		// eclipse c++ indexer
		///////////////////////

		#elif defined DIVERSALIS__COMPILER__ECLIPSE
			#define DIVERSALIS__COMPILER
			#define DIVERSALIS__COMPILER__FEATURE__NOT_CONCRETE
			#define DIVERSALIS__COMPILER__VERSION__MAJOR 2 // version of the cdt
			#define DIVERSALIS__COMPILER__VERSION__MINOR 1
			#define DIVERSALIS__COMPILER__VERSION__PATCH 0
			#if !defined __cplusplus
				#define __cplusplus
			#endif

		////////////////
		// gnu compiler
		////////////////

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
			#define DIVERSALIS__COMPILER__VERSION __VERSION__
			#define DIVERSALIS__COMPILER__VERSION__MAJOR __GNUC__
			#define DIVERSALIS__COMPILER__VERSION__MINOR __GNUC_MINOR__
			#define DIVERSALIS__COMPILER__VERSION__PATCH __GNUC_PATCHLEVEL__
			// check if version is recent enough__
			#if DIVERSALIS__COMPILER__VERSION__MAJOR < 3 || (DIVERSALIS__COMPILER__VERSION__MAJOR == 3 && DIVERSALIS__COMPILER__VERSION__MINOR < 3)
				#error "compiler too old... better giving up now."
			#endif
			// check if version supports pre-compilation.
			// gcc < 3.4 does not support pre-compilation.
			#if DIVERSALIS__COMPILER__VERSION__MAJOR > 3 || (DIVERSALIS__COMPILER__VERSION__MAJOR == 3 && DIVERSALIS__COMPILER__VERSION__MINOR >= 4)
				#define DIVERSALIS__COMPILER__FEATURE__PRE_COMPILATION
			#endif
			#define DIVERSALIS__COMPILER__VERSION__ABI __GXX_ABI_VERSION

		/////////////////////
		// borland's compiler
		/////////////////////

		#elif defined __BORLAND__
			#define DIVERSALIS__COMPILER
			#define DIVERSALIS__COMPILER__BORLAND
			#define DIVERSALIS__COMPILER__FEATURE__PRE_COMPILATION
			#define DIVERSALIS__COMPILER__FEATURE__AUTO_LINK
			#define DIVERSALIS__COMPILER__VERSION__MAJOR 5
			#define DIVERSALIS__COMPILER__VERSION__MINOR 6
			#define DIVERSALIS__COMPILER__VERSION__PATCH 0

		////////////////////////
		// microsoft's compiler
		////////////////////////

		// TODO:
		// These things are mostly bogus. 1200 means version 12.0 of the compiler. The 
		// artificial versions assigned to them only refer to the versions of some IDE
		// these compilers have been shipped with, and even that is not all of it. Some
		// were shipped with freely downloadable SDKs, others as crosscompilers in eVC.
		// IOW, you can't use these 'versions' in any sensible way. Sorry.
		#elif defined _MSC_VER
			#define DIVERSALIS__COMPILER
			#define DIVERSALIS__COMPILER__MICROSOFT
			#if defined _MSC_FULL_VER
				#define DIVERSALIS__COMPILER__VERSION _MSC_FULL_VER
			#else
				#define DIVERSALIS__COMPILER__VERSION _MSC_VER
			#endif
			#define DIVERSALIS__COMPILER__FEATURE__PRE_COMPILATION
			#define DIVERSALIS__COMPILER__FEATURE__AUTO_LINK
			#if _MSC_VER < 1300
				#define DIVERSALIS__COMPILER__VERSION__MAJOR 0 // just purge it
			#elif _MSC_VER < 1400
				#define DIVERSALIS__COMPILER__VERSION__MAJOR 7
				#if _MSC_VER < 1310
					#define DIVERSALIS__COMPILER__VERSION__MINOR 0
				#else
					#define DIVERSALIS__COMPILER__VERSION__MINOR 1
				#endif
			#else
				#define DIVERSALIS__COMPILER__VERSION__MAJOR 8
				#define DIVERSALIS__COMPILER__VERSION__MINOR 0
				#define DIVERSALIS__COMPILER__VERSION__PATCH 0
			#endif
			#if DIVERSALIS__COMPILER__VERSION__MAJOR < 7 || (DIVERSALIS__COMPILER__VERSION__MAJOR == 7 && DIVERSALIS__COMPILER__VERSION__MINOR < 1)
				#error "Compiler is too old ... better giving up now."
			#elif DIVERSALIS__COMPILER__VERSION__MAJOR < 8
				//#pragma message(__FILE__ "(" DIVERSALIS__STRINGIZED(__LINE__) ") : warning: compiler is too old ... some problems are to be expected.")
			#endif
			#pragma conform(forScope, on) // ISO conformance of the scope of variables declared inside the parenthesis of a loop instruction.
		#endif



		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// consistency check



		#if !defined DIVERSALIS__COMPILER && !defined DIVERSALIS__COMPILER__RESOURCE
			#error "Unkown compiler."
		#endif
			
		#if !defined __cplusplus && !defined DIVERSALIS__COMPILER__RESOURCE
			#error "Not a c++ compiler. For the gnu compiler, please invoke gcc with the language option set to c++ (or invoke gcc via the g++ driver)."
		#endif



	//#endregion
//#endregion
#endif
