///\file
///\brief project-wide compiler-specific tweaks.
///\meta generic
#pragma once
#include INCLUDE(PROJECT/project.hpp)

#undef COMPILER

#if defined COMPILER__DOXYGEN
	#undef COMPILER__DOXYGEN // redefine to insert documentation.
	/// doxygen documentation compiler, manually defined in configuration file.
	/// version 1.3.8
	#define COMPILER__DOXYGEN
	/// compiler version, major number.
	#define COMPILER__VERSION__MAJOR 1
	/// compiler version, minor number.
	#define COMPILER__VERSION__MINOR 4
	/// compiler version, patch number.
	#define COMPILER__VERSION__PATCH 0

	/// resource compiler (only relevant on microsoft's operating system), autodetected via RC_INVOKED.
	#define COMPILER__RESOURCE
	#undef COMPILER__RESOURCE // was just defined to insert documentation.

	/// eclipse cdt c++ indexer, manually defined in project settings.
	/// version 2.0.2
	#define COMPILER__ECLIPSE
	#undef COMPILER__ECLIPSE // was just defined to insert documentation.

	/// a compiler feature description, set for compilers which don't generate code for a processor
	/// (e.g. the eclipse cdt c++ indexer, the doxygen documentation compiler, resource compilers, etc ...).
	/// [bohan] it was originally #define COMPILER__FEATURE__NO_CODE_GENERATION, but msvc7.1's resource compiler freaked out:\n
	/// [bohan] warning RC4011: identifier truncated to 'COMPILER__FEATURE__NO_CODE_GENE'
	#define COMPILER__FEATURE__NOT_CONCRETE
	#undef COMPILER__FEATURE__NOT_CONCRETE // was just defined to insert documentation.
	
	/// a compiler feature description, set for compilers which support pre-compilation.
	#define COMPILER__FEATURE__PRE_COMPILATION
	#undef COMPILER__FEATURE__PRE_COMPILTION // was just defined to insert documentation.
	
	/// gnu g++/gcc, g++, autodetected via __GNUG__ (equivalent to __GNUC__ && __cplusplus).
	// http://gcc.gnu.org/onlinedocs/gcc
	/// to see the predefined macros, run:
	///\verbatim
	/// g++ -xc++ -std=c++98 -dM -E /dev/null
	///\endverbatim
	/// version autodetected.
	#define COMPILER__GNU
	#undef COMPILER__GNU // was just defined to insert documentation.

	/// bcc (c++builder or zero-cost version).
	/// version 5.6
	#define COMPILER__BORLAND
	#undef COMPILER__BORLAND // was just defined to insert documentation.

	/// msvc, autodetected via _MSC_VER.
	/// msvc 6.? detected via _MSC_VER < 1300, we don't not test for service pack 5 nor processor pack.\n
	/// msvc 7.0 detected via _MSC_VER < 1310.\n
	/// msvc 7.1 detected via _MSC_VER >= 1310.\n
	/// msvc 8.0 detected via _MSC_VER todo.
	///
	/// command line options: http://msdn.microsoft.com/library/default.asp?url=/library/en-us/vccore/html/vcrefCompilerOptionsListedAlphabetically.asp
	///
	/// #pragma: http://msdn.microsoft.com/library/default.asp?url=/library/en-us/vclang/html/_predir_pragma_directives.asp
	///
	/// exception handling: http://msdn.microsoft.com/library/default.asp?url=/library/en-us/vclang/html/_core_exception_handling_topics_.28.c.2b2b29.asp
	#define COMPILER__MICROSOFT
	#undef COMPILER__MICROSOFT // was just defined to insert documentation.
#endif

// RC_INVOKED is defined by resource compilers (only relevant on microsoft's operating system).
#if defined RC_INVOKED
	//#define COMPILER
		// don't #define COMPILER ; we can determine the real preprocessor used for parsing.
	#define COMPILER__RESOURCE
	#define COMPILER__FEATURE__NOT_CONCRETE
#endif // COMPILER is not #defined

#if defined COMPILER__DOXYGEN
	#define COMPILER
	#define COMPILER__FEATURE__NOT_CONCRETE
	// version is defined above.
	#if !defined __cplusplus
		#define __cplusplus
	#endif
