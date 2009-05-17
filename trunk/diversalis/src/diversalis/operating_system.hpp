// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2008 members of the psycle project http://psycle.pastnotecut.org ; johan boule <bohan@jabber.org>

///\file \brief project-wide operating system specific tweaks.

#ifndef DIVERSALIS__OPERATING_SYSTEM__INCLUDED
#define DIVERSALIS__OPERATING_SYSTEM__INCLUDED
#pragma once

#include "compiler.hpp"

#if defined DIVERSALIS__COMPILER__DOXYGEN

	/**********************************************************************************/
	// documentation about what is defined in this file

	/// unix, autodetected via __unix__.
	#define DIVERSALIS__OPERATING_SYSTEM__UNIX
	#undef DIVERSALIS__OPERATING_SYSTEM__UNIX // was just defined to insert documentation.

	/// the posix standard, autodetected.
	#define DIVERSALIS__OPERATING_SYSTEM__POSIX
	#undef DIVERSALIS__OPERATING_SYSTEM__POSIX // was just defined to insert documentation.

	/// gnu operating system, autodetected.
	#define DIVERSALIS__OPERATING_SYSTEM__GNU
	#undef DIVERSALIS__OPERATING_SYSTEM__GNU // was just defined to insert documentation.

	/// mach kernel.
	#define DIVERSALIS__OPERATING_SYSTEM__MACH
	#undef DIVERSALIS__OPERATING_SYSTEM__MACH // was just defined to insert documentation.

	/// hurd kernel, \see DIVERSALIS__OPERATING_SYSTEM__MACH.
	#define DIVERSALIS__OPERATING_SYSTEM__HURD
	#undef DIVERSALIS__OPERATING_SYSTEM__HURD // was just defined to insert documentation.

	/// linux kernel, autodetected via __linux__.
	#define DIVERSALIS__OPERATING_SYSTEM__LINUX
	#undef DIVERSALIS__OPERATING_SYSTEM__LINUX // was just defined to insert documentation.

	/// bsd kernel, autodetected for apple's darwin/macos mach/bsd kernel via __STRICT_BSD__, \see DIVERSALIS__OPERATING_SYSTEM__DARWIN.
	#define DIVERSALIS__OPERATING_SYSTEM__BSD
	#undef DIVERSALIS__OPERATING_SYSTEM__BSD // was just defined to insert documentation.

	/// darwin/mach kernel, \see DIVERSALIS__OPERATING_SYSTEM__MACH and \see DIVERSALIS__OPERATING_SYSTEM__BSD, and for apple's darwin/macos \see DIVERSALIS__OPERATING_SYSTEM__APPLE.
	#define DIVERSALIS__OPERATING_SYSTEM__DARWIN
	#undef DIVERSALIS__OPERATING_SYSTEM__DARWIN // was just defined to insert documentation.

	/// apple's macosx darwin mach/bsd kernel. \see DIVERSALIS__OPERATING_SYSTEM__DARWIN.
	#define DIVERSALIS__OPERATING_SYSTEM__APPLE
	#undef DIVERSALIS__OPERATING_SYSTEM__APPLE // was just defined to insert documentation.

	/// cygwin, autodetected via __CYGWIN__.
	#define DIVERSALIS__OPERATING_SYSTEM__CYGWIN
	#undef DIVERSALIS__OPERATING_SYSTEM__CYGWIN // was just defined to insert documentation.

	/// microsoft's windows.
	/// note: on cygwin, DIVERSALIS__OPERATING_SYSTEM__MICROSOFT is not defined, as it's considered to be in the unix family.
	#define DIVERSALIS__OPERATING_SYSTEM__MICROSOFT
	#undef DIVERSALIS__OPERATING_SYSTEM__MICROSOFT // was just defined to insert documentation.

	///\name operating system version
	///\{
		/// operating system version, as an integral number.
		/// This combines the major, minor and patch numbers into a single integral number.
		#define DIVERSALIS__OPERATING_SYSTEM__VERSION
		#undef DIVERSALIS__OPERATING_SYSTEM__VERSION // was just defined to insert documentation.
		/// operating system version, major number.
		#define DIVERSALIS__OPERATING_SYSTEM__VERSION__MAJOR
		#undef DIVERSALIS__OPERATING_SYSTEM__VERSION__MAJOR // was just defined to insert documentation.
		/// operating system version, minor number.
		#define DIVERSALIS__OPERATING_SYSTEM__VERSION__MINOR
		#undef DIVERSALIS__OPERATING_SYSTEM__VERSION__MINOR // was just defined to insert documentation.
		/// operating system version, patch number.
		#define DIVERSALIS__OPERATING_SYSTEM__VERSION__PATCH
		#undef DIVERSALIS__OPERATING_SYSTEM__VERSION__PATCH // was just defined to insert documentation.
	///\}

#endif



/**********************************************************************************/
// now the real work


////////
// unix

#if defined __unix__
	#define DIVERSALIS__OPERATING_SYSTEM
	#define DIVERSALIS__OPERATING_SYSTEM__UNIX
#endif

/////////
// linux

#if defined __linux__
	#define DIVERSALIS__OPERATING_SYSTEM
	#define DIVERSALIS__OPERATING_SYSTEM__UNIX
	#define DIVERSALIS__OPERATING_SYSTEM__LINUX

////////
// *bsd

#elif defined __FreeBSD__ // todo netbsd and openbsd
	#define DIVERSALIS__OPERATING_SYSTEM
	#define DIVERSALIS__OPERATING_SYSTEM__UNIX
	#define DIVERSALIS__OPERATING_SYSTEM__BSD
	#define DIVERSALIS__OPERATING_SYSTEM__FREEBSD

