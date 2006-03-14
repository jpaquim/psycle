// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// Copyright (C) 1999-2006 Johan Boule <bohan@jabber.org>
// Copyright (C) 2004-2006 Psycledelics http://psycle.pastnotecut.org

///\file
///\brief inclusions of headers which must be pre-compiled.
///\meta generic
///\meta depends-on-set-of-libraries
#if defined PACKAGENERIC__PRE_COMPILED__INCLUDED
	#error pre-compiled headers already included
#else
	#define PACKAGENERIC__PRE_COMPILED__INCLUDED
#endif



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



#include <diversalis/compiler.hpp>



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



#if defined DIVERSALIS__COMPILER__MICROSOFT
	#pragma message("packageneric::pre_compiled:: parsing " __FILE__)
#endif



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



#include <diversalis/diversalis.hpp>
#include <universalis/compiler.hpp> // includes universalis' warning settings



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



#if defined DIVERSALIS__COMPILER__MICROSOFT && DIVERSALIS__COMPILER__VERSION__MAJOR < 7
	#pragma warning(push, 3) // msvc6's standard libraries generate level-4 warnings... hmm, no comment
#endif



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



///////////////
// os-specific
///////////////

#if defined DIVERSALIS__COMPILER__MICROSOFT
	#pragma message("packageneric::pre_compiled:: parsing operating-system specific headers")
#endif
#if defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT
	#if defined DIVERSALIS__COMPILER__MICROSOFT
		#pragma warning(push) // don't let microsoft mess around with our warning settings
	#endif
	#if !defined WIN32_LEAN_AND_MEAN
		/// excludes rarely used stuff from windows headers.
		/// beware,
		/// when including <gdiplus.h>, MIDL_INTERFACE is not declared, needs <atlbase.h>.
		/// when including <mmsystem.h>, WAVEFORMATEX is not declared
		#define WIN32_LEAN_AND_MEAN
	#endif
	#if !defined VC_EXTRA_LEAN
		/// excludes some more of the rarely used stuff from windows headers.
		#define VC_EXTRA_LEAN
	#endif
	#if !defined NOMINMAX
		/// tells microsoft's headers not to pollute the global namespace with min and max macros (which break a lot of libraries, including the standard c++ library!)
		#define NOMINMAX
	#endif
	#if defined _AFXDLL // when mfc is used
		#if defined DIVERSALIS__COMPILER__MICROSOFT
			#pragma message("packageneric::pre_compiled:: parsing mfc headers")
		#endif
		#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS // some CString constructors will be explicit
		#define _AFX_ALL_WARNINGS // turns off mfc's hiding of some common and often safely ignored warning messages
		#include <afxwin.h> // mfc core and standard components
		#include <afxext.h> // mfc extensions
		//#include <afxdisp.h> // mfc Automation classes
		#include <afxdtctl.h> // mfc support for Internet Explorer Common Controls
		#if !defined _AFX_NO_AFXCMN_SUPPORT
			#include <afxcmn.h> // mfc support for Windows Common Controls
		#endif
		#include <afxmt.h> // multithreading?
		#if defined DIVERSALIS__COMPILER__MICROSOFT
			#pragma message("packageneric::pre_compiled:: done parsing mfc headers")
		#endif
	#else
		#if !defined WIN32_EXTRA_LEAN
			#define WIN32_EXTRA_LEAN // for mfc apps, we would get unresolved symbols
		#endif
		#include <windows.h>
	#endif
	#if 1 || !defined NOMINMAX
		// The following two mswindows macros break a lot of libraries, including the standard c++ library!
		#undef min
		#undef max
	#endif
	// gdi+, must be included after <windows.h> or <afxwin.h>
	#if defined DIVERSALIS__COMPILER__MICROSOFT ///\todo is gdi+ available with other compilers than microsoft's? by default, it's safer to assume it's not, as usual.
		#if !defined min || !defined max // gdi+ needs min and max in the root namespace :-(
			// the standard c++ header <algorithm> defines std::min and std::max, but microsoft named them __cpp_min and __cpp_max, prefering their own macro stuff rather than the standard
			#if !defined min
				/// replacement for mswindows' min macro, but using a template in the root namespace instead of the ubiquitous macro, so we prevent clashes.
				template<typename X> X const inline & min(X const & x1, X const & x2) { return x1 < x2 ? x1 : x2; }
			#endif
			#if !defined max
				/// replacement for mswindows' max macro, but using a template in the root namespace instead of the ubiquitous macro, so we prevent clashes.
				template<typename X> X const inline & max(X const & x1, X const & x2) { return x1 > x2 ? x1 : x2; }
			#endif
		#endif
		#if defined _AFXDLL // when mfc is used, also include <gdiplus.h>
			//#include <atlbase.h> // for MIDL_INTERFACE used by <gdiplus.h>
			#include <gdiplus.h>
			#if defined DIVERSALIS__COMPILER__FEATURE__AUTO_LINK
				#pragma comment(lib, "gdiplus")
			#endif
		#endif
	#endif
	#if defined DIVERSALIS__COMPILER__MICROSOFT
		#pragma warning(pop) // don't let microsoft mess around with our warning settings
	#endif
