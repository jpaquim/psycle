// This file is an exact copy of the following file from freepsycle:
// http://bohan.dyndns.org/cgi-bin/archzoom.cgi/psycle@sourceforge.net/psycle--mainline--LATEST--LATEST/src/project.hpp.in
///\file
///\brief project-wide compiler, operating system, and processor specific tweaks.
///\meta generic
#pragma once



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// public configuration header



// includes the public configuration header.
#include "configuration.hpp"



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



#if !defined COMPILER || !defined OPERATING_SYSTEM || !defined PROCESSOR



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// doxygen documentation compiler



#if defined COMPILER__DOXYGEN
///\name compilers
///\{
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
	/// msvc 6.? detected via _MSC_VER < 1300, we don't not test for service pack 5 nor processor pack.
	/// msvc 7.0 detected via _MSC_VER < 1310.
	/// msvc 7.1 detected via _MSC_VER >= 1310.
	/// msvc 8.0 detected via _MSC_VER todo.	
	#define COMPILER__MICROSOFT
	#undef COMPILER__MICROSOFT // was just defined to insert documentation.
///\}
///\name operating systems
///\{
	/// mach kernel.
	#define OPERATING_SYSTEM__MACH
	#undef OPERATING_SYSTEM__MACH // was just defined to insert documentation.   

	/// hurd kernel, \see OPERATING_SYSTEM__MACH.
	#define OPERATING_SYSTEM__HURD
	#undef OPERATING_SYSTEM__HURD // was just defined to insert documentation.
	
	/// linux kernel, autodetected via __linux__.
	/// version 2.6.7
	#define OPERATING_SYSTEM__LINUX
	#undef OPERATING_SYSTEM__LINUX // was just defined to insert documentation.

	/// bsd kernel, autodetected for apple's darwin/macos mach/bsd kernel via __STRICT_BSD__, \see OPERATING_SYSTEM__DARWIN.
	#define OPERATING_SYSTEM__BSD
	#undef OPERATING_SYSTEM__BSD // was just defined to insert documentation.

	/// darwin/mach kernel, \see OPERATING_SYSTEM__MACH and \see OPERATING_SYSTEM__BSD, and for apple's darwin/macos \see OPERATING_SYSTEM__APPLE.
	#define OPERATING_SYSTEM__DARWIN
	#undef OPERATING_SYSTEM__DARWIN // was just defined to insert documentation.   

	/// apple's macosx darwin mach/bsd kernel, autodetected via __APPLE__ and __MACH__, \see OPERATING_SYSTEM__DARWIN.
	/// version 10
	#define OPERATING_SYSTEM__APPLE
	#undef OPERATING_SYSTEM__APPLE // was just defined to insert documentation.

	/// microsoft's windows, autodetected via _WIN32 and _WIN64 (and __MINGW32__).
	/// version 5.1.2600 (xp service pack 1).
	/// [bohan]
	/// i don't know what to do with old versions (9x/me)...
	/// it would be cubersome to support such old systems.
	/// are there still many users of these?
	#define OPERATING_SYSTEM__MICROSOFT
	#undef OPERATING_SYSTEM__MICROSOFT // was just defined to insert documentation.

	/// mswind version compatibility selection.\n
	/// can be set to one of:\n
	/// OPERATING_SYSTEM__VERSION__MICROSOFT__COMPATIBILITY__XP\n
	/// OPERATING_SYSTEM__VERSION__MICROSOFT__COMPATIBILITY__2000\n
	/// OPERATING_SYSTEM__VERSION__MICROSOFT__COMPATIBILITY__1998\n
	#define OPERATING_SYSTEM__MICROSOFT__COMPATIBILITY
	#undef OPERATING_SYSTEM__MICROSOFT__COMPATIBILITY // was just defined to insert documentation.

	/// operating system version, major number.
	#define OPERATING_SYSTEM__VERSION__MAJOR
	#undef OPERATING_SYSTEM__VERSION__MAJOR // was just defined to insert documentation.
	/// operating system version, minor number.
	#define OPERATING_SYSTEM__VERSION__MINOR
	#undef OPERATING_SYSTEM__VERSION__MINOR // was just defined to insert documentation.
	/// operating system version, patch number.
	#define OPERATING_SYSTEM__VERSION__PATCH
	#undef OPERATING_SYSTEM__VERSION__PATCH // was just defined to insert documentation.

	/// If defined, prefers to use crossplatforms libraries when choice is available.
	/// For examples, we may use:\n
	/// - glib or boost instead of the operating system proprietary/specific api directly.\n
	/// - the x window system on apple's macosx and cygwin instead of the operating system proprietary/specific gui api directly.\n
	#define OPERATING_SYSTEM__CROSSPLATFORM
	#undef OPERATING_SYSTEM__CROSSPLATFORM // was just defined to insert documentation.

	/// the posix standard, autodetermined.
	/// version 2
	#define OPERATING_SYSTEM__POSIX
	#undef OPERATING_SYSTEM__POSIX // was just defined to insert documentation.
	
	/// gnu operating system, autodetermined.
	#define OPERATING_SYSTEM__GNU
	#undef OPERATING_SYSTEM__GNU // was just defined to insert documentation.

	/// X Window System.
	/// On apple's macosx operating system,
	/// and on cygwin,
	/// we have to choose whether we want to use the x window system or not.
	/// This is done by testing if OPERATING_SYSTEM__CROSSPLATFORM is defined.
	#define OPERATING_SYSTEM__X_WINDOW
	#undef OPERATING_SYSTEM__X_WINDOW // was just defined to insert documentation.