////////////////////////
// darwin/apple's macos

#elif defined __APPLE__ && defined __MACH__
	#define DIVERSALIS__OPERATING_SYSTEM
	#define DIVERSALIS__OPERATING_SYSTEM__UNIX
	#if defined __STRICT_BSD__
		#define DIVERSALIS__OPERATING_SYSTEM__BSD
	#endif
	#define DIVERSALIS__OPERATING_SYSTEM__MACH
	#define DIVERSALIS__OPERATING_SYSTEM__DARWIN
	#define DIVERSALIS__OPERATING_SYSTEM__APPLE

////////////
// sgi irix

#elif defined __sgi
	#define DIVERSALIS__OPERATING_SYSTEM
	#define DIVERSALIS__OPERATING_SYSTEM__UNIX
	#define DIVERSALIS__OPERATING_SYSTEM__IRIX

////////
// hpux

#elif defined __hpux
	#define DIVERSALIS__OPERATING_SYSTEM
	#define DIVERSALIS__OPERATING_SYSTEM__UNIX
	#define DIVERSALIS__OPERATING_SYSTEM__HPUX

//////////
// cygwin

#elif defined __CYGWIN__
	#define DIVERSALIS__OPERATING_SYSTEM
	#define DIVERSALIS__OPERATING_SYSTEM__UNIX
	#define DIVERSALIS__OPERATING_SYSTEM__CYGWIN

////////
// msys

#elif defined __MSYS__
	#define DIVERSALIS__OPERATING_SYSTEM
	#define DIVERSALIS__OPERATING_SYSTEM__UNIX
	#define DIVERSALIS__OPERATING_SYSTEM__MSYS

////////
// uwin

#elif defined _UWIN
	#define DIVERSALIS__OPERATING_SYSTEM
	#define DIVERSALIS__OPERATING_SYSTEM__UNIX
	#define DIVERSALIS__OPERATING_SYSTEM__UWIN

///////////////////////
// microsoft's windows

#elif defined _WIN64 || defined _WIN32
	#define DIVERSALIS__OPERATING_SYSTEM
	#define DIVERSALIS__OPERATING_SYSTEM__MICROSOFT
	#define DIVERSALIS__OPERATING_SYSTEM__MICROSOFT__VERSION WINVER

	//////////////////
	// WINVER
	// _WIN32_WINDOWS
	// _WIN32_NT
	//////////////////

	#if defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT__REQUIRED_VERSION
		// WINVER
		#if !defined WINVER
			#define WINVER DIVERSALIS__OPERATING_SYSTEM__MICROSOFT__REQUIRED_VERSION
		#elif WINVER < DIVERSALIS__OPERATING_SYSTEM__MICROSOFT__REQUIRED_VERSION
			#error "WINVER too low."
		#endif
		#if !WINVER
			#error "WINVER is zero."
		#endif

		// _WIN32_WINDOWS
		#if !defined _WIN32_WINDOWS
			#define _WIN32_WINDOWS DIVERSALIS__OPERATING_SYSTEM__MICROSOFT__REQUIRED_VERSION
		#elif _WIN32_WINDOWS < DIVERSALIS__OPERATING_SYSTEM__MICROSOFT__REQUIRED_VERSION
			#error "_WIN32_WINDOWS too low."
		#endif
		#if !_WIN32_WINDOWS
			#error "_WIN32_WINDOWS is zero."
		#endif

		// _WIN32_WINNT
		#if !defined _WIN32_WINNT
			#define _WIN32_WINNT DIVERSALIS__OPERATING_SYSTEM__MICROSOFT__REQUIRED_VERSION
		#elif _WIN32_WINNT < DIVERSALIS__OPERATING_SYSTEM__MICROSOFT__REQUIRED_VERSION
			#error "_WIN32_WINNT too low."
		#endif
		#if !_WIN32_WINNT
			#error "_WIN32_WINNT is zero."
		#endif
	#endif



/**********************************************************************************/
// consistency check



#else
	/// We don't know the exact operating system, but we'll try to compile anyway, using crossplatform libraries.
	#define DIVERSALIS__OPERATING_SYSTEM
	#define DIVERSALIS__OPERATING_SYSTEM__UNIVERSALIS
#endif

#if !defined DIVERSALIS__OPERATING_SYSTEM
	#error "Unkown operating system."
#endif



/**********************************************************************************/
// inferences



// operating system kernels for gnu operating system applications
#if \
	defined DIVERSALIS__OPERATING_SYSTEM__HURD || \
	defined DIVERSALIS__OPERATING_SYSTEM__LINUX || \
	defined DIVERSALIS__OPERATING_SYSTEM__CYGWIN
	#define DIVERSALIS__OPERATING_SYSTEM__GNU
#endif

// operating systems following the posix standard
#if \
	defined DIVERSALIS__OPERATING_SYSTEM__GNU || \
	defined DIVERSALIS__OPERATING_SYSTEM__BSD || \
	defined DIVERSALIS__OPERATING_SYSTEM__MACH
	#define DIVERSALIS__OPERATING_SYSTEM__POSIX
#endif

// operating systems known to have an X Window System "by default"
#if \
	!defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT && \
	!defined DIVERSALIS__OPERATING_SYSTEM__APPLE
	#define DIVERSALIS__OPERATING_SYSTEM__X_WINDOW
#endif

#endif // !defined DIVERSALIS__OPERATING_SYSTEM__INCLUDED
