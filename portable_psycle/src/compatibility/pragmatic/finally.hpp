///\file
///\brief try-finally statements
#pragma once
#include INCLUDE(PROJECT/project.hpp)
#include INCLUDE(PROJECT/compilers.hpp)
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
