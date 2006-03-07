///\file
///\brief project-wide compiler, operating system, and processor specific tweaks.
///\meta generic
#pragma once
#include <psycle/host/detail/project.hpp>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



// following is old legacy stuff that's superseeded by universalis



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



#if !defined COMPILER || !defined OPERATING_SYSTEM || !defined PROCESSOR



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// compilers



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
	#elif _MSC_VER < 1310
		#define COMPILER__VERSION__MAJOR 7
		#define COMPILER__VERSION__MINOR 0
	#elif _MSC_VER < 1400
		#define COMPILER__VERSION__MAJOR 7
		#define COMPILER__VERSION__MINOR 1
	#else
		#define COMPILER__VERSION__MAJOR 8
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



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// operating systems



#if defined __linux__
	#define OPERATING_SYSTEM
	#define OPERATING_SYSTEM__LINUX
	#define OPERATING_SYSTEM__VERSION__MAJOR 2
	#define OPERATING_SYSTEM__VERSION__MINOR 6
	#define OPERATING_SYSTEM__VERSION__PATCH 8
#elif defined __APPLE__ && defined __MACH__
	#define OPERATING_SYSTEM
	#define OPERATING_SYSTEM__MACH
	#define OPERATING_SYSTEM__DARWIN
	#define OPERATING_SYSTEM__VERSION__MAJOR 7
	#define OPERATING_SYSTEM__VERSION__MINOR 7
	#define OPERATING_SYSTEM__VERSION__PATCH 0
	#define OPERATING_SYSTEM__APPLE
	#define OPERATING_SYSTEM__VERSION__EXTRA_LAYER__MAJOR 10
	#define OPERATING_SYSTEM__VERSION__EXTRA_LAYER__MINOR 3
	#define OPERATING_SYSTEM__VERSION__EXTRA_LAYER__PATCH 3
	#if defined __STRICT_BSD__
		#define OPERATING_SYSTEM__BSD
	#endif
#elif defined __MINGW32__ || defined _WIN32 || defined _WIN64
	#if defined _WIN64
		#error "These sources have never been tested on the 64-bit version of microsoft's operating system ; nevertheless, you may edit the file where this error is triggered to force compilation and test if it works."
	#else
		// [bohan] i don't know what to do with old msdos-based versions (9x/me)... are there still many users of these?
		#define OPERATING_SYSTEM
		#define OPERATING_SYSTEM__MICROSOFT
		#define OPERATING_SYSTEM__VERSION__MAJOR 5 // >= 2k
		#define OPERATING_SYSTEM__VERSION__MINOR 0 // == 2k
		//#define OPERATING_SYSTEM__VERSION__MINOR 1 // == xp
		#define OPERATING_SYSTEM__VERSION__PATCH 2600 // xp service pack 1
		#if defined COMPILER__RESOURCE //&& defined COMPILER__MICROSOFT
			// msvc7.1's resource compiler freaks out: warning RC4011: identifier truncated to 'OPERATING_SYSTEM__VERSION__MICR'
		#else
			#define OPERATING_SYSTEM__VERSION__MICROSOFT__COMPATIBILITY__XP 0x501
			#define OPERATING_SYSTEM__VERSION__MICROSOFT__COMPATIBILITY__2000 0x500
			#define OPERATING_SYSTEM__VERSION__MICROSOFT__COMPATIBILITY__1998 0x410
			#if defined OPERATING_SYSTEM__VERSION__MICROSOFT__COMPATIBILITY
				#define WINVER OPERATING_SYSTEM__VERSION__MICROSOFT__COMPATIBILITY
				#define _WIN32_WINNT WINVER
				#define _WIN32_WINDOWS WINVER
				#define _WIN32_IE WINVER
			#endif
		#endif
		/// microsoft's #define MAX_PATH has too low value for ntfs
		#define OPERATING_SYSTEM__MICROSOFT__MAX_PATH (MAX_PATH < (1 << 12) ? (1 << 12) : MAX_PATH)
	#endif
#else
	/// We don't know the exact operating system, but we'll try to compile anyway, using crossplatform libraries.
	#define OPERATING_SYSTEM
	#define OPERATING_SYSTEM__CROSSPLATFORM
#endif

#if !defined OPERATING_SYSTEM
	#error "Unkown operating system."
#endif

// operating system kernels for gnu operating system applications
#if defined OPERATING_SYSTEM__HURD || defined OPERATING_SYSTEM__LINUX
	#define OPERATING_SYSTEM__GNU
#endif

// operating systems following the posix standard
#if defined OPERATING_SYSTEM__GNU || defined OPERATING_SYSTEM__BSD
	#define OPERATING_SYSTEM__POSIX
#endif

