///\file
#pragma once
#include INCLUDE(PROJECT/project.hpp)
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
