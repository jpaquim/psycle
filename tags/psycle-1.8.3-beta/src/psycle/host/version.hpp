#include "../../project.hpp"

///\file
///\brief the version number of the psycle host application.

/// M.m.p , where:
/// 	- M = major version number.
/// 	- m = minor version number:
/// 		- if even, then it's a stable release.
/// 		- if odd, then it's an unstable build or release candidate for next stable minor version number.
/// 	- p = patch number:
/// 		- if the minor version number is even (stable release), then this patch number is used for bug fixes, so, it's hopefully rarely incremented.
/// 		- if the minor version number is odd (unstable builds), then this patch number is incremented very often, each cvs commit.

/// Other files that need to be updated accordingly:
/// - make/doxygen/doxygen.configuration: PROJECT_NUMBER
/// - doc/for-end-users/readme.txt
/// - doc/for-end-users/whatsnew.txt
/// ... if only we were using a programmable build system like autoconf.

#define PSYCLE__TAR_NAME "psycle"
#define PSYCLE__NAME "Psycle Modular Music Creation Studio"
#define PSYCLE__BRANCH "psycledelics"
#define PSYCLE__COPYRIGHT "Copyright (C) 2000-2007 Psycledelics (http://psycle.pastnotecut.org and http://psycle.sourceforge.net)"
#define PSYCLE__LICENSE "none, public domain"
#define PSYCLE__VERSION__MAJOR 1
#define PSYCLE__VERSION__MINOR 8
#define PSYCLE__VERSION__PATCH 3
#define PSYCLE__VERSION__QUALITY "beta3"

/// identifies what sources the build comes from.
#define PSYCLE__VERSION \
	PSYCLE__BRANCH " " \
	STRINGIZED(PSYCLE__VERSION__MAJOR) "." \
	STRINGIZED(PSYCLE__VERSION__MINOR) "." \
	STRINGIZED(PSYCLE__VERSION__PATCH) " " \
	PSYCLE__VERSION__QUALITY 

/// identifies both what sources the build comes from, and what build options were used.
#define PSYCLE__BUILD__IDENTIFIER(EOL) \
	"version: " PSYCLE__VERSION EOL \
	"build configuration options:" EOL PSYCLE__CONFIGURATION__OPTIONS(EOL) EOL \
	"built on: " PSYCLE__BUILD__DATE

#if defined COMPILER__RESOURCE
	/// __DATE__ and __TIME__ doesn't seem to work with msvc's resource compiler
	#define PSYCLE__BUILD__DATE "a sunny day"
#else
	#define PSYCLE__BUILD__DATE __DATE__ ", " __TIME__
#endif

#if defined COMPILER__RESOURCE
	// http://msdn.microsoft.com/library/default.asp?url=/library/en-us/tools/tools/versioninfo_resource.asp

	#define RC__CompanyName PSYCLE__BRANCH
	#define RC__LegalCopyright PSYCLE__COPYRIGHT
	#define RC__License PSYCLE__LICENSE

	#define RC__InternalName PSYCLE__TAR_NAME
	#define RC__ProductName PSYCLE__NAME
	#define RC__ProductVersion PSYCLE__VERSION EOL "$Revision$" EOL "$Date$"

	#define RC__OriginalFilename PSYCLE__TAR_NAME ".exe"
	#define RC__FileDescription RC__ProductName " - Host"
	#define RC__FileVersion RC__ProductVersion

	#define RC__SpecialBuild PSYCLE__CONFIGURATION__OPTIONS(EOL)
	#define RC__PrivateBuild PSYCLE__BUILD__DATE

	// Actual resource version info code is in resources.rc.
	// Using msvc's resource compiler, #including this file from it doesn't create the version info.
	// There's no reason this wouldn't work, it's weird.
	// Anyway, all the version information is set via the above parameters,
	// so that there's no need to change the resources.rc file.
#endif