///\}
///\name processors
///\{
		/// big endian byte order.
		#define PROCESSOR__ENDIAN__BIG
		#undef PROCESSOR__ENDIAN__BIG // was just defined to insert documentation.
	
		/// little endian byte order.
		#define PROCESSOR__ENDIAN__LITTLE
		#undef PROCESSOR__ENDIAN__LITTLE // was just defined to insert documentation.
	
		/// both endian byte order.
		#define PROCESSOR__ENDIAN__BOTH
		#undef PROCESSOR__ENDIAN__BOTH // was just defined to insert documentation.

		/// power-pc instruction set (ibm/motorola).\n
		/// 5 == g5 (970).\n
		/// 4 == g4 (7400/7410).\n
		/// 3 == g3 (750).\n
		/// 2 == 604/604e.\n
		/// 1 == 603/603e.\n
		/// 0 == 602.\n
		/// -1 == 601.\n
		#define PROCESSOR__POWER_PC 5
		#undef PROCESSOR__POWER_PC // was just defined to insert documentation.

		/// arm instruction set (intel(digital-e-c) / element-14 acorn/advanced risc machine, and others).\n
		/// 10 == 1110 x-scale (intel).\n
		/// 9 == 110 strong-arm (digital-e-c).\n
		/// 8 == 100 strong-arm (digital-e-c).\n
		/// 7 == arm7 (advanced risc machine (acorn & apple)).\n
		/// 6 == arm6 (advanced risc machine (acorn & apple)).\n
		/// 2 == arm2 (acorn risc machine).\n
		/// 1 == arm1 (acorn risc machine).\n
		/// -1 == 6502 (commodore(mos technology)).\n
		/// -2 == 6800 (motorola).\n
		#define PROCESSOR__ARM 10
		#undef PROCESSOR__ARM // was just defined to insert documentation.

		/// mips instruction set (mips technologies incorporation).\n
		/// (MIPS I, MIPS II, MIPS III, MIPS IV, MIPS 32/64 + MIPS16 + MDMX + MIPS MT)\n
		/// 4 == mips-iv\n
		/// 3 == mips-iii\n
		/// 2 == mips-ii\n
		/// 1 == mips-i\n
		/// 16000 == R16000 (mips).\n
		/// 14000 == R14000 (sgi).\n
		/// 12000 == R12000 (sgi(mips)).\n
		/// 10000 == R10000 (sgi(mips)).\n
		/// 9000 == R9000 (pmc-sierra(qed)()sgi(mips)).\n
		/// 8000 == R8000 (sgi(mips)).\n
		/// 7100 == SR7100 (sand-craft()mips).\n
		/// 6000 == R6000ECL (pmc-sierra(bit()fps()intel()tektronix)).\n
		/// 5432 == R5432 (sand-craft()mips/nec).\n
		/// 5000 == R5000 (pmc-sierra(qed)()sgi(mips)).\n
		/// 4700 == R4700 (pmc-sierra(qed)()sgi(mips)).\n
		/// 4650 == R4650 (pmc-sierra(qed)()sgi(mips)).\n
		/// 4600 == R4600 (pmc-sierra(qed)()sgi(mips)).\n
		/// 4400 == R4400 (sgi(mips)).\n
		/// 4200 == R4200 (excluding the bridled R4300) (sgi(mips)).\n
		/// 4000 == R4000 (mips).\n
		/// 3010 == R3000+R3010 (mips).\n
		/// 2010 == R2000+R2010 (mips).\n
		/// 2000 == R2000 (mips).\n
		/// 1000 == R1000 (stanford).\n
		///      == (lexra).\n
		///      == Au1000 (broadcom(alchemy()dec/mips)).\n
		///      == SB1250 (amd(sibyte()dec)/mips).\n
		#define PROCESSOR__MIPS 10
		#undef PROCESSOR__MIPS // was just defined to insert documentation.

		/// sparc instruction set (gnu/sparc/sun(cray(fps))/texas-instruments/cypress-semiconductors/fujitsu).\n
		/// 9 == \n
		/// 8 == \n
		/// 5 == B5000ECL (pmc-sierra(bit()fps()intel()tektronix)/sun).\n
		///   == ultra-sparc-iii (sun).\n
		///   == ultra-sparc-ii (sun).\n
		///   == ultra-sparc-i (sun).\n
		///   == super-sparc (sun).\n
		/// 1 == sparc-1 (lis/weitek/sun).\n
		/// 0 == risc-ii (berkeley).\n
		/// -1 == risc-i (berkeley).\n
		#define PROCESSOR__SPARC 9
		#undef PROCESSOR__SPARC // was just defined to insert documentation.

		/// ia instruction set (hewlett-packard/(sgi)/intel architecture).\n
		/// 2 == itanium2.\n
		/// 1 == itanium1.\n
		/// (side-note: HP plans to retire PA and Alpha RISC CPUs to concentrate on itanium development.)\n
		#define PROCESSOR__IA 2
		#undef PROCESSOR__IA // was just defined to insert documentation.

		/// x86 instruction set.\n
		/// 8 == amd64, intel emt64 "nocona".\n
		/// 7 == sse2, from late duron, atlhon-xp, intel pentium-iv.\n
		/// 6 == sse1, from pentium-pro to pentium-iii, early duron.\n
		/// 5 == i80586 aka "pentium"-i.\n
		/// 4 == i80486+487.\n
		/// 3 == i80386dx+387 (excluding the bridled i386sx).\n
		/// 2 == i80286.\n
		/// 1 == i8086 (excluding the bridled i8088).\n
		/// 0 == i8080/i8085.\n
		#define PROCESSOR__X86 7
		#undef PROCESSOR__X86 // was just defined to insert documentation.
