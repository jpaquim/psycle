#pragma once

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**

	\file
	\brief project-wide compiler, operating system, and processor specific tweaks.
	\verbatim

	compilers:

		// gnu g++/gcc, g++ can be detected via __GNUG__ (equivalent to __GNUC__ && __cplusplus)
		#define	COMPILER__GNU
		#define	COMPILER__VERSION__MAJOR __GNUC__
		#define	COMPILER__VERSION__MINOR __GNUC_MINOR__
		#define	COMPILER__VERSION__PATCH __GNUC_PATCHLEVEL__

		// bcc (c++builder or free version)
		#define COMPILER__BORLAND
		#define COMPILER__VERSION__MAJOR 5
		#define COMPILER__VERSION__MINOR 6

		// msvc can be detected via _MSC_VER
		// msvc 6.?.? can be detected via _MSC_VER < 1300, no way to detect service pack 5 nor processor pack
		// msvc 7.0 can be detected via _MSC_VER < 1310
		// msvc 7.1 can be detected via _MSC_VER >= 1310
		#define COMPILER__MICROSOFT
		#define COMPILER__VERSION__MAJOR 7
		#define COMPILER__VERSION__MINOR 1

	operating systems:

		// the posix standard version 2
		#define OPERATING_SYSTEM__POSIX

		// gnu
		#define OPERATING_SYSTEM__GNU

		// hurd kernel
		#define OPERATING_SYSTEM__HURD

		// linux kernel can be detected via __LINUX__
		#define OPERATING_SYSTEM__LINUX
		#define OPERATING_SYSTEM__VERSION__MAJOR 2
		#define OPERATING_SYSTEM__VERSION__MINOR 4
		#define OPERATING_SYSTEM__VERSION__PATCH 1

		// apple macos
		#define OPERATING_SYSTEM__APPLE
		#define OPERATING_SYSTEM__VERSION__MAJOR 10
		#define OPERATING_SYSTEM__VERSION__MINOR 0
		#define OPERATING_SYSTEM__VERSION__PATCH 0

		// mswin can be detected via _WIN32 and _WIN64
		// <bohan> i don't know what to do with old versions (9x/me)...
		// <bohan> it would be cubersome to support such old systems.
		// <bohan> are there still many users of these?
		#define OPERATING_SYSTEM__MICROSOFT
		#define OPERATING_SYSTEM__VERSION__MAJOR 5
		#define OPERATING_SYSTEM__VERSION__MINOR 1
		#define OPERATING_SYSTEM__VERSION__PATCH 2600 // service pack 1

	processors:

		#define PROCESSOR__ENDIAN__BIG // natural, big endian byte order
		#define PROCESSOR__ENDIAN__LITTLE // reversed, little endian byte order
		#define PROCESSOR__ENDIAN__WEIRD // weird endian byte order

		#define PROCESSOR__X86 6 // 686 (from pentium-pro to amd athlon or opteron-64 and intel pentium-iii)
		#define PROCESSOR__X86 7 // 786 (intel pentium-iv (sse2))

		#define PROCESSOR__POWER_PC 4 // ibm/motorola power pc g4
		#define PROCESSOR__POWER_PC 5 // ibm/motorola power pc g5
		#define PROCESSOR__POWER_PC 6 // ibm/motorola power pc g6

		#define PROCESSOR__IA // intel ia64

	\endverbatim
*/
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// compilers



#if defined __GNUC__
#	define COMPILER__GNU
#	define COMPILER__VERSION__MAJOR __GNUC__
#	define COMPILER__VERSION__MINOR __GNUC_MINOR__
#	define COMPILER__VERSION__PATCH __GNUC_PATCHLEVEL__
#	if !defined __GNUG__ || !defined __cpluscplus
#		if defined __GNUG__ || defined __cpluscplus
#			error weird settings... we should have both __GNUG__ and __cpluscplus
#		else
#			error please invoke gcc with the language option set to c++ (or invoke gcc via the g++ alias)
#		endif
#	endif
#	if COMPILER__VERSION__MAJOR < 2
		// <bohan> bah. that compiler is too old. no recent source will fit.
#		error compiler too old... better giving up now.
#	endif
#endif

