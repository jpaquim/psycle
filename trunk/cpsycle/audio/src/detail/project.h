///\file
///\brief public project-wide definitions. file included first by every header.
#pragma once
// #include "config.hpp"
#include "../../detail/compiler.h"
#include "../../detail/os.h"
#include "../../detail/stringize.h"


/**************************************************************************************************/
/// string describing the configuration of the build

#define PSYCLE__BUILD__CONFIG(EOL) \
	"Compiler build tool chain = " DIVERSALIS__COMPILER__VERSION__STRING EOL \
	"RMS VU = " DIVERSALIS__STRINGIZE(PSYCLE__CONFIGURATION__RMS_VUS) EOL \
	"Debugging = " PSYCLE__BUILD__CONFIG__DEBUG

	/// value to show in the string describing the configuration of the build.
	#ifdef NDEBUG
		#define PSYCLE__BUILD__CONFIG__DEBUG "off"
	#else
		#define PSYCLE__BUILD__CONFIG__DEBUG "on"
	#endif