#elif defined COMPILER__ECLIPSE
	#define COMPILER
	#define COMPILER__FEATURE__NOT_CONCRETE
	#define COMPILER__VERSION__MAJOR 2
	#define COMPILER__VERSION__MINOR 1
	#define COMPILER__VERSION__PATCH 0
	#if !defined __cplusplus
		#define __cplusplus
	#endif
#elif defined __GNUC__
	#define COMPILER
	#define COMPILER__GNU
	#if (!defined __GNUG__ || !defined __cplusplus) && !defined COMPILER__RESOURCE
		#if defined __GNUG__ || defined __cplusplus
			#error "weird settings... we should have both __GNUG__ and __cplusplus"
		#else
			#error "please invoke gcc with the language option set to c++ (or invoke gcc via the g++ driver)"
		#endif
	#endif
	#define COMPILER__VERSION __VERSION__
	#define COMPILER__VERSION__MAJOR __GNUC__
	#define COMPILER__VERSION__MINOR __GNUC_MINOR__
	#define COMPILER__VERSION__PATCH __GNUC_PATCHLEVEL
	// check if version is recent enough__
	#if COMPILER__VERSION__MAJOR < 3 || (COMPILER__VERSION__MAJOR == 3 && COMPILER__VERSION__MINOR < 2)
		// [bohan] bah. that compiler is too old. no recent source will fit.
		#error "Compiler too old... better giving up now."
	#endif
	// check if version supports pre-compilation.
	// gcc < 3.4 does not support pre-compilation.
	#if COMPILER__VERSION__MAJOR > 3 || (COMPILER__VERSION__MAJOR == 3 && COMPILER__VERSION__MINOR >= 4)
		#define COMPILER__FEATURE__PRE_COMPILATION
	#endif
	#define COMPILER__GNU__ABI__VERSION __GXX_ABI_VERSION
#elif defined _MSC_VER
	#define COMPILER
	#define COMPILER__MICROSOFT
	#define COMPILER__FEATURE__PRE_COMPILATION
	#if _MSC_VER < 1300
		#define COMPILER__VERSION__MAJOR 6
		#define COMPILER__VERSION__MINOR 5 // service pack 5, we should detect this.
		#define COMPILER__VERSION__PATCH 1 // processor pack after service pack 5, we should detect this.
	#elif _MSC_VER < 1400
		#define COMPILER__VERSION__MAJOR 7
		#if _MSC_VER < 1310
			#define COMPILER__VERSION__MINOR 0
		#else
			#define COMPILER__VERSION__MINOR 1
		#endif
	#else
		#define COMPILER__VERSION__MAJOR 8
		#define COMPILER__VERSION__MINOR 0
		#define COMPILER__VERSION__MINOR 0
	#endif
	#if COMPILER__VERSION__MAJOR < 7 || (COMPILER__VERSION__MAJOR == 7 && COMPILER__VERSION__MINOR < 1)
		// [bohan] bah. that compiler is too old. no recent source will fit.
		#error "Ouch, shitty compiler... better giving up now."
	#endif
	#pragma conform(forScope, on) // ISO conformance of the scope of variables declared inside the parenthesis of a loop instruction.
#endif

#if defined _AFXDLL // mfc
		#if defined COMPILER__RESOURCE //&& defined COMPILER__MICROSOFT
			// msvc7.1's resource compiler freaks out: warning RC4011: identifier truncated to 'OPERATING_SYSTEM__VERSION__MICR'
		#else
			#if defined APPSTUDIO_INVOKED__THIS_MSVC_MFC_RESOURCE_EDITOR_IS_A_SHIT_AND_DOES_NOT_HANDLE_NAMESPACES_AND_WRONGLY_PARSES_PREPROCESSOR_DIRECTIVES
				#define NAMESPACE__BEGIN(x)
				#define NAMESPACE__END
			#else
				#define NAMESPACE__BEGIN(x) namespace x {
				#define NAMESPACE__END }
			#endif
		#endif
#endif

#if !defined COMPILER && !defined COMPILER__RESOURCE
	#error "Unkown compiler."
#endif
	
#if !defined __cplusplus && !defined COMPILER__RESOURCE
	#error "Not a c++ compiler. For the gnu compiler, please invoke gcc with the language option set to c++ (or invoke gcc via the g++ driver)."
#endif
