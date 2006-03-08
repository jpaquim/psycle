///\file
///\brief project-wide operating system specific tweaks.
///\meta generic
#pragma once
#include INCLUDE(PROJECT/project.hpp)

#if defined COMPILER__DOXYGEN
	/// mach kernel.
	#define OPERATING_SYSTEM__MACH
	#undef OPERATING_SYSTEM__MACH // was just defined to insert documentation.   

	/// hurd kernel, \see OPERATING_SYSTEM__MACH.
	#define OPERATING_SYSTEM__HURD
	#undef OPERATING_SYSTEM__HURD // was just defined to insert documentation.
	
	/// linux kernel, autodetected via __linux__.
	/// version 2.6.7
	#define OPERATING_SYSTEM__LINUX
	#undef OPERATING_SYSTEM__LINUX // was just defined to insert documentation.

	/// bsd kernel, autodetected for apple's darwin/macos mach/bsd kernel via __STRICT_BSD__, \see OPERATING_SYSTEM__DARWIN.
	#define OPERATING_SYSTEM__BSD
	#undef OPERATING_SYSTEM__BSD // was just defined to insert documentation.

	/// darwin/mach kernel, \see OPERATING_SYSTEM__MACH and \see OPERATING_SYSTEM__BSD, and for apple's darwin/macos \see OPERATING_SYSTEM__APPLE.
	#define OPERATING_SYSTEM__DARWIN
	#undef OPERATING_SYSTEM__DARWIN // was just defined to insert documentation.   

	/// apple's macosx darwin mach/bsd kernel, autodetected via __APPLE__ and __MACH__, \see OPERATING_SYSTEM__DARWIN.
	/// version 10
	#define OPERATING_SYSTEM__APPLE
	#undef OPERATING_SYSTEM__APPLE // was just defined to insert documentation.

	/// microsoft's windows, autodetected via _WIN32 and _WIN64 (and __MINGW32__).
	/// version 5.1.2600 (xp service pack 1).
	/// [bohan]
	/// i don't know what to do with old versions (9x/me)...
	/// it would be cubersome to support such old systems.
	/// are there still many users of these?
	#define OPERATING_SYSTEM__MICROSOFT
	#undef OPERATING_SYSTEM__MICROSOFT // was just defined to insert documentation.

	/// mswind version compatibility selection.\n
	/// can be set to one of:\n
	/// OPERATING_SYSTEM__VERSION__MICROSOFT__COMPATIBILITY__XP\n
	/// OPERATING_SYSTEM__VERSION__MICROSOFT__COMPATIBILITY__2000\n
	/// OPERATING_SYSTEM__VERSION__MICROSOFT__COMPATIBILITY__1998\n
	#define OPERATING_SYSTEM__MICROSOFT__COMPATIBILITY
	#undef OPERATING_SYSTEM__MICROSOFT__COMPATIBILITY // was just defined to insert documentation.

	/// operating system version, major number.
	#define OPERATING_SYSTEM__VERSION__MAJOR
	#undef OPERATING_SYSTEM__VERSION__MAJOR // was just defined to insert documentation.
	/// operating system version, minor number.
	#define OPERATING_SYSTEM__VERSION__MINOR
	#undef OPERATING_SYSTEM__VERSION__MINOR // was just defined to insert documentation.
	/// operating system version, patch number.
	#define OPERATING_SYSTEM__VERSION__PATCH
	#undef OPERATING_SYSTEM__VERSION__PATCH // was just defined to insert documentation.

	/// If defined, prefers to use crossplatforms libraries when choice is available.
	/// For examples, we may use:\n
	/// - glib or boost instead of the operating system proprietary/specific api directly.\n
	/// - the x window system on apple's macosx and cygwin instead of the operating system proprietary/specific gui api directly.\n
	#define OPERATING_SYSTEM__CROSSPLATFORM
	#undef OPERATING_SYSTEM__CROSSPLATFORM // was just defined to insert documentation.

	/// the posix standard, autodetermined.
	/// version 2
	#define OPERATING_SYSTEM__POSIX
	#undef OPERATING_SYSTEM__POSIX // was just defined to insert documentation.
	
	/// gnu operating system, autodetermined.
	#define OPERATING_SYSTEM__GNU
	#undef OPERATING_SYSTEM__GNU // was just defined to insert documentation.

	/// X Window System.
	/// On apple's macosx operating system,
	/// and on cygwin,
	/// we have to choose whether we want to use the x window system or not.
	/// This is done by testing if OPERATING_SYSTEM__CROSSPLATFORM is defined.
	#define OPERATING_SYSTEM__X_WINDOW
	#undef OPERATING_SYSTEM__X_WINDOW // was just defined to insert documentation.