#endif
#if defined DIVERSALIS__COMPILER__MICROSOFT
	#pragma message("packageneric::pre_compiled:: done parsing operating-system specific headers")
#endif



////////////////
// c++ standard
////////////////

#if defined DIVERSALIS__COMPILER__MICROSOFT
	#pragma message("packageneric::pre_compiled:: parsing standard c++ headers")
#endif

#if defined DIVERSALIS__COMPILER__MICROSOFT
	#define _CRT_SECURE_NO_DEPRECATE
#endif

#if defined DIVERSALIS__COMPILER__MICROSOFT
	#pragma warning(push)
		#pragma warning(disable:4702) // unreachable code (was for msvc7.1, which still has problems with its own implementation of the c++ standard library)
		#include <stdexcept>
	#pragma warning(pop)
#endif

// c++ headers
#include <algorithm>
//#include <bitset>
//#include <complex>
#include <deque>
#include <exception>
#include <fstream>
#include <functional>
#include <iomanip>
//#include <ios>
//#include <iosfwd>
#include <iostream>
#include <istream>
#include <iterator>
#include <limits>
#include <list>
//#include <locale>
#include <map>
#include <memory>
#include <new>
#include <numeric>
#include <ostream>
#include <queue>
#include <set>
#include <sstream>
#include <stack>
#include <stdexcept>
#include <streambuf>
#include <string>
#include <typeinfo>
//#include <utility>
//#include <valarray>
#include <vector>

// c headers
#include <cassert>
#include <cctype>
#include <cfloat>
//#include <ciso646>
#include <climits>
#include <cerrno>
//#include <clocale>
#include <cmath>
//#include <csetjmp>
//#include <csignal>
//#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
//#include <ctime>
//#include <cwchar>
//#include <cwctype>

#if defined DIVERSALIS__COMPILER__MICROSOFT
	#pragma message("packageneric::pre_compiled:: done parsing standard c++ headers")
#endif



/////////
// posix
/////////

#if defined DIVERSALIS__OPERATING_SYSTEM__POSIX
	#if defined DIVERSALIS__COMPILER__MICROSOFT
		#pragma message("packageneric::pre_compiled:: parsing standard posix headers")
	#endif
	#include <sys/unistd.h>
	#if defined DIVERSALIS__COMPILER__MICROSOFT
		#pragma message("packageneric::pre_compiled:: done parsing standard posix headers")
	#endif
#endif



//////////////////////////
// boost http://boost.org
//////////////////////////

#if defined DIVERSALIS__COMPILER__MICROSOFT
	#pragma message("packageneric::pre_compiled:: parsing boost headers")
#endif
#include <boost/static_assert.hpp>
#include <boost/filesystem/path.hpp>
#define BOOST_THREAD_USE_DLL
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/recursive_mutex.hpp>
//#include <boost/thread/read_write_mutex.hpp>
#include <boost/thread/condition.hpp>
// huge include! #include <boost/spirit.hpp>
#if defined DIVERSALIS__COMPILER__MICROSOFT
	#pragma message("packageneric::pre_compiled:: done parsing boost headers")
#endif



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



#if defined DIVERSALIS__COMPILER__MICROSOFT && DIVERSALIS__COMPILER__VERSION__MAJOR < 7
	#pragma warning(pop)
#endif



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



#if defined DIVERSALIS__COMPILER__MICROSOFT
	#pragma message("packageneric::pre_compiled:: done parsing " __FILE__)
#endif



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// end