#if defined _MSC_VER
#	define COMPILER__MICROSOFT
#	if _MSC_VER < 1300 // msvc 6
		// <bohan> bah. that compiler is too old. no recent source will fit.
#		define COMPILER__VERSION__MAJOR 6
#		define COMPILER__VERSION__MINOR 5
			// service pack 5
#		define COMPILER__VERSION__PATCH 1
			// processor pack after service pack 5
#	else
#		define COMPILER__VERSION__MAJOR 7
#		if _MSC_VER < 1310 // msvc 7.0
#			define COMPILER__VERSION__MINOR 0
#		else // msvc 7.1
#			define COMPILER__VERSION__MINOR 1
#		endif
#	endif
#	if COMPILER__VERSION__MAJOR < 7
		// <bohan> bah. that compiler is too old. no recent source will fit.
#		error ouch, shitty compiler... better giving up now.
#	endif
#	pragma conform(forScope, on)
#endif



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// operating systems



#if defined __LINUX__
#	define OPERATING_SYSTEM__LINUX 20401
#	define OPERATING_SYSTEM__VERSION__MAJOR 2
#	define OPERATING_SYSTEM__VERSION__MINOR 4
#	define OPERATING_SYSTEM__VERSION__PATCH 1
#endif

#if defined _WIN32
#	if defined _WIN64
#		error todo...
#	else
		// <bohan> i don't know what to do with old versions (9x/me)... are there still many users of these?
#		define OPERATING_SYSTEM__MICROSOFT
#		define OPERATING_SYSTEM__VERSION__MAJOR 5
			// >= 2k
#		define OPERATING_SYSTEM__VERSION__MINOR 1
			// >= xp/2k3.net
#		define OPERATING_SYSTEM__VERSION__PATCH 2600
			// service pack 1
#	endif
#endif

// operating system kernels for gnu
#if defined OPERATING_SYSTEM__HURD || defined OPERATING_SYSTEM__LINUX || defined OPERATING_SYSTEM__BSD
#	define OPERATING_SYSTEM__GNU
#endif

// operating systems following the posix standard
#if defined OPERATING_SYSTEM__GNU
#	define OPERATING_SYSTEM__POSIX
#endif

// x window (on the apple macosx operating system, we have to choose whether we want to use x window or not)
#if !defined OPERATING_SYSTEM__MICROSOFT && (!defined OPERATING_SYSTEM__APPLE || defined OPERATING_SYSTEM__CROSSPLATFORM)
#	define OPERATING_SYSTEM__X_WINDOW
#endif



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// processors



#if defined COMPILER__MICROSOFT
#	if defined _M_IA64
#		define PROCESSOR__IA
#	elif defined _M_IX86
#		define PROCESSOR__X86 _M_IX86 / 100
#	else
#		error microsoft gave up will all processors but x86 and ia
#	endif
#endif

#if defined OPERATING_SYSTEM__APPLE
#	define PROCESSOR__POWER_PC 5
	// implied because the version of the apple operating system for 68k processors has been discontinued.
	// now, only power pc processors are supported by this operating system.
#endif

// minimum requirements for processors
#if defined PROCESSOR__X86 && PROCESSOR__X86 < 6
#	error target processor too old
	// <bohan> actually, i don't know why we should put such a strong requirement.
	// the code should compile and run fine on any 32-bit cpu, that is, up from 386(dx?).
#endif

// processor endianess
#if defined PROCESSOR__POWER_PC
#	define PROCESSOR__ENDIAN__BIG
#elif defined PROCESSOR__X86 || defined PROCESSOR__IA
#	define PROCESSOR__ENDIAN__LITTLE
#else
#	error unkown byte sex
#endif



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// extensions to the language which would become standard



// online documentation
	// COMPILER__GNU
		// http://gcc.gnu.org/onlinedocs/gcc
	// COMPILER__MICROSOFT
		// command line options: http://msdn.microsoft.com/library/default.asp?url=/library/en-us/vccore/html/vcrefCompilerOptionsListedAlphabetically.asp
		// #pragma: http://msdn.microsoft.com/library/default.asp?url=/library/en-us/vclang/html/_predir_pragma_directives.asp
		// exception handling: http://msdn.microsoft.com/library/default.asp?url=/library/en-us/vclang/html/_core_exception_handling_topics_.28.c.2b2b29.asp