#endif

#if defined __linux__
	#define OPERATING_SYSTEM
	#define OPERATING_SYSTEM__LINUX
	#define OPERATING_SYSTEM__VERSION__MAJOR 2
	#define OPERATING_SYSTEM__VERSION__MINOR 6
	#define OPERATING_SYSTEM__VERSION__PATCH 8
#elif defined __APPLE__ && defined __MACH__
	#define OPERATING_SYSTEM
	#define OPERATING_SYSTEM__MACH
	#define OPERATING_SYSTEM__DARWIN
	#define OPERATING_SYSTEM__VERSION__MAJOR 7
	#define OPERATING_SYSTEM__VERSION__MINOR 7
	#define OPERATING_SYSTEM__VERSION__PATCH 0
	#define OPERATING_SYSTEM__APPLE
	#define OPERATING_SYSTEM__VERSION__EXTRA_LAYER__MAJOR 10
	#define OPERATING_SYSTEM__VERSION__EXTRA_LAYER__MINOR 3
	#define OPERATING_SYSTEM__VERSION__EXTRA_LAYER__PATCH 3
	#if defined __STRICT_BSD__
		#define OPERATING_SYSTEM__BSD
	#endif
#elif defined __MINGW32__ || defined _WIN32 || defined _WIN64
	#if defined _WIN64
		#error "These sources have never been tested on the 64-bit version of microsoft's operating system ; nevertheless, you may edit the file where this error is triggered to force compilation and test if it works."
	#else
		// [bohan] i don't know what to do with old msdos-based versions (9x/me)... are there still many users of these?
		#define OPERATING_SYSTEM
		#define OPERATING_SYSTEM__MICROSOFT
		#define OPERATING_SYSTEM__VERSION__MAJOR 5 // >= 2k
		#define OPERATING_SYSTEM__VERSION__MINOR 0 // == 2k
		//#define OPERATING_SYSTEM__VERSION__MINOR 1 // == xp
		#define OPERATING_SYSTEM__VERSION__PATCH 2600 // xp service pack 1
		#if defined COMPILER__RESOURCE //&& defined COMPILER__MICROSOFT
			// msvc7.1's resource compiler freaks out: warning RC4011: identifier truncated to 'OPERATING_SYSTEM__VERSION__MICR'
		#else
			#define OPERATING_SYSTEM__VERSION__MICROSOFT__COMPATIBILITY__XP 0x501
			#define OPERATING_SYSTEM__VERSION__MICROSOFT__COMPATIBILITY__2000 0x500
			#define OPERATING_SYSTEM__VERSION__MICROSOFT__COMPATIBILITY__1998 0x410
			#if defined OPERATING_SYSTEM__VERSION__MICROSOFT__COMPATIBILITY
				#define WINVER OPERATING_SYSTEM__VERSION__MICROSOFT__COMPATIBILITY
				#define _WIN32_WINDOWS WINVER
				#define _WIN32_WINNT WINVER
				#define _WIN32_IE WINVER
			#endif
		#endif
		/// microsoft's #define MAX_PATH has too low value for ntfs
		#define OPERATING_SYSTEM__MICROSOFT__MAX_PATH (MAX_PATH < (1 << 12) ? (1 << 12) : MAX_PATH)
	#endif
#else
	/// We don't know the exact operating system, but we'll try to compile anyway, using crossplatform libraries.
	#define OPERATING_SYSTEM
	#define OPERATING_SYSTEM__CROSSPLATFORM
#endif

#if !defined OPERATING_SYSTEM
	#error "Unkown operating system."
#endif

// operating system kernels for gnu operating system applications
#if defined OPERATING_SYSTEM__HURD || defined OPERATING_SYSTEM__LINUX
	#define OPERATING_SYSTEM__GNU
#endif

// operating systems following the posix standard
#if defined OPERATING_SYSTEM__GNU || defined OPERATING_SYSTEM__BSD
	#define OPERATING_SYSTEM__POSIX
#endif

#if defined OPERATING_SYSTEM__CROSSPLATFORM || !(defined OPERATING_SYSTEM__MICROSOFT || defined OPERATING_SYSTEM__APPLE)
	#define OPERATING_SYSTEM__X_WINDOW
#endif
