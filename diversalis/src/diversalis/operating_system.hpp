// -*- mode:c++; indent-tabs-mode:t -*-
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

	/// apple's macosx darwin mach/bsd kernel, autodetected via __APPLE__ and __MACH__, \see DIVERSALIS__OPERATING_SYSTEM__DARWIN.
	#define DIVERSALIS__OPERATING_SYSTEM__APPLE
	#undef DIVERSALIS__OPERATING_SYSTEM__APPLE // was just defined to insert documentation.

	/// cygwin, autodetected via __CYGWIN__.
	#define DIVERSALIS__OPERATING_SYSTEM__CYGWIN
	#undef DIVERSALIS__OPERATING_SYSTEM__CYGWIN // was just defined to insert documentation.

	/// X Window System.
	/// On apple's macosx operating system,
	/// and on cygwin,
	/// we have to choose whether we want to use the x window system or not.
	/// This is done by testing if DIVERSALIS__OPERATING_SYSTEM__CROSSPLATFORM is defined.
	#define DIVERSALIS__OPERATING_SYSTEM__X_WINDOW
	#undef DIVERSALIS__OPERATING_SYSTEM__X_WINDOW // was just defined to insert documentation.

	/// microsoft's windows, autodetected via _WIN64 and _WIN32 (and __MINGW32__).
	///
	/// ?.? is vista\n
	/// ?.? is longhorn\n
	/// ?.? is 2003\n
	/// 5.1 is xp\n
	/// 5.0 is 2000\n
	/// 4.1 is 1998 nt or msdos!\n
	/// 4.0 is 1995 nt or msdos!\n
	/// 3.5 is nt\n
	///
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

	///\name operating system version ... microsoft's branch mess
	///\{
		/// indicates the target microsoft's windows operating system branch is nt.
		#define DIVERSALIS__OPERATING_SYSTEM__BRANCH__NT
		#undef DIVERSALIS__OPERATING_SYSTEM__BRANCH__NT // was just defined to insert documentation.
		/// indicates the target microsoft's windows operating system branch is msdos.
		#define DIVERSALIS__OPERATING_SYSTEM__BRANCH__DOS
		#undef DIVERSALIS__OPERATING_SYSTEM__BRANCH__DOS // was just defined to insert documentation.
	///\}

	/// If defined, prefers to use crossplatforms libraries when choice is available.
	/// For examples, we may use:\n
	/// - glib or boost instead of the operating system proprietary/specific api directly.\n
	/// - the x window system on apple's macosx and cygwin instead of the operating system proprietary/specific gui api directly.\n
	#define DIVERSALIS__OPERATING_SYSTEM__UNIVERSALIS
	#undef DIVERSALIS__OPERATING_SYSTEM__UNIVERSALIS // was just defined to insert documentation.

	/// The opposite of DIVERSALIS__OPERATING_SYSTEM__UNIVERSALIS
	#define DIVERSALIS__OPERATING_SYSTEM__QUAQUAVERSALIS
	#undef DIVERSALIS__OPERATING_SYSTEM__QUAQUAVERSALIS // was just defined to insert documentation.
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
	#define DIVERSALIS__OPERATING_SYSTEM__BSD__FREE

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
	//////////////////
	// WINVER
	// _WIN32_WINDOWS
	// _WIN32_NT
	//////////////////

	// read the configuration

	#if !defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT__BRANCH
		#define DIVERSALIS__OPERATING_SYSTEM__MICROSOFT__BRANCH__NT
	#endif
	// patch => minor
	#if defined DIVERSALIS__OPERATING_SYSTEM__VERSION__PATCH
		#if !defined DIVERSALIS__OPERATING_SYSTEM__VERSION__MINOR
			#error "if you set the patch number, you must also set the minor number."
		#endif
	#endif
	// minor => major
	#if defined DIVERSALIS__OPERATING_SYSTEM__VERSION__MINOR
		#if !defined DIVERSALIS__OPERATING_SYSTEM__VERSION__MAJOR
			#error "if you set the minor number, you must also set the major number."
		#endif
	#endif
	// set version 5 by default
	#if !defined DIVERSALIS__OPERATING_SYSTEM__VERSION__MAJOR
		#define DIVERSALIS__OPERATING_SYSTEM__VERSION__MAJOR 5
	#endif

	// set the compatibility selection macro accordingly

	#if defined DIVERSALIS__OPERATING_SYSTEM__VERSION__MAJOR
		#if !defined DIVERSALIS__OPERATING_SYSTEM__VERSION__MINOR
			#define DIVERSALIS__OPERATING_SYSTEM__VERSION__MINOR 0
		#endif

		#if !defined DIVERSALIS__OPERATING_SYSTEM__VERSION__PATCH
			#define DIVERSALIS__OPERATING_SYSTEM__VERSION__PATCH 0
		#endif

		#define DIVERSALIS__OPERATING_SYSTEM__VERSION__WANTED_RAW \
			( \
				DIVERSALIS__OPERATING_SYSTEM__VERSION__MAJOR * 0x100 + \
				DIVERSALIS__OPERATING_SYSTEM__VERSION__MINOR * 0x10 + \
				DIVERSALIS__OPERATING_SYSTEM__VERSION__PATCH \
			)

		#if !defined WINVER
			#define WINVER DIVERSALIS__OPERATING_SYSTEM__VERSION__WANTED_RAW
		#elif WINVER < DIVERSALIS__OPERATING_SYSTEM__VERSION__WANTED_RAW
			#error "WINVER too low."
		#endif

		#if !defined _WIN32_WINDOWS
			#define _WIN32_WINDOWS WINVER
		#elif _WIN32_WINDOWS < DIVERSALIS__OPERATING_SYSTEM__VERSION__WANTED_RAW
			#error "_WIN32_WINDOWS too low."
		#endif

		#if !defined DIVERSALIS__OPERATING_SYSTEM__BRANCH && defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT__BRANCH__NT
			#define DIVERSALIS__OPERATING_SYSTEM__BRANCH
			#define DIVERSALIS__OPERATING_SYSTEM__BRANCH__NT
		#endif

		#if 1 || defined DIVERSALIS__OPERATING_SYSTEM__BRANCH__NT
			#if !defined _WIN32_WINNT
				#define _WIN32_WINNT WINVER
			#elif _WIN32_WINNT < DIVERSALIS__OPERATING_SYSTEM__VERSION__WANTED_RAW
				#error "_WIN32_WINNT too low."
			#endif
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

// operating systems not yet having a X Window System by default
#if \
	!defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT && \
	!defined DIVERSALIS__OPERATING_SYSTEM__APPLE
	#define DIVERSALIS__OPERATING_SYSTEM__X_WINDOW
#endif

#endif // !defined DIVERSALIS__OPERATING_SYSTEM__INCLUDED
