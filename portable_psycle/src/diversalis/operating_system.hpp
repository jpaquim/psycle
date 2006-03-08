// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// Copyright (C) 1999-2005 Psycledelics http://psycle.pastnotecut.org : Johan Boule

///\file
///\brief project-wide operating system specific tweaks.
///\meta generic
#ifndef DIVERSALIS__OPERATING_SYSTEM__INCLUDED
#define DIVERSALIS__OPERATING_SYSTEM__INCLUDED
#pragma once
#include "compiler.hpp"
//#namespace DIVERSALIS
	//#namespace OPERATING_SYSTEM



		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// documentation about what is defined by this file



		#if defined DIVERSALIS__COMPILER__DOXYGEN
			/// unix, autodetected via __unix__.
			#define DIVERSALIS__OPERATING_SYSTEM__UNIX
			#undef DIVERSALIS__OPERATING_SYSTEM__UNIX // was just defined to insert documentation.

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

			/// microsoft's windows, autodetected via _WIN32 and _WIN64 (and __MINGW32__).
			///
			/// ?.? is longhorn\n
			/// ?.? is 2003\n
			/// 5.1 is xp\n
			/// 5.0 is 2000\n
			/// 4.1 is 1998\n
			/// 4.0 is 1995\n
			///
			/// note: on cygwin, DIVERSALIS__OPERATING_SYSTEM__MICROSOFT is not defined, as it's considered to be in the unix family.
			#define DIVERSALIS__OPERATING_SYSTEM__MICROSOFT
			#undef DIVERSALIS__OPERATING_SYSTEM__MICROSOFT // was just defined to insert documentation.

			/// operating system version, major number.
			#define DIVERSALIS__OPERATING_SYSTEM__VERSION__MAJOR
			#undef DIVERSALIS__OPERATING_SYSTEM__VERSION__MAJOR // was just defined to insert documentation.
			/// operating system version, minor number.
			#define DIVERSALIS__OPERATING_SYSTEM__VERSION__MINOR
			#undef DIVERSALIS__OPERATING_SYSTEM__VERSION__MINOR // was just defined to insert documentation.
			/// operating system version, patch number.
			#define DIVERSALIS__OPERATING_SYSTEM__VERSION__PATCH
			#undef DIVERSALIS__OPERATING_SYSTEM__VERSION__PATCH // was just defined to insert documentation.

			/// If defined, prefers to use crossplatforms libraries when choice is available.
			/// For examples, we may use:\n
			/// - glib or boost instead of the operating system proprietary/specific api directly.\n
			/// - the x window system on apple's macosx and cygwin instead of the operating system proprietary/specific gui api directly.\n
			#define DIVERSALIS__OPERATING_SYSTEM__UNIVERSALIS
			#undef DIVERSALIS__OPERATING_SYSTEM__UNIVERSALIS // was just defined to insert documentation.

			/// the posix standard, autodetermined.
			#define DIVERSALIS__OPERATING_SYSTEM__POSIX
			#undef DIVERSALIS__OPERATING_SYSTEM__POSIX // was just defined to insert documentation.
			
			/// gnu operating system, autodetermined.
			#define DIVERSALIS__OPERATING_SYSTEM__GNU
			#undef DIVERSALIS__OPERATING_SYSTEM__GNU // was just defined to insert documentation.

			/// X Window System.
			/// On apple's macosx operating system,
			/// and on cygwin,
			/// we have to choose whether we want to use the x window system or not.
			/// This is done by testing if DIVERSALIS__OPERATING_SYSTEM__CROSSPLATFORM is defined.
			#define DIVERSALIS__OPERATING_SYSTEM__X_WINDOW
			#undef DIVERSALIS__OPERATING_SYSTEM__X_WINDOW // was just defined to insert documentation.
		#endif

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// now the real work


		////////
		// unix
		////////

		#if defined __unix__
			#define DIVERSALIS__OPERATING_SYSTEM
			#define DIVERSALIS__OPERATING_SYSTEM__UNIX
		#endif

		/////////
		// linux
		/////////

		#if defined __linux__
			#define DIVERSALIS__OPERATING_SYSTEM
			#define DIVERSALIS__OPERATING_SYSTEM__UNIX
			#define DIVERSALIS__OPERATING_SYSTEM__LINUX
			#define DIVERSALIS__OPERATING_SYSTEM__VERSION__MAJOR 2
			#define DIVERSALIS__OPERATING_SYSTEM__VERSION__MINOR 6
			#define DIVERSALIS__OPERATING_SYSTEM__VERSION__PATCH 8

		////////
		// *bsd
		////////

		#elif defined __FreeBSD__
			#define DIVERSALIS__OPERATING_SYSTEM
			#define DIVERSALIS__OPERATING_SYSTEM__UNIX
			#define DIVERSALIS__OPERATING_SYSTEM__BSD
			#define DIVERSALIS__OPERATING_SYSTEM__BSD__FREE
			#define DIVERSALIS__OPERATING_SYSTEM__VERSION__MAJOR 0
			#define DIVERSALIS__OPERATING_SYSTEM__VERSION__MINOR 0
			#define DIVERSALIS__OPERATING_SYSTEM__VERSION__PATCH 0

		////////////////////////
		// darwin/apple's macos
		////////////////////////

		#elif defined __APPLE__ && defined __MACH__
			#define DIVERSALIS__OPERATING_SYSTEM
			#define DIVERSALIS__OPERATING_SYSTEM__UNIX
			#if defined __STRICT_BSD__
				#define DIVERSALIS__OPERATING_SYSTEM__BSD
			#endif
			#define DIVERSALIS__OPERATING_SYSTEM__MACH
			#define DIVERSALIS__OPERATING_SYSTEM__DARWIN
			#define DIVERSALIS__OPERATING_SYSTEM__VERSION__MAJOR 7
			#define DIVERSALIS__OPERATING_SYSTEM__VERSION__MINOR 7
			#define DIVERSALIS__OPERATING_SYSTEM__VERSION__PATCH 0
			#define DIVERSALIS__OPERATING_SYSTEM__APPLE
			#define DIVERSALIS__OPERATING_SYSTEM__VERSION__EXTRA_LAYER__MAJOR 10
			#define DIVERSALIS__OPERATING_SYSTEM__VERSION__EXTRA_LAYER__MINOR 3
			#define DIVERSALIS__OPERATING_SYSTEM__VERSION__EXTRA_LAYER__PATCH 3

		//////////
		// cygwin
		//////////

		#elif defined __CYGWIN__
			#define DIVERSALIS__OPERATING_SYSTEM
			#define DIVERSALIS__OPERATING_SYSTEM__UNIX
			#define DIVERSALIS__OPERATING_SYSTEM__CYGWIN
			#define DIVERSALIS__OPERATING_SYSTEM__VERSION__MAJOR 1
			#define DIVERSALIS__OPERATING_SYSTEM__VERSION__MINOR 0
			#define DIVERSALIS__OPERATING_SYSTEM__VERSION__PATCH 0

		///////////////////////
		// microsoft's windows
		///////////////////////

		#elif defined __MINGW32__ || defined _WIN32 || defined _WIN64
			#if defined _WIN64
				#error "These sources have never been tested on the 64-bit version of microsoft's operating system ; nevertheless, you may edit the file where this error is triggered to force compilation and test if it works, or else, as a last resort, you may enable the 32-bit compatibility mode (WOW library)."
			#endif
			#define DIVERSALIS__OPERATING_SYSTEM
			#define DIVERSALIS__OPERATING_SYSTEM__MICROSOFT
			#if defined _WINDOWS_ || defined WINVER || defined _WIN32_WINDOWS || defined _WIN32_WINNT || defined _WIN32_IE
				#error "Please #include <diversalis/operating_system.hpp> before using any mswindows-specific code."
			#endif
			#if defined DIVERSALIS__COMPILER__RESOURCE //&& defined DIVERSALIS__COMPILER__MICROSOFT
				// msvc7.1's resource compiler freaks out: warning RC4011: identifier truncated to 'DIVERSALIS__OPERATING_SYSTEM__XXXXXXXXXXXXX'
			#else
				/////////////////////////////////////
				// WINVER
				// _WIN32_WINDOWS
				// _WIN32_NT
				/////////////////////////////////////
					// read the configuration
						// patch => minor
						#if		 defined DIVERSALIS__PACKAGENERIC__CONFIGURATION__OPERATING_SYSTEM__VERSION__PATCH
							#if	!defined DIVERSALIS__PACKAGENERIC__CONFIGURATION__OPERATING_SYSTEM__VERSION__MINOR
								#error "if you set the patch number, you must also set the minor number."
							#endif
						#endif
						// minor => major
						#if		 defined DIVERSALIS__PACKAGENERIC__CONFIGURATION__OPERATING_SYSTEM__VERSION__MINOR
							#if	!defined DIVERSALIS__PACKAGENERIC__CONFIGURATION__OPERATING_SYSTEM__VERSION__MAJOR
								#error "if you set the minor number, you must also set the major number."
							#endif
						#endif
					// set the compatibility selection macro accordingly
						#if defined DIVERSALIS__PACKAGENERIC__CONFIGURATION__OPERATING_SYSTEM__VERSION__MAJOR
						      #define                            DIVERSALIS__OPERATING_SYSTEM__VERSION__MAJOR              DIVERSALIS__PACKAGENERIC__CONFIGURATION__OPERATING_SYSTEM__VERSION__MAJOR
						      #if defined       DIVERSALIS__PACKAGENERIC__CONFIGURATION__OPERATING_SYSTEM__VERSION__MINOR
						            #define                                  DIVERSALIS__OPERATING_SYSTEM__VERSION__MINOR  DIVERSALIS__PACKAGENERIC__CONFIGURATION__OPERATING_SYSTEM__VERSION__MINOR
						            #if defined DIVERSALIS__PACKAGENERIC__CONFIGURATION__OPERATING_SYSTEM__VERSION__PATCH
						                  #define                            DIVERSALIS__OPERATING_SYSTEM__VERSION__PATCH  DIVERSALIS__PACKAGENERIC__CONFIGURATION__OPERATING_SYSTEM__VERSION__PATCH
						                  #define WINVER \
										( \
											DIVERSALIS__OPERATING_SYSTEM__VERSION__MAJOR * 0x100 + \
											DIVERSALIS__OPERATING_SYSTEM__VERSION__MINOR * 0x10 + \
											DIVERSALIS__OPERATING_SYSTEM__VERSION__PATCH \
										)
						            #else
						                  #define WINVER \
										( \
											DIVERSALIS__OPERATING_SYSTEM__VERSION__MAJOR * 0x100 + \
											DIVERSALIS__OPERATING_SYSTEM__VERSION__MINOR * 0x10 \
										)
						            #endif
						      #else
						                  #define WINVER \
										( \
											DIVERSALIS__OPERATING_SYSTEM__VERSION__MAJOR * 0x100 \
										)
						      #endif
						      #define _WIN32_WINDOWS WINVER
						      #define _WIN32_WINNT   WINVER
						#endif
				////////////////////////////////////////////
				// _WIN32_IE (internet explorer extra layer)
				////////////////////////////////////////////
					// read the configuration
						// patch => minor
						#if		 defined DIVERSALIS__CONFIGURATION__OPERATING_SYSTEM__VERSION__EXTRA_LAYER__INTERNET_EXPLORER__PATCH
							#if	!defined DIVERSALIS__CONFIGURATION__OPERATING_SYSTEM__VERSION__EXTRA_LAYER__INTERNET_EXPLORER__MINOR
								#error "if you set the patch number, you must also set the minor number."
							#endif
						#endif
						// minor => major
						#if		 defined DIVERSALIS__CONFIGURATION__OPERATING_SYSTEM__VERSION__EXTRA_LAYER__INTERNET_EXPLORER__MINOR
							#if	!defined DIVERSALIS__CONFIGURATION__OPERATING_SYSTEM__VERSION__EXTRA_LAYER__INTERNET_EXPLORER__MAJOR
								#error "if you set the minor number, you must also set the major number."
							#endif
						#endif
					// set the compatibility selection macro accordingly
						#if defined             DIVERSALIS__PACKAGENERIC__CONFIGURATION__OPERATING_SYSTEM__VERSION__EXTRA_LAYER__INTERNET_EXPLORER__MAJOR
						      #define                                        DIVERSALIS__OPERATING_SYSTEM__VERSION__EXTRA_LAYER__INTERNET_EXPLORER__MAJOR  DIVERSALIS__PACKAGENERIC__CONFIGURATION__OPERATING_SYSTEM__VERSION__EXTRA_LAYER__INTERNET_EXPLORER__MAJOR
						      #if defined       DIVERSALIS__PACKAGENERIC__CONFIGURATION__OPERATING_SYSTEM__VERSION__EXTRA_LAYER__INTERNET_EXPLORER__MINOR
						            #define                                  DIVERSALIS__OPERATING_SYSTEM__VERSION__EXTRA_LAYER__INTERNET_EXPLORER__MINOR  DIVERSALIS__PACKAGENERIC__CONFIGURATION__OPERATING_SYSTEM__VERSION__EXTRA_LAYER__INTERNET_EXPLORER__MINOR
						            #if defined DIVERSALIS__PACKAGENERIC__CONFIGURATION__OPERATING_SYSTEM__VERSION__EXTRA_LAYER__INTERNET_EXPLORER__PATCH
						                  #define                            DIVERSALIS__OPERATING_SYSTEM__VERSION__EXTRA_LAYER__INTERNET_EXPLORER__PATCH  DIVERSALIS__PACKAGENERIC__CONFIGURATION__OPERATING_SYSTEM__VERSION__EXTRA_LAYER__INTERNET_EXPLORER__PATCH
						                  #define _WIN32_IE \
										( \
											DIVERSALIS__OPERATING_SYSTEM__VERSION__EXTRA_LAYER__INTERNET_EXPLORER__MAJOR * 0x100 + \
											DIVERSALIS__OPERATING_SYSTEM__VERSION__EXTRA_LAYER__INTERNET_EXPLORER__MINOR * 0x10 + \
											DIVERSALIS__OPERATING_SYSTEM__VERSION__EXTRA_LAYER__INTERNET_EXPLORER__PATCH \
										)
						            #else
						                  #define _WIN32_IE \
										( \
											DIVERSALIS__OPERATING_SYSTEM__VERSION__EXTRA_LAYER__INTERNET_EXPLORER__MAJOR * 0x100 + \
											DIVERSALIS__OPERATING_SYSTEM__VERSION__EXTRA_LAYER__INTERNET_EXPLORER__MINOR * 0x10 \
										)
						            #endif
						      #else
						                  #define _WIN32_IE \
										( \
											DIVERSALIS__OPERATING_SYSTEM__VERSION__EXTRA_LAYER__INTERNET_EXPLORER__MAJOR * 0x100 \
										)
						      #endif
						#endif
			#endif



		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// consistency check



		#else
			/// We don't know the exact operating system, but we'll try to compile anyway, using crossplatform libraries.
			#define DIVERSALIS__OPERATING_SYSTEM
			#define DIVERSALIS__UNIVERSALIS
		#endif

		#if !defined DIVERSALIS__OPERATING_SYSTEM
			#error "Unkown operating system."
		#endif



		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
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
			defined DIVERSALIS__OPERATING_SYSTEM__BSD
			#define DIVERSALIS__OPERATING_SYSTEM__POSIX
		#endif

		// operating systems not yet having a X Window System by default
		#if \
			!defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT && \
			!defined DIVERSALIS__OPERATING_SYSTEM__APPLE
			#define DIVERSALIS__OPERATING_SYSTEM__X_WINDOW
		#endif



	//#endnamespace
//#endnamespace
#endif
// arch-tag: 848dec49-971c-40b0-a980-3367ebe4dcf6