///\}
#endif



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
	#if _MSC_VER < 1300 // msvc 6
		#define COMPILER__VERSION__MAJOR 6
		#define COMPILER__VERSION__MINOR 5 // service pack 5, we should detect this.
		#define COMPILER__VERSION__PATCH 1 // processor pack after service pack 5, we should detect this.
	#else
		#define COMPILER__VERSION__MAJOR 7
		#if _MSC_VER < 1310 // msvc 7.0
			#define COMPILER__VERSION__MINOR 0
		#else // msvc 7.1
			#define COMPILER__VERSION__MINOR 1
		#endif
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
///\name debug
///\{



#if defined COMPILER__MICROSOFT
	// microsoft uses its own vendor #define _DEBUG (for debug), instead of the iso #define NDEBUG (for no debug)
	// so, we recover the iso way
	#if !defined NDEBUG
		#if !defined _DEBUG
			// no debug
			#define NDEBUG
			#pragma message("no debug")
		#else
			#pragma message("debug")
		#endif
	#elif defined _DEBUG
		#error "both ISO #define NDEBUG (no debug) and microsoft's vendor #define _DEBUG (debug)... this is non sensical"
	#endif
#elif defined COMPILER__DOXYGEN
	/// parts of the source code that contain instructions only useful for debugging purpose
	/// shall be encapsulated inside:
	///\code
	/// #if !defined NDEBUG
	/// 	some debugging stuff
	/// #endif
	///\endcode
	#define NDEBUG
#endif


///\}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



// value to show in the string describing the configuration options.
#if defined NDEBUG
	#define PSYCLE__CONFIGURATION__OPTION__ENABLE__DEBUG "off"
#else
	#define PSYCLE__CONFIGURATION__OPTION__ENABLE__DEBUG "on"
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



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// warnings



#if defined COMPILER__MICROSOFT
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
	#pragma warning(disable:4290) // c++ exception specification not yet implemented except to indicate a function is not __declspec(nothrow)
		// "A function is declared using exception specification, which Visual C++ accepts but does not implement.
		// Code with exception specifications that are ignored during compilation may need to be
		// recompiled and linked to be reused in future versions supporting exception specifications."
	#if COMPILER__VERSION__MAJOR < 7
		#pragma warning(disable:4786) // identifier was truncated to '255' characters in the debug information ; this is ultra shiatz.
	#endif

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
	#pragma warning(disable:4244) // conversion from 'int' to 'float', possible loss of data
		// x-bit int to x-bit float conversions are not lossless, but in many cases we do not care about it

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
		// beware with this ... it is disabled because of /D_WINDLL ... otherwize it could be relevant