/////////////
// attribute
/////////////

#if defined COMPILER__GNU
#	define attribute(x) __attribute__ ((x))
#elif defined COMPILER__MICROSOFT
#	define attribute(x) __declspec(x)
#endif

//////////////////////
// deprecated warning
//////////////////////

//#define deprecated attribute(deprecated)
// won't work if not stringized
// use attribute(deprecated) instead for now

////////////////////////
// thread local storage
////////////////////////

#if defined COMPILER__GNU
#	define thread_local_storage __thread
#elif defined COMPILER__MICROSOFT
#	define thread_local_storage attribute(thread)
#endif

////////////////////
// aligned / packed
////////////////////

#if defined COMPILER__GNU
#	define aligned(x) attribute(aligned(x))
#	define packed attribute(packed)
	// packed is an attribute
	// note: a bit field is packed to 1 bit, not one byte.
#elif defined COMPILER__MICROSOFT
#	define aligned(x) attribute(align(x))
#	define packed align(1)
	// see also: #pragma pack(x) in the optimization section
#endif

// example: class RIFF_Chunk attribute(packed, aligned(1)) { unsigned int8[4] tag; unsigned int32 size; unsigned int8[0] data; }

///////
// asm
///////

#if defined COMPILER__MICROSOFT
#	define asm __asm
#endif

////////////
// restrict
////////////

// this keyword has been introduced in the iso 1999 standard.
// it is a hint for the compiler telling it some references or pointers will *always* holds different memory addresses.
// hence it can optimize more, knowing that writting to memory via one reference cannot alias antoher.
// example:
	// void f(int & restrict r1, int & restrict r2, int * restrict p1, int restrict p2[]);
	// here the compiler is told that &r1 != &r2 != p1 != p2

#if defined COMPILER__GNU
#	define restrict __restrict
#elif defined COMPILER__MICROSOFT
#	define restrict
	// unsupported?
	// see also: #pragma optimize("a", on) // assumes no aliasing
#endif

//////////
// assume
//////////

// <bohan> i don't quite understand what microsoft says about this keyword...

#if defined COMPILER__GNU
#	define assume (x)
	// unsupported?
#elif defined COMPILER__MICROSOFT
#	define assume(x) __assume(x)
#endif

/////////
// super
/////////

// <bohan> this keyword is just syntactic sugar

#if defined COMPILER__GNU
	// unsupported?
#elif defined COMPILER__MICROSOFT
	// super is a keyword
#endif

//////////
// typeof
//////////

#if defined COMPILER__GNU
	// typeof is a standard keyword
#elif defined COMPILER__MICROSOFT
#	define typeof __typeof
	// it looks like a reserved keyword, but it's not implemented yet (only in c#).
#endif

///////////////////////////////
// generic floating point type
///////////////////////////////

#if defined COMPILER__MICROSOFT
	// no typeof with this compiler yet (only in c#).
	typedef double real;
#else
	typedef typeof(0.) real;
#endif

//////////////////////////
// explicitly sized types
//////////////////////////

typedef long double float80;
typedef double float64;
typedef float float32;
typedef __int64 int64;
typedef __int32 int32;
typedef __int16 int16;
typedef __int8 int8;

///////////
// wchar_t
///////////

#if defined COMPILER__MICROSOFT && 0 // <bohan> disabled... i can't help microsoft with this... let them fix that flaw.
	// the microsoft compiler has a broken implementation of wchar_t: it is 16 bits while it actually must be 32 bits.
	// see also: _WCHAR_T_DEFINED _NATIVE_WCHAR_T_DEFINED
#	if defined wchar_t
#		undef wchar_t
		typedef unsigned int32 __identifier(wchar_t);
#	else
#		define wchar_t unsigned int32
		// we cannot use a typedef since wchar_t is already be type, and we cannot #undef a type.
#	endif
#endif

//////////////////////////
// try-finally statements
//////////////////////////

// msvc's try-finally statements are useless because they cannot be mixed with try-catch statements in the same function
// <bohan> this was with msvc 6. it might have been fixed with msvc 7.1
#if defined COMPILER__GNU
#	define try_finally
#	define finally
#elif defined COMPILER__BORLAND
#	define try_finally try
#	define finally __finally
#elif defined COMPILER__MICROSOFT
#	define try_finally
#	define finally
#else
#	error unkown compiler
#endif

