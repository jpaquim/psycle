///\file
/// function calling conventions

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
#pragma once
#include INCLUDE(PROJECT/project.hpp)
#include INCLUDE(PROJECT/compilers.hpp)
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
	#error "Unsupported compiler ; please add support for function calling conventions for your compiler in the file where this error is triggered."
	//#elif defined COMPILER__<your_compiler_name>
		#define cdecl ...
		#define stdcall ...
		#define pascall ...
		#define fastcall ...
	//#endif
#endif