#endif



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// optimizations



#if defined COMPILER__MICROSOFT
	#pragma pack(8)
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
		//#define inline __forceinline
			// [bohan] disabled, because it can make the compiler/linker eat awesome amounts of RAM and CPU time.
	#endif
#endif



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///\name extensions to the language which would become standard
///\{



// online documentation:
	// COMPILER__GNU
		// http://gcc.gnu.org/onlinedocs/gcc
	// COMPILER__MICROSOFT
		// command line options: http://msdn.microsoft.com/library/default.asp?url=/library/en-us/vccore/html/vcrefCompilerOptionsListedAlphabetically.asp
		// #pragma: http://msdn.microsoft.com/library/default.asp?url=/library/en-us/vclang/html/_predir_pragma_directives.asp
		// exception handling: http://msdn.microsoft.com/library/default.asp?url=/library/en-us/vclang/html/_core_exception_handling_topics_.28.c.2b2b29.asp



////////////////////////////////////
/// overridding of virtual function
////////////////////////////////////

#define override virtual
//#define override

/////////////////////////
/// pragmatic attributes
/////////////////////////

#if defined COMPILER__GNU
	#define pragma(x) __attribute__ ((x))
#elif defined COMPILER__MICROSOFT
	#define pragma(x) __declspec(x)
#else
	#define pragma(x)
#endif

/////////////////////////////////////
/// deprecated warning.
/// declares a symbol as deprecated.
/////////////////////////////////////

#if defined COMPILER__GNU
	#define deprecated pragma(deprecated)
#elif defined COMPILER__MICROSOFT
	// deprecated is a keyword, not an attribute?
#else
	#define deprecated
#endif

///////////////////////////////////////////////////
/// thread local storage
/// variable stored in a per thread local storage.
///////////////////////////////////////////////////

#if defined COMPILER__GNU
	#define thread_local_storage pragma(thread)
#elif defined COMPILER__MICROSOFT
	#define thread_local_storage __thread
#else
	#define thread_local_storage
#endif

////////////////////
// memory alignment
////////////////////

#if defined COMPILER__GNU
	#define aligned(x) pragma(aligned(x))
	//#define packed pragma(packed)
	// note: a bit field is packed to 1 bit, not one byte.
#elif defined COMPILER__MICROSOFT
	#define aligned(x) pragma(align(x))
	//#define packed pragma(packed)
	// see also: #pragma pack(x) in the optimization section
#else
	/// memory alignment.
	///\see packed
	#define aligned(x)
	/// memory alignment.
	///\see aligned()
	#define packed
#endif

// example: class riff_chunk packed aligned(1) { uint8[4] tag; uint32 size; uint8[0] data; }

///////
// asm
///////

#if defined COMPILER__GNU
	// asm is a keyword
#elif defined COMPILER__MICROSOFT
	#define asm __asm
#endif

/////////////
/// restrict
/////////////

/// this keyword has been introduced in the iso 1999 standard.
/// it is a hint for the compiler telling it several references or pointers will *always* holds different memory addresses.
/// hence it can optimize more, knowing that writting to memory via one reference cannot alias another.
/// example:
	/// void f(int & restrict r1, int & restrict r2, int * restrict p1, int restrict p2[]);
	/// here the compiler is told that &r1 != &r2 != p1 != p2

#if defined COMPILER__GNU
	// restrict is a keyword
	//#define restrict __restrict__
#elif defined COMPILER__MICROSOFT
	#define restrict
	// unsupported?
	// see also: #pragma optimize("a", on) // assumes no aliasing
#else
	#define restrict
#endif

//////////
/// assume
//////////

/// [bohan] i don't quite understand what microsoft says about this keyword...
#if defined COMPILER__GNU
	// unsupported?
	#define assume(x)
#elif defined COMPILER__MICROSOFT
	#define assume(x) __assume(x)
#else
	#define assume(x)
#endif

/////////
// super
/////////

// [bohan] this keyword is just syntactic sugar, so, we don't *require* it.
#if defined COMPILER__GNU
	// unsupported?
#elif defined COMPILER__MICROSOFT
	// super is a keyword
#else
	// prolly unsupported
#endif

//////////
// typeof
//////////

#if defined COMPILER__GNU
	// typeof is a standard keyword
	#define COMPILER__FEATURE__TYPEOF
