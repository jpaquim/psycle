#if defined PRE_COMPILED_HEADERS
	#error pre-compiled headers already included
#else
	#define PRE_COMPILED_HEADERS
#endif



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**

	\file
	\brief inclusions of headers which must be pre-compiled.

*/
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



#if !defined COMPILER__GNU
	// gcc does not support pre-compiled headers yet,
	// the following inclusions would just slow it down considerably



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



#pragma message("parsing " __FILE__)



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



#if defined COMPILER__MICROSOFT && COMPILER__VERSION__MAJOR < 7
#	pragma warning(push, 3) // msvc6's standard libraries generate level-4 warnings... hmm, no comment
#endif

////////////////
// c++ standard
////////////////

#if defined COMPILER__MICROSOFT
#	pragma warning(disable:4702) // unreachable code
#	include <stdexcept>
#	pragma warning(default:4702) // unreachable code
#endif
#include <cstddef>
#include <cstdlib>
#include <cmath>
#include <cstring> // memset, memcpy, memmove
#include <cassert>
#include <csignal>
#include <ctime> // clock_t clock() / CLOCKS_PER_SEC (\todo not as accurate as the performance counter, CLOCKS_PER_SEC is only 1000)
#include <exception>
#include <typeinfo>
#include <limits>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <iomanip>
#include <iterator>
#include <vector>
#include <list>
#include <set>
#include <map>
#include <algorithm>
#include <functional>

/////////
// posix
/////////

#if defined OPERATING_SYSTEM__POSIX
#	include <sys/unistd.h>
#endif

///////////////
// os-specific
///////////////

#if defined OPERATING_SYSTEM__MICROSOFT
	// excludes rarely used stuff from windows headers:
#	if !defined WIN32_LEAN_AND_MEAN
#		define WIN32_LEAN_AND_MEAN
#	endif
#	if !defined VC_EXTRA_LEAN
#		define VC_EXTRA_LEAN
#	endif
#	if !defined _AFXDLL
		// when mfc is used we must not do the following ("explained" below)
#		if !defined WIN32_EXTRA_LEAN
#			define WIN32_EXTRA_LEAN // for mfc apps, we would get unresolved symbols
#		endif
#		include <windows.h> // for mfc apps,  we would get a fatal error C1189: #error :  WINDOWS.H already included.  MFC apps must not #include <windows.h>
#	endif
#endif

//////////////////////////
// boost http://boost.org
//////////////////////////

#if defined OPERATING_SYSTEM__CROSSPLATFORM
#	include <boost/thread/thread.hpp>
#	include <boost/thread/mutex.hpp>
#	include <boost/thread/recursive_mutex.hpp>
#	include <boost/thread/condition.hpp>
#	include <boost/spirit.hpp>
#	if defined COMPILER__MICROSOFT
#		if !defined LIBRARY__BOOST__PATH
#			define LIBRARY__BOOST__PATH "/usr/lib/boost/libs/"
#		endif
#		if defined NDEBUG
#			pragma comment(lib, LIBRARY__BOOST__PATH "thread/build/bin-stage/boost_thread")
#		else
#			pragma comment(lib, LIBRARY__BOOST__PATH "thread/build/bin-stage/boost_threadd")
#		endif
#	endif
#endif

////////////////////////
// fltk http://fltk.org
////////////////////////

#if defined OPERATING_SYSTEM__CROSSPLATFORM
	// when compiling fltk dynamic shared library
//#		define USE_CONF
//#		define FL_DLL
//#		define FL_LIBRARY
//#		define FL_GL_LIBRARY
//#		pragma comment(lib, "OpenGL32")
//#		define FL_IMAGES_LIBRARY
//#		define _MSC_DLL
//#		define WIN32
//#		define _WINDOWS
//#		pragma comment(lib, "wsock32")
#	if !defined FL_SHARED
#		define FL_SHARED // when using fltk dynamic shared library
#	endif
#	if !defined _WIN32_WINNT
#		define _WIN32_WINNT 0x0500
#	endif
#	if 0 // disabled since i've put fltk in the directory tree
#	if !defined LIBRARY__FLTK__PATH
#		define LIBRARY__FLTK__PATH "/home/bohan/projects/cvs.sourceforge.net/fltk/fltk/lib/"
#	endif
#	if defined NDEBUG
#		pragma comment(lib, LIBRARY__FLTK__PATH "fltkdll")
#	else
#		pragma comment(lib, LIBRARY__FLTK__PATH "fltkdlld")
#	endif
#	endif
#endif

#if defined COMPILER__MICROSOFT && COMPILER__VERSION__MAJOR < 7
#	pragma warning(pop)
#endif



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



#pragma message("done parsing " __FILE__)



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



#endif // !defined COMPILER__GNU // gcc does not support pre-compiled headers yet



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
