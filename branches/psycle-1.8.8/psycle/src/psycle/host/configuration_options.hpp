///\file
///\brief configuration of the build.
#pragma once
/// JAZ: Define to 1 to enable the volume column for XMSampler. It will also make the machine column in the pattern to show
///      the values of the volume column instead.
#define PSYCLE__CONFIGURATION__VOLUME_COLUMN 0

/// Test for RMS Vu's
#define PSYCLE__CONFIGURATION__RMS_VUS 1

/// unmasks fpu exceptions
#define PSYCLE__CONFIGURATION__FPU_EXCEPTIONS 0


/// string describing the configuration of the build.
#define PSYCLE__CONFIGURATION(EOL) \
	"compiler build tool chain = " DIVERSALIS__COMPILER__VERSION__STRING EOL \
	"fpu exceptions = " UNIVERSALIS__COMPILER__STRINGIZE(PSYCLE__CONFIGURATION__FPU_EXCEPTIONS) EOL \
	"volume column = " UNIVERSALIS__COMPILER__STRINGIZE(PSYCLE__CONFIGURATION__VOLUME_COLUMN) EOL \
	"rms vu = " UNIVERSALIS__COMPILER__STRINGIZE(PSYCLE__CONFIGURATION__RMS_VUS) EOL \
	"debugging = " PSYCLE__CONFIGURATION__DEBUG

/// value to show in the string describing the configuration of the build.
#if defined NDEBUG
	#define PSYCLE__CONFIGURATION__DEBUG "off"
#else
	#define PSYCLE__CONFIGURATION__DEBUG "on"
#endif
#include <universalis/compiler/stringize.hpp> // for UNIVERSALIS__COMPILER__STRINGIZE