#elif defined COMPILER__MICROSOFT
	/// it looks like a reserved keyword, but it's not implemented yet (only in c#).
	#define typeof __typeof
#endif

/////////////////////////////////
/// function calling conventions
/////////////////////////////////
// calling convention | modifier keyword | parameters stack push            | parameters stack pop | extern "C" symbol name mangling                   | extern "C++" symbol name mangling
// register           | __fastcall       | 3 registers then pushed on stack | callee               | '@' and arguments' byte size in decimal prepended | no standard
// pascal             | __pascall        | left to right                    | callee               | uppercase                                         | no standard
// standard call      | __stdcall        | right to left                    | callee               | preserved                                         | no standard
// c declaration      | __cdecl          | right to left, variable count    | caller               | '_' prepended                                     | no standard

// note: the register convention is different from one compiler to another (for example left to right for borland, right to left for microsoft).
// for borland's compatibility with microsoft's register calling convention: #define fastcall __msfastcall.
// note: on the gnu compiler, one can use the #define __USER_LABEL_PREFIX__ to know what character is prepended to extern "C" symbols.
// note: on microsoft's compiler, with cdecl, there's is no special decoration for extern "C" declarations, i.e., no '_' prepended.
/// member functions defaults to thiscall
/// non-member functions defaults to cdecl

#if defined COMPILER__FEATURE__NOT_CONCRETE
	#define cdecl
	#define stdcall
	#define pascall
	#define fastcall
	// for pseudo compilers, we just define all this to nothing to avoid errors in case the compiler would not know about them.
	#define __cdecl
	#define __stdcall
	#define __pascall
	#define __fastcall
	#define __msfastcall
#elif defined COMPILER__GNU
	#define cdecl pragma(__cdecl__)
	#define stdcall pragma(__stdcall__)
	#define pascall pragma(__pascall__)
	#define fastcall pragma(__fastcall__)
#elif defined COMPILER__BORLAND
	#define cdecl __cdecl
	#define stdcall __stdcall
	#define pascall __pascall
	/// borland's compatibility with microsoft's register calling convention
	#define fastcall __msfastcall
#elif defined COMPILER__MICROSOFT
	/// \todo [bohan] don't know why windef.h defines cdecl to nothing... because of this, functions with cdecl in their signature aren't considered to be of the same type as ones with __cdecl in their signature.
	#define cdecl
	#define stdcall __stdcall
	#define pascall __pascall
	#define fastcall __fastcall
#else
	#error "Unsupported compiler ; please add support for function calling conventions for your compiler in src/project.hpp(.in) where this error is triggered."
	//#elif defined COMPILER__<your_compiler_name>
		#define cdecl ...
		#define stdcall ...
		#define pascall ...
		#define fastcall ...
	//#endif
#endif

///////////
/// wchar_t
///////////

#if defined COMPILER__MICROSOFT && 0 // [bohan] disabled... i can't help microsoft with this... let them fix that flaw.
	// the microsoft compiler has a broken implementation of wchar_t: it is 16 bits while it actually must be 32 bits.
	// see also: _WCHAR_T_DEFINED _NATIVE_WCHAR_T_DEFINED
	#if defined wchar_t
		#undef wchar_t
		typedef compiler::uint32 __identifier(wchar_t);
	#else
		#define wchar_t compiler::uint32
		// we cannot use a typedef since wchar_t is already a type, and we cannot #undef a type.
	#endif
#endif

//////////////////////////
/// try-finally statements
//////////////////////////

#if 0 // disabled ; as of january 2005, we still can't use try-finally statements easily but with borland's compiler.
	#if defined COMPILER__FEATURE__NOT_CONCRETE
		#define try_finally
		#define finally
	#elif defined COMPILER__GNU
		#define try_finally
		#define finally
	#elif defined COMPILER__BORLAND
		#define try_finally try
		#define finally __finally
	#elif defined COMPILER__MICROSOFT
		// msvc's try-finally statements are useless because they cannot be mixed with try-catch statements in the same function
		// [bohan] this was with msvc 6. it might have been fixed with msvc 7.1, but there's only little hope.
		//#define try_finally __try
		//#define finally __finally
	#else
		#error "Unsupported compiler ; please add support for try-finally statements for your compiler in src/project.hpp(.in) where this error is triggered."
		//#elif defined COMPILER__<your_compiler_name>
			#define try_finally ...
			#define finally ...
		//#endif
	#endif
#endif // 0

////////////////////////////////
/// hardware exception handling
////////////////////////////////

