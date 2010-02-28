///\file
///\brief configuration of the build.
#pragma once

/// Define to 1 to use plugins in the build dir, 0 otherwise.
/// When enabled, psycle will read the env var with the same name (PSYCLE__CONFIGURATION__USE_BUILT_PLUGINS),
/// and if it's set (to any value), it won't read nor save the config file or registry settings regarding the plugin dir.
/// So to really enable this you need to setup your IDE's debug settings to set the env var when launching the process.
/// The advantage of using an env var is that it doesn't hardcode the behaviour in the executable,
/// hence it continues working normally when launched outside the IDE.
///
/// Note 1: Visual sutio saves the debug settings in the .user file,
///         so these don't go in the repository, which is a good thing.
///
/// Note 2: With visual studio, you need to set the env var as
///         "PSYCLE__CONFIGURATION__USE_BUILT_PLUGINS=", that is, with an equal sign at the end,
///         even if there no value after the equal, otherwise the var is *not* set!
#define PSYCLE__CONFIGURATION__USE_BUILT_PLUGINS 1

/// Define to 1 to use psycle-core, 0 otherwise
#define PSYCLE__CONFIGURATION__USE_PSYCORE 1

/// Define to 1 to enable the volume column for XMSampler, 0 otherwise.

/// Define to 1 to use RMS Vu's, 0 otherwise
#define PSYCLE__CONFIGURATION__RMS_VUS 0

///\todo that won't work on big-endian machines (this is for IT and XM module loaders)
#define PSYCLE__CORE__FILEIO__WANT_DEPRECATED_RAW_ACCESS

#if _WIN64
#define BUILD_WITH_BITS " 64 bits"
#else
#define BUILD_WITH_BITS " 32 bits"
#endif
/**************************************************************************************************/
/// string describing the configuration of the build.
#define PSYCLE__CONFIGURATION(EOL) \
	"compiler build tool chain = " PSYCLE__COMPILER__BUILD BUILD_WITH_BITS EOL \
	"rms vu = " UNIVERSALIS__COMPILER__STRINGIZE(PSYCLE__CONFIGURATION__RMS_VUS) EOL \
	"psycle-core = " UNIVERSALIS__COMPILER__STRINGIZE(PSYCLE__CONFIGURATION__USE_PSYCORE) EOL \
	"debugging = " PSYCLE__CONFIGURATION__DEBUG

	/// value to show in the string describing the configuration of the build.
	#if defined NDEBUG
		#define PSYCLE__CONFIGURATION__DEBUG "off"
	#else
		#define PSYCLE__CONFIGURATION__DEBUG "on"
	#endif

	/// value to show in the string describing the configuration of the build.
	/// the compiler used to build.
	#if defined DIVERSALIS__COMPILER__GNU
		#define PSYCLE__COMPILER__BUILD "gcc-" UNIVERSALIS__COMPILER__STRINGIZE(DIVERSALIS__COMPILER__VERSION)
	#elif defined DIVERSALIS__COMPILER__INTEL
		#define PSYCLE__COMPILER__BUILD "icc-" UNIVERSALIS__COMPILER__STRINGIZE(DIVERSALIS__COMPILER__VERSION)
	#elif defined DIVERSALIS__COMPILER__MICROSOFT
		#define PSYCLE__COMPILER__BUILD "msvc-" UNIVERSALIS__COMPILER__STRINGIZE(DIVERSALIS__COMPILER__VERSION)
	#else
		#define PSYCLE__COMPILER__BUILD "unknown"
	#endif

#include <universalis/compiler/stringize.hpp> // for UNIVERSALIS__COMPILER__STRINGIZE