///////////////////////////////
// hardware exception handling
///////////////////////////////

// we want to be able to catch hardware exceptions as c++ ones, not only via catch(...)
#if defined COMPILER__GNU
#	define try_hard try
#elif defined COMPILER__BORLAND
#	define try_hard try
#elif defined COMPILER__MICROSOFT
#	define try_hard try
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
		// function exception specification                      EHsc==GX   EHs        EHa        EHac
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
		// <bohan> conclusion: the best mode is EHsc (==GX) even if we can have some memory leaks if hardware exceptions are raised
		// <bohan> note:
		// <obhan> there is a bug in msvc concerning synchronous exception handling model ... disable it locally where necessary
		// <bohan> problems appears when using synchronous exception handling model...
		// <bohan> on msvc 6, i had to use asynchronous exception handling
		// <bohan> this is still not fixed in different with msvc 7.1 (it's only fixed with global optimization).
		// <bohan> it hopefully generates a warning->error, otherwize, we would have a runtime bug
		// <bohan> warning C4702: unreachable code
		// <bohan> error C2220: warning treated as error - no object file generated
		// <bohan> as of 2003-12-11 i decided not to make the compile treat the warning as an error...
		// <bohan> yet, i don't know if the compiler actually puts the code in the binary or discard it badly :/
		//
		// <bohan> shit! warning C4535: calling _set_se_translator() requires /EHa; the command line options /EHc and /GX are insufficient
#else
#	error unkown compiler
#endif

////////////////////////////////
// function calling conventions
////////////////////////////////

// calling convention | modifier keyword | parameters stack push            | parameters stack pop | extern "C" symbol name mangling                   | extern "C++" symbol name mangling
// register           | __fastcall       | 3 registers then pushed on stack | callee               | '@' and arguments' byte size in decimal prepended | no standard
// pascal             | __pascall        | left to right                    | callee               | uppercase                                         | no standard
// standard call      | __stdcall        | right to left                    | callee               | preserved                                         | no standard
// c declaration      | __cdecl          | right to left, variable count    | caller               | '_' prepended                                     | no standard

// note: the register convention is different from one compiler to another (for example left to right for borland, right to left for microsoft).
// note: on gcc, one can use the #define __USER_LABEL_PREFIX__ to know what character is prepended to extern "C" symbols

#define stdcall __stdcall

// \todo i don't know why windef.h defines cdecl to nothing... because of this, functions with cdecl in their signature aren't considered to be of the same type as ones with __cdecl in their signature
// <bohan> this bug was with msvc 6. it might have been fixed with msvc 7.1
//#undef cdecl
//#define cdecl __cdecl

#if defined COMPILER__BORLAND
#	define fastcall __msfastcall
		// borland compatibility with msvc's register calling convention:
#elif defined COMPILER__MICROSOFT
#	define fastcall __fastcall
#else
#	define fastcall
#endif

#define CALL fastcall
	// member functions defaults to thiscall
	// non-member functions defaults to cdecl

///////////////////////////////
// shared dynamic-link library
///////////////////////////////

#if defined OPERATING_SYSTEM__MICROSOFT
	// the microsoft dll horror show system begins...
#	define IMPORT attribute(dllimport)
#	define EXPORT attribute(dllexport)
#else
	// everything is nice and simple
#	define IMPORT
#	define EXPORT
#endif
#define LIBRARY__IMPORT <operating_system/library/import.h>
#define LIBRARY__EXPORT <operating_system/library/export.h>



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// debug



#if defined COMPILER__MICROSOFT
	// microsoft uses its own vendor #define _DEBUG (for debug), instead of the iso #define NDEBUG (for no debug)
	// so, we recover the iso way
#	if !defined NDEBUG
#		if !defined _DEBUG
#			define NDEBUG
#			pragma message("no debug")
#		else
#			pragma message("debug")
#		endif
#	elif defined _DEBUG
#		error both #define NDEBUG (no debug) and #define _DEBUG (debug)... this is non sensical
#	endif
#endif



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// warnings



#if defined COMPILER__MICROSOFT