#if 0 // not needed ... the 'try' keyword let us do what we want
// we want to be able to catch hardware exceptions as typed c++ ones, not only via catch(...)
	#if defined COMPILER__FEATURE__NOT_CONCRETE
		#define try_hard try
	#elif defined COMPILER__GNU
		#define try_hard try
	#elif defined COMPILER__BORLAND
		#define try_hard try
	#elif defined COMPILER__MICROSOFT
		#define try_hard try
		// http://msdn.microsoft.com/library/default.asp?url=/library/en-us/vclang/html/_core_exception_handling_topics_.28.c.2b2b29.asp
		// compiler option EH<s|a>[c[-]]>, exception handling:
			// http://msdn.microsoft.com/library/default.asp?url=/library/en-us/vccore/html/_core_.2f.Zs.asp
			// This option specifies the model of exception handling to be used by the compiler.
			// * Use /EHs to specify the synchronous exception handling model
			//   (C++ exception handling without structured exception handling exceptions).
			//   If you use /EHs, do not rely on the compiler to catch asynchronous exceptions.
			// * Use /EHa to specify the asynchronous exception handling model
			//   (C++ exception handling with structured exception handling exceptions).
			// http://msdn.microsoft.com/library/default.asp?url=/library/en-us/vclang/html/vclrfExceptionSpecifications.asp
			// function exception specification                      EHsc==GX   EHs EHsc-  EHa EHac-  EHac
			// c function (no exception specification)               throw()    throw(...) throw(...) throw(...)
			// c++ function with no exception specification          throw(...) throw(...) throw(...) throw(...)
			// c++ function with throw() exception specification     throw()    throw()    throw(...) throw(...)
			// c++ function with throw(...) exception specification  throw(...) throw(...) throw(...) throw(...)
			// c++ function with throw(type) exception specification throw(...) throw(...) throw(...) throw(...)
			//
			// notes:
			//
			// both exception handling models, synchronous and asynchronous, are fully compatible and can be mixed in the same application.
			// with both models, the compiler allows to translate hardware exceptions into c++ ones.
			//
			// (in)correctness:
			//
			// when a *hardware* exception is raised,
			// with the synchronous model, some of the unwindable objects in the function where the exception occurs may not get unwound,
			// if the compiler judges their lifetime tracking mechanics to be unnecessary for the synchronous model.
			// this problem does not occur if the asynchronous model is used, but,
			//
			// efficiency:
			//
			// the synchronous model allows the compiler to eliminate the mechanics of tracking the lifetime of certain unwindable objects,
			// and to significantly reduce the code size, if the objects' lifetimes do not overlap a function call or a throw statement.
			//
			// [bohan] conclusion: the best mode is EHsc (==GX) even if we can have some memory leaks if hardware exceptions are raised
			// [bohan] note:
			// [bohan] there is a bug in msvc concerning synchronous exception handling model ... disable it locally where necessary
			// [bohan] problems appears when using synchronous exception handling model...
			// [bohan] on msvc 6, i had to use asynchronous exception handling
			// [bohan] this is still not fixed in msvc 7.1 (it's only fixed with global optimization).
			// [bohan] it hopefully generates a warning->error, otherwize, we would have a runtime bug
			// [bohan] warning C4702: unreachable code
			// [bohan] error C2220: warning treated as error - no object file generated
			// [bohan] as of 2003-12-11 i decided not to make the compile treat the warning as an error...
			// [bohan] yet, i don't know if the compiler actually puts the code in the binary or discard it badly :/
			//
			// [bohan] shit! warning C4535: calling _set_se_translator() requires /EHa; the command line options /EHc and /GX are insufficient
	#else
		#error "Unsupported compiler ; please add support for hardware exception handling for your compiler in src/project.hpp(.in) where this error is triggered."
		//#elif defined COMPILER__<your_compiler_name>
			#define try_hard ...
		//#endif
	#endif
#endif // 0

///////////////////////////////
// shared dynamic-link library
///////////////////////////////

#if !defined OPERATING_SYSTEM__MICROSOFT || defined COMPILER__GNU // gcc handles dll transparently :-)
	/// everything is nice and simple
	/// [bohan] but, see http://people.redhat.com/drepper/dsohowto.pdf (pdf sux ... is there a non pdf version?)
	///\todo /dev/null will fail on mingw outside of msys ... we need an empty file... or an already-included one... maybe #include __FILE__
	#define LIBRARY__BEGIN  "/dev/null"
	#define LIBRARY__EXPORT "/dev/null"
	#define LIBRARY__IMPORT "/dev/null"
	#define LIBRARY__END    "/dev/null"
