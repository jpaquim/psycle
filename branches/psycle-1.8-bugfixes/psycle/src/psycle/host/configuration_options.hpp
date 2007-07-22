///\file
///\brief configuration of the build.
#pragma once
#include <universalis/compiler/stringized.hpp> // to convert a token into a string literal (UNIVERSALIS__COMPILER__STRINGIZED)

/// JAZ: Define to 1 to enable the volume column for XMSampler. It will also make the machine column in the pattern to show
///      the values of the volume column instead.
#define PSYCLE__CONFIGURATION__VOLUME_COLUMN 0

/// Test for RMS Vu's
//#define PSYCLE__CONFIGURATION__RMS_VUS 1

/// unmasks fpu exceptions
#define PSYCLE__CONFIGURATION__FPU_EXCEPTIONS 0

/// the compiler used to build ... should be autodetermined, but we don't autoconfigure.
#define PSYCLE__COMPILER__BUILD "msvc"

/// string describing the configuration of the build.
#define PSYCLE__CONFIGURATION(EOL) \
	"compiler build tool chain = " PSYCLE__COMPILER__BUILD EOL \
	"fpu exceptions = " UNIVERSALIS__COMPILER__STRINGIZED(PSYCLE__CONFIGURATION__FPU_EXCEPTIONS) EOL \
	"volume column = " UNIVERSALIS__COMPILER__STRINGIZED(PSYCLE__CONFIGURATION__VOLUME_COLUMN) EOL \
	"debugging = " PSYCLE__CONFIGURATION__DEBUG

/// value to show in the string describing the configuration of the build.
#if defined NDEBUG
	#define PSYCLE__CONFIGURATION__DEBUG "off"
#else
	#define PSYCLE__CONFIGURATION__DEBUG "on"
#endif