#if defined OPERATING_SYSTEM__CROSSPLATFORM || !(defined OPERATING_SYSTEM__MICROSOFT || defined OPERATING_SYSTEM__APPLE)
	#define OPERATING_SYSTEM__X_WINDOW
#endif



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// processors



#if defined COMPILER__GNU
	//#if defined _X86_
		#if defined __i686__
			#define PROCESSOR
			#define PROCESSOR__X86 6
		#elif defined __i586__
			#define PROCESSOR
			#define PROCESSOR__X86 5
		#elif defined __i486__
			#define PROCESSOR
			#define PROCESSOR__X86 4
		#elif defined __i386__
			#define PROCESSOR
			#define PROCESSOR__X86 3
		#endif
	//#endif
#elif defined COMPILER__MICROSOFT
	#if defined _M_IA64
		#define PROCESSOR
		#define PROCESSOR__IA 2
	#elif defined _M_IX86
		#define PROCESSOR
		#define PROCESSOR__X86 _M_IX86 / 100
	#elif defined OPERATING_SYSTEM__MICROSOFT // well, it's obvious anyway , it's COMPILER__MICROSOFT
		#if OPERATING_SYSTEM__VERSION__MAJOR > 5 || (OPERATING_SYSTEM__VERSION__MAJOR == 5 && OPERATING_SYSTEM__VERSION__MINOR == 0 && OPERATING_SYSTEM__VERSION__PATCH > 3)
			#error "microsoft, after v5.0.3 (2ksp3), gave up with all processors but x86 and ia... no more hp(compaq(digital-e-c)) alpha axp."
		#else
			#define PROCESSOR
			#define PROCESSOR__ALPHA_AXP 7 // ev7 (213?64)
		#endif
	#endif
#endif

#if defined OPERATING_SYSTEM__APPLE
	#define PROCESSOR
	#define PROCESSOR__POWER_PC 5
	// Implied because the version of apple's operating system for 68k processors has been discontinued.
	// Hence now, only power pc processors are supported by this operating system.
	// [bohan] Actually it seems that darwin also runs on sparc and hppa processors.
#endif

#if !defined PROCESSOR && !defined COMPILER__FEATURE__NOT_CONCRETE
	#error "Unkown processor."
#endif

// minimum requirements for processors
#if defined PROCESSOR__X86 && PROCESSOR__X86 < 3
	#error "Compiler's target processor too old."
#endif

// processor endianess
#if defined CONFIGURATION__PSYCLE
	#if defined CONFIGURATION__PSYCLE__WORDS_BIGENDIAN
		#define PROCESSOR__ENDIAN
		#define PROCESSOR__ENDIAN__BIG
	#else
		#define PROCESSOR__ENDIAN
		#define PROCESSOR__ENDIAN__LITTLE
	#endif
#elif defined PROCESSOR__POWER_PC
	#define PROCESSOR__ENDIAN
	#define PROCESSOR__ENDIAN__BOTH
#elif defined PROCESSOR__X86 || defined PROCESSOR__IA
	#define PROCESSOR__ENDIAN
	#define PROCESSOR__ENDIAN__LITTLE
#endif

#if defined PROCESSOR__POWER_PC && !defined PROCESSOR__ENDIAN__BOTH
	// Since autoconf uses some real tests to determine things, it just will find the endianess that was effective during the test, but won't know it can be changed.
	#define PROCESSOR__ENDIAN__BOTH
#endif

#if !defined PROCESSOR__ENDIAN && !defined COMPILER__FEATURE__NOT_CONCRETE
	#error "Unkown byte sex."
#endif



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///\name stringization
///\{



#if !defined STRINGIZED
	/// Interprets argument as a string litteral.
	/// The indirection in the call to # lets the macro expansion on the argument be done first.
	#define STRINGIZED(tokens) STRINGIZED__NO_EXPANSION(tokens)
	/// Don't call this macro directly ; call STRINGIZED, which calls this macro after macro expansion is done on the argument.
	///\relates STRINGIZED
	#define STRINGIZED__NO_EXPANSION(tokens) #tokens
#endif



///\}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///\name end of lines
///\{



#if !defined EOL
	#if defined OPERATING_SYSTEM__MICROSOFT
		#define EOL "\r\n"
	#elif defined OPERATING_SYSTEM__APPLE && OPERATING_SYSTEM__VERSION__MAJOR < 10 // before bsd-unix (darwin)
		#define EOL "\n\r"
	#else
		#define EOL "\n"
	#endif
#endif



///\}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// If this is a resource compiler, skips the rest of this file (only relevant on microsoft's operating system).
#if !defined COMPILER__RESOURCE
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



///\}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif // !defined COMPILER__RESOURCE (only relevant on microsoft's operating system).
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// end
#endif