#else
	#define EXPORT pragma(dllexport)
	#define IMPORT pragma(dllimport)
	#define LIBRARY__BEGIN  <operating_system/library/begin.hpp>
	#define LIBRARY__EXPORT <operating_system/library/export.hpp>
	#define LIBRARY__IMPORT <operating_system/library/import.hpp>
	#define LIBRARY__END    <operating_system/library/end.hpp>
	/**/#if 0
	// This horrible problem that bloats the headers hopefully
	// only occurs with microsoft's dll format with other compilers than GCC :-)
	// If you really insist on using another compiler on the microsoft operating system,
	// see:
		#include <operating_system/library/export.hpp>
		#include <operating_system/library/import.hpp>
	// then, starts bloating every header and source files with "appropriate" preprocessor directives.
	// Example of use:
		// in file foo/bar.cpp:
			#define LIBRARY__FOO__BAR
			#include "bar.hpp"
		// in file foo/bar.hpp:
			#pragma once
			#if defined LIBRARY__FOO__BAR
				#include LIBRARY__EXPORT
			#else
				#include LIBRARY__IMPORT
			#endif
			namespace foo
			{
				class bar
				{
				};
			}
	// Alternative way is to use:
		#include <operating_system/library/begin.hpp>
		#include <operating_system/library/end.hpp>
	// which attempt to be smarter by doing,
			#define class class pragma(dllexport)
	// or,
			#define class class pragma(dllimport)
	// so that everything is handled transparently.
	// Example of use:
		// in file foo/bar.cpp:
			#define FOO__BAR 1
			#include "bar.hpp"
		// in file foo/bar.hpp:
			#pragma once
			#define LIBRARY FOO__BAR
			#include LIBRARY__BEGIN
			namespace foo
			{
				class bar
				{
				};
			}
			#include LIBRARY__END
	/**/#endif
#endif

///////////////////////////////////////
/// generic floating point number type
///////////////////////////////////////

#if defined COMPILER__FEATURE__TYPEOF
	typedef typeof(0.) real;
#else
	// no typeof with this compiler yet, assuming default floating point number type for litterals is double.
	typedef double real;
#endif

//////////////////////////
// explicitly sized types
//////////////////////////

#if defined COMPILER__FEATURE__NOT_CONCRETE
	namespace compiler
	{
		/// >=80-bit floating point number
		typedef long double float80;
		/// 64-bit floating point number
		typedef double float64;
		/// 32-bit floating point number
		typedef float float32;
		/// signed 64-bit integral number
		typedef signed long int sint64; 
		/// unsigned 64-bit integral number
		typedef unsigned long int uint64;
		/// signed 32-bit integral number
		typedef signed int sint32;
		/// unsigned 32-bit integral number
		typedef unsigned int uint32;
		/// signed 16-bit integral number
		typedef signed short int sint16;
		/// unsigned 16-bit integral number
		typedef unsigned short int uint16;
		/// 8-bit integral number
		typedef signed char sint8;
		/// unsigned 8-bit integral number
		typedef unsigned char uint8;
	}