#	pragma warning(push, 4) // generate level-4 (i.e. all) warnings
		// <bohan> note: we never pops this one... but there's no syntax without push.

	//////////////////////////////
	// warnings treated as errors
	//////////////////////////////
#	pragma warning(error:4662) // explicit instantiation; template-class 'X' has no definition from which to specialize X<...>
#	pragma warning(error:4150) // deletion of pointer to incomplete type 'X<...>'; no destructor called
#	pragma warning(error:4518) // storage-class or type specifier(s) unexpected here; ignored

	/////////////
	// weirdness
	/////////////
//#	pragma warning(disable:4702) // unreachable code
		// see comments on exception handling (and also global optimization on msvc 6)
		// <bohan> actually, here is what i did:
		// <bohan> the problem appeared in <vector>, but seems to be due to code in <stdexcept>, which is #included by <vector>.
		// <bohan> i've just disabled the warning/error in precompiled_headers.h,
		// <bohan> around the #include <stdexcept> (which is #included just for the fix)
		// <bohan> maybe there will be more problematic headers appearing later, i'll do the same as with <stdexcept>.

	//////////
	// shiatz
	//////////
#	pragma warning(disable:4258) // definition from the for loop is ignored; the definition from the enclosing scope is used
#	pragma warning(disable:4673) // thrown exception type not catched
#	pragma warning(disable:4290) // c++ exception specification not yet implemented except to indicate a function is not __declspec(nothrow)
		// "A function is declared using exception specification, which Visual C++ accepts but does not implement.
		// Code with exception specifications that are ignored during compilation may need to be
		// recompiled and linked to be reused in future versions supporting exception specifications."
#	if COMPILER__VERSION__MAJOR < 70
#		pragma warning(disable:4786) // identifier was truncated to '255' characters in the debug information
#	endif

	//////////
	// stupid
	//////////
#	pragma warning(disable:4251) // class 'X' needs to have dll-interface to be used by clients of class 'Y'
#	pragma warning(disable:4275) // non dll-interface class 'X' used as base for dll-interface class 'Y'"

	/////////
	// style
	/////////
#	pragma warning(disable:4554) // check operator precedence for possible error; use parentheses to clarify precedence
#	pragma warning(disable:4706) // assignment within conditional expression
#	pragma warning(disable:4127) // conditional expression is constant
#	pragma warning(disable:4100) // unreferenced formal parameter

	////////
	// cast
	////////
#	pragma warning(disable:4800) // forcing value to bool 'true' or 'false' (performance warning)
#	pragma warning(disable:4244) // conversion from 'int' to 'float', possible loss of data
		// x-bit int to x-bit float conversions are not lossless, but in many cases we do not care about it

	//////////
	// inline
	//////////
#	pragma warning(disable:4711) // selected for automatic inline expansion
#	pragma warning(disable:4710) // function was not inlined
#	pragma warning(disable:4714) // function marked as __forceinline not inlined

	/////////////////////////
	// implicit constructors
	/////////////////////////
#	pragma warning(disable:4512) // assignment operator could not be generated
#	pragma warning(disable:4511) // copy constructor could not be generated

	///////////////
	// yes, yes...
	///////////////
//#	pragma warning(disable:4096) // '__cdecl' must be used with '...'
#	pragma warning(disable:4652) // compiler option 'link-time code generation (/GL)' inconsistent with precompiled header; current command-line option will override that defined in the precompiled header
		// see the comments about the /GL option in the optimization section
		// note:
		// since the warning is issued before the first line of code is even parsed,
		// it is not possible to disable it using a #pragma.
		// i just put it here for consistency and documentation, but you should disable it
		// using the /Wd4652 command line option to cl, or
		// in the project settings under C++ / Advanced / Disable Specific Warnings.
#	pragma warning(disable:4651) // '/D_XXX' specified for pre-compiled header but not for current compilation
		// beware with this ... it is disabled because of /D_WINDLL ... otherwize it could be relevant

#endif



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// optimizations



