// This file is an exact copy of the following file from freepsycle:
// http://bohan.dyndns.org/cgi-bin/archzoom.cgi/psycle@sourceforge.net/psycle--mainline--0--patch-286/src/pre_compiled_headers/pre_compiled_headers.private.hpp
///\file
///\brief inclusions of headers which must be pre-compiled.
///\meta generic
///\meta depends-on-set-of-libraries
#if defined PRE_COMPILED_HEADERS
	#error pre-compiled headers already included
#else
	#define PRE_COMPILED_HEADERS
#endif



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



#pragma message("parsing " __FILE__)



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



#if defined COMPILER__MICROSOFT && COMPILER__VERSION__MAJOR < 7
	#pragma warning(push, 3) // msvc6's standard libraries generate level-4 warnings... hmm, no comment
#endif



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



///////////////
// os-specific
///////////////

#if defined OPERATING_SYSTEM__MICROSOFT
	// excludes rarely used stuff from windows headers:
	#if !defined WIN32_LEAN_AND_MEAN
		/// beware,
		/// when including <gdiplus.h>, MIDL_INTERFACE is not declared,
		/// when including <mmsystem.h>, WAVEFORMATEX is not declared,
		#define WIN32_LEAN_AND_MEAN
	#endif
	#if !defined VC_EXTRA_LEAN
		#define VC_EXTRA_LEAN
	#endif
	#if !defined _AFXDLL
		// when mfc is used we must not do the following ("explained" below)
		#if !defined WIN32_EXTRA_LEAN
			#define WIN32_EXTRA_LEAN // for mfc apps, we would get unresolved symbols
		#endif
		#include <windows.h> // for mfc apps,  we would get a fatal error C1189: #error :  WINDOWS.H already included.  MFC apps must not #include <windows.h>
		//#include <gdiplus.h> // since <windows.h> must be included before <gdiplus.h>,
		// we can't include it here for mfc apps, it is instead included by pre_compiled_headers.private.mfc.hpp after the inclusion of <afxwin.h>.
	#endif
#endif



////////////////
// c++ standard
////////////////

#if defined COMPILER__MICROSOFT
	#pragma warning(push)
		#pragma warning(disable:4702) // unreachable code
		#include <stdexcept>
	#pragma warning(pop)
#endif

// c++ headers
//#include <algorithm>
//#include <bitset>
//#include <complex>
//#include <deque>
#include <exception>
#include <fstream>
//#include <functional>
//#include <iomanip>
//#include <ios>
//#include <iosfwd>
#include <iostream>
#include <istream>
//#include <iterator>
#include <limits>
//#include <list>
//#include <locale>
//#include <map>
#include <memory>
#include <new>
//#include <numeric>
#include <ostream>
//#include <queue>
//#include <set>
#include <sstream>
//#include <stack>
#include <stdexcept>
//#include <streambuf>
#include <string>
#include <typeinfo>
//#include <utility>
//#include <valarray>
#include <vector>

// c headers
#include <cassert>
//#include <cctype>
//#include <cfloat>
//#include <ciso646>
//#include <climits>
//#include <clocale>
#include <cmath>
//#include <csetjmp>
//#include <csignal>
//#include <cstdarg>
//#include <cstddef>
//#include <cstdio>
//#include <cstdlib>
#include <cstring> // std::memset, std::memcpy, std::memmove
//#include <ctime> // std::clock_t std::clock() / CLOCKS_PER_SEC (not as accurate as the performance counter, CLOCKS_PER_SEC is only 1000)

// c headers that might not work if wchar_t support is disabled.
/*
#include <bits/c++config.h>
#if _GLIBCXX_USE_WCHAR_T
	#include <cwchar>
	#include <cwctype>
#endif
*/



/////////
// posix
/////////

#if defined OPERATING_SYSTEM__POSIX
	#include <sys/unistd.h>
	#include <cerrno> // is it ISO?
#endif



//////////////////////////
// boost http://boost.org
//////////////////////////

#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/thread/condition.hpp>
//#include <boost/spirit.hpp>
#if defined COMPILER__MICROSOFT
	#if !defined LIBRARY__BOOST__PATH
		#define LIBRARY__BOOST__PATH ""
	#endif
	#if defined NDEBUG
		#pragma comment(lib, LIBRARY__BOOST__PATH "boost_thread")
	#else
		#pragma comment(lib, LIBRARY__BOOST__PATH "boost_threadd")
	#endif
	#undef LIBRARY__BOOST__PATH
#endif



////////////////////////
// fltk http://fltk.org
////////////////////////
/*
#if defined OPERATING_SYSTEM__CROSSPLATFORM
	// when compiling fltk dynamic shared library
		//#define USE_CONF
		//#define FL_DLL
		//#define FL_LIBRARY
		//#define FL_GL_LIBRARY
		//#pragma comment(lib, "OpenGL32")
		//#define FL_IMAGES_LIBRARY
		//#define _MSC_DLL
		//#define WIN32
		//#define _WINDOWS
		//#pragma comment(lib, "wsock32")
	#if !defined FL_SHARED
		#define FL_SHARED // when using fltk dynamic shared library
	#endif
	#if !defined _WIN32_WINNT
		#define _WIN32_WINNT 0x0500
	#endif
	#if !defined LIBRARY__FLTK__PATH
		#define LIBRARY__FLTK__PATH ""
	#endif
	#if defined NDEBUG
		#pragma comment(lib, LIBRARY__FLTK__PATH "fltkdll")
	#else
		#pragma comment(lib, LIBRARY__FLTK__PATH "fltkdlld")
	#endif
	#endif
#endif
*/



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



#if defined COMPILER__MICROSOFT && COMPILER__VERSION__MAJOR < 7
	#pragma warning(pop)
#endif



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



#if defined _AFXDLL
	#include "pre_compiled_headers.mfc.private.hpp"
#endif



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



#pragma message("done parsing " __FILE__)



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// end