#elif defined CONFIGURATION__PSYCLE
	namespace compiler
	{
		#if CONFIGURATION__PSYCLE__SIZEOF_LONG_DOUBLE >= 10
			typedef long double float80;
		#elif CONFIGURATION__PSYCLE__SIZEOF_DOUBLE >= 10
			typedef double float80;
		#elif CONFIGURATION__PSYCLE__SIZEOF_FLOAT >= 10
			typedef float float80;
		#else
			#error "No >=80-bit floating point number."
		#endif
		#if CONFIGURATION__PSYCLE__SIZEOF_LONG_DOUBLE == 8
			typedef long double float64;
		#elif CONFIGURATION__PSYCLE__SIZEOF_DOUBLE == 8
			typedef double float64;
		#elif CONFIGURATION__PSYCLE__SIZEOF_FLOAT == 8
			typedef float float64;
		#else
			#error "No 64-bit floating point number."
		#endif
		#if CONFIGURATION__PSYCLE__SIZEOF_LONG_DOUBLE == 4
			typedef long double float32;
		#elif CONFIGURATION__PSYCLE__SIZEOF_DOUBLE == 4
			typedef double float32;
		#elif CONFIGURATION__PSYCLE__SIZEOF_FLOAT == 4
			typedef float float32;
		#else
			#error "No 32-bit floating point number."
		#endif
	}
	#if defined CONFIGURATION__PSYCLE__HAVE_INTTYPES_H
		#include <boost/cstdint.hpp>
		namespace compiler
		{
			typedef boost::uint64_t uint64;
			typedef boost::int64_t sint64;
			typedef boost::uint32_t uint32;
			typedef boost::int32_t sint32;
			typedef boost::uint16_t uint16;
			typedef boost::int16_t sint16;
			typedef boost::uint8_t uint8;
			typedef boost::int8_t sint8;
		}
	#else
		namespace compiler
		{
			#if CONFIGURATION__PSYCLE__SIZEOF_LONG_LONG_INT == 8
				typedef signed long long int sint64;
				typedef unsigned long long int uint64;
			#elif CONFIGURATION__PSYCLE__SIZEOF_LONG_INT == 8
				typedef signed long int sint64;
				typedef unsigned long int uint64;
			#elif CONFIGURATION__PSYCLE__SIZEOF_INT == 8
				typedef signed int sint64;
				typedef unsigned int uint64;
			#elif CONFIGURATION__PSYCLE__SIZEOF_SHORT_INT == 8
				typedef signed short int sint64;
				typedef unsigned short int uint64;
			#elif CONFIGURATION__PSYCLE__SIZEOF_CHAR == 8
				typedef signed char sint64;
				typedef unsigned char uint64;
			#else
				#error "No 64-bit integral number."
			#endif
			#if CONFIGURATION__PSYCLE__SIZEOF_LONG_LONG_INT == 4
				typedef signed long long int sint32;
				typedef unsigned long long int uint32;
			#elif CONFIGURATION__PSYCLE__SIZEOF_LONG_INT == 4
				typedef signed long int sint32;
				typedef unsigned long int uint32;
			#elif CONFIGURATION__PSYCLE__SIZEOF_INT == 4
				typedef signed int sint32;
				typedef unsigned int uint32;
			#elif CONFIGURATION__PSYCLE__SIZEOF_SHORT_INT == 4
				typedef signed short int sint32;
				typedef unsigned short int uint32;
			#elif CONFIGURATION__PSYCLE__SIZEOF_CHAR == 4
				typedef signed char sint32;
				typedef unsigned char uint32;
			#else
				#error "No 32-bit integral number."
			#endif
			#if CONFIGURATION__PSYCLE__SIZEOF_LONG_LONG_INT == 2
				typedef signed long long int sint16;
				typedef unsigned long long int uint16;
			#elif CONFIGURATION__PSYCLE__SIZEOF_LONG_INT == 2
				typedef signed long int sint16;
				typedef unsigned long int uint16;
			#elif CONFIGURATION__PSYCLE__SIZEOF_INT == 2
				typedef signed int sint16;
				typedef unsigned int uint16;
			#elif CONFIGURATION__PSYCLE__SIZEOF_SHORT_INT == 2
				typedef signed short int sint16;
				typedef unsigned short int uint16;
			#elif CONFIGURATION__PSYCLE__SIZEOF_CHAR == 2
				typedef signed char sint16;
				typedef unsigned char uint16;
			#else
				#error "No 16-bit integral number."
			#endif
			#if CONFIGURATION__PSYCLE__SIZEOF_LONG_LONG_INT == 1
				typedef signed long long int sint8;
				typedef unsigned long long int uint8;
			#elif CONFIGURATION__PSYCLE__SIZEOF_LONG_INT == 1
				typedef signed long int sint8;
				typedef unsigned long int uint8;
			#elif CONFIGURATION__PSYCLE__SIZEOF_INT == 1
				typedef signed int sint8;
				typedef unsigned int uint8;
			#elif CONFIGURATION__PSYCLE__SIZEOF_SHORT_INT == 1
				typedef signed short int sint8;
				typedef unsigned short int uint8;
			#elif CONFIGURATION__PSYCLE__SIZEOF_CHAR == 1
				typedef signed char sint8;
				typedef unsigned char uint8;
			#else
				#error "No 8-bit integral number."
			#endif
		}
	#endif
#elif defined COMPILER__MICROSOFT
	namespace compiler
	{
		typedef long double float80;
		typedef double float64;
		typedef float float32;
		typedef signed __int64 sint64; 
		typedef unsigned __int64 uint64;
		typedef signed __int32 sint32;
		typedef unsigned __int32 uint32;
		typedef signed __int16 sint16;
		typedef unsigned __int16 uint16;
		typedef signed __int8 sint8;
		typedef unsigned __int8 uint8;
	}
#else
	#error "No explicitly sized types."
#endif



///\}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif // !defined COMPILER__RESOURCE (only relevant on microsoft's operating system).
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// end
#endif