#if defined COMPILER__MICROSOFT
#	pragma pack(8)
#	if defined NDEBUG // if no dedug
#		pragma runtime_checks("c", off) // reports when a value is assigned to a smaller data type that results in a data loss
#		pragma runtime_checks("s", off) // stack (frame) verification
#		pragma runtime_checks("u", off) // reports when a variable is used before it is defined
		// Run-time error checks is a way for you to find problems in your running code.
		// The run-time error checks feature is enabled and disabled by the /RTC group of compiler options and the runtime_checks pragma.
		// Note:
		// If you compile your program at the command line using any of the /RTC compiler options,
		// any pragma optimize instructions in your code will silently fail.
		// This is because run-time error checks are not valid in a release (optimized) build.
		// <bohan> this mean we must not have the /RTC option enabled to enable optimizations, even if we disable runtime checks here.
#		pragma optimize("s", off) // favors small code
#		pragma optimize("t", on) // favors fast code
#		pragma optimize("p", off) // improves floating-point consistency (this is for iso conformance, slower and less precise)
#		pragma optimize("a", on) // assumes no aliasing, see also: the restrict iso keyword
#		pragma optimize("w", off) // assumes that aliasing can occur across function calls
#		pragma optimize("y", off) // generates frame pointers on the program stack
#		pragma optimize("g", on) // global optimization
			// <bohan> note: there is a bug in msvc 6 concerning global optimization ... disable it locally where necessary
			// <bohan> it hopefully generates a warning->error, otherwize, we would have a runtime bug
			// <bohan> warning C4702: unreachable code
			// <bohan> error C2220: warning treated as error - no object file generated
			// <bohan> this bug was with msvc 6. it seems to have been fixed with msvc 7.1,
			// <bohan> but it still appears with msvc 7.1 when using synchronous exception hnalding model.
//		no #pragma for /GL == enable whole program, accross .obj, optimization
			// <bohan> we have to disable that optimization in some projects because it's boggus.
			// <bohan> because it's only performed at link time it doesn't matter if the precompiled headers were done using it or not.
			// <bohan> static initialization and termination is done/ordered weirdly when enabled
			// <bohan> so, if you experience any weird bug,
			// <bohan> first thing to try is to disable this /GL option in the command line to cl, or
			// <bohan> in the project settings, it's hidden under General Properties / Whole Program Optimization.
#		pragma inline_depth(255)
#		pragma inline_recursion(on)
#		define inline __forceinline
#	endif
#endif



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// pre-compiled headers



#if !defined COMPILER__GNU
	// gcc does not support pre-compiled headers yet,
	// this inclusion would just slow it down considerably
#	include "pre_compiled_headers.h"
#endif



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// processor's floating point unit 
// this is declared after the pre-compiled headers are #included because we may need runtime functions.



namespace processor
{
	namespace fpu
	{
		using ::real;
		inline void set_mode()
		{
#			if defined PROCESSOR__X86
#				if defined OPERATING_SYSTEM__LINUX || OPERATING_SYSTEM__HURD
					; // nothing for now
#				elif defined OPERATING_SYSTEM__MICROSOFT
					_control87(_RC_CHOP | _PC_53 | _EM_INEXACT | _EM_DENORMAL, _MCW_RC | _MCW_PC | _MCW_EM);
						// http://msdn.microsoft.com/library/default.asp?url=/library/en-us/vclib/html/_crt__control87.2c_._controlfp.asp
						// rounding mode: toward zero (iso conformance) (_RC_CHOP, _MCW_RC)
						// precision: 53 bits (_PC_53, _MCW_PC)
						// exception mask: (_EM_INEXACT | _EM_DENORMAL, _MCW_EM)
							// big warning: exceptions are enabled when their *mask* flag is *not* set...
							// so, this enables:  _EM_INVALID | _EM_UNDERFLOW | _EM_OVERFLOW | _EM_ZERODIVIDE
							// and this disables: _EM_INEXACT | _EM_DENORMAL
							// <bohan> i don't know why _EM_DENORMAL stays disabled even when not given as parameter, but it's actually what we want.
							// <bohan> actually, it's was because i used _controlfp, which ignores this for compatibility with non x86 processors
#				endif
#				if defined COMPILER__MICROSOFT
					// see the optimization section for #pragma optimize("p", off)
#				endif
#			elif defined PROCESSOR__POWER_PC
				; // nothing for now
#			else
				; // nothing for now
#			endif
		}
	}
}



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// end
