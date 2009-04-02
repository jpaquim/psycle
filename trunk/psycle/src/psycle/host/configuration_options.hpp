///\file
///\brief configuration of the build.
#pragma once
#include <universalis/compiler/stringized.hpp> // to convert a token into a string literal (UNIVERSALIS__COMPILER__STRINGIZED)

/// Define to use psycle-core
///\todo check xml library in psycle-core
#define use_psycore 0

// correctness check: must not be defined to zero
#if defined use_psycore && !use_psycore
	#undef use_psycore
#endif

/// JAZ: Define to 1 to enable the volume column for XMSampler.
///      It will also make the machine column in the pattern to show the values of the volume column instead.
#define PSYCLE__CONFIGURATION__VOLUME_COLUMN 0

/// Test for RMS Vu's
#define PSYCLE__CONFIGURATION__RMS_VUS 0

/// string describing the configuration of the build.
#define PSYCLE__CONFIGURATION(EOL) \
	"compiler build tool chain = " PSYCLE__COMPILER__BUILD EOL \
	"volume column = " UNIVERSALIS__COMPILER__STRINGIZED(PSYCLE__CONFIGURATION__VOLUME_COLUMN) EOL \
	"rms vu = " UNIVERSALIS__COMPILER__STRINGIZED(PSYCLE__CONFIGURATION__RMS_VUS) EOL \
	"psycle-core = " UNIVERSALIS__COMPILER__STRINGIZED(use_psycore) EOL \
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
		#define PSYCLE__COMPILER__BUILD "gcc-" UNIVERSALIS__COMPILER__STRINGIZED(DIVERSALIS__COMPILER__VERSION)
	#elif defined DIVERSALIS__COMPILER__INTEL
		#define PSYCLE__COMPILER__BUILD "icc-" UNIVERSALIS__COMPILER__STRINGIZED(DIVERSALIS__COMPILER__VERSION)
	#elif defined DIVERSALIS__COMPILER__MICROSOFT
		#define PSYCLE__COMPILER__BUILD "msvc-" UNIVERSALIS__COMPILER__STRINGIZED(DIVERSALIS__COMPILER__VERSION)
	#else
		#define PSYCLE__COMPILER__BUILD "unknown"
	#endif
