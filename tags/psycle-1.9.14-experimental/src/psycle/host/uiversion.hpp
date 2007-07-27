#include <psycle/host/detail/project.hpp>
#include <universalis/compiler/stringized.hpp>

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
#define PSYCLE__COPYRIGHT "copyright 2000-2007 psycledelics (http://psycle.pastnotecut.org and http://psycle.sourceforge.net) ; see the AUTHORS file"
#define PSYCLE__LICENSE "none, public domain"
#define PSYCLE__VERSION__MAJOR 1
#define PSYCLE__VERSION__MINOR 9
#define PSYCLE__VERSION__PATCH 12
#define PSYCLE__VERSION__QUALITY "alpha"

/// identifies what sources the build comes from.
#define PSYCLE__VERSION \
	PSYCLE__BRANCH " " \
	UNIVERSALIS__COMPILER__STRINGIZED(PSYCLE__VERSION__MAJOR) "." \
	UNIVERSALIS__COMPILER__STRINGIZED(PSYCLE__VERSION__MINOR) "." \
	UNIVERSALIS__COMPILER__STRINGIZED(PSYCLE__VERSION__PATCH) " " \
	PSYCLE__VERSION__QUALITY 

/// identifies both what sources the build comes from, and what build options were used.
#define PSYCLE__BUILD__IDENTIFIER \
	"version: "  PSYCLE__VERSION  UNIVERSALIS__OPERATING_SYSTEM__EOL \
	"build configuration options:"  UNIVERSALIS__OPERATING_SYSTEM__EOL  PSYCLE__CONFIGURATION__DESCRIPTION  UNIVERSALIS__OPERATING_SYSTEM__EOL \
	"built on: "  PSYCLE__BUILD__DATE

#if defined DIVERSALIS__COMPILER__RESOURCE && !defined DIVERSALIS__COMPILER__GNU // fine with gcc/mingw's winres compiler, which uses gcc's preprocessor
	/// __DATE__ and __TIME__ doesn't work with msvc's resource compiler
	#define PSYCLE__BUILD__DATE "a sunny day"
#else
	#define PSYCLE__BUILD__DATE __DATE__ ", " __TIME__
#endif

#if defined DIVERSALIS__COMPILER__RESOURCE
	// http://msdn.microsoft.com/library/default.asp?url=/library/en-us/tools/tools/versioninfo_resource.asp

	#define RC__CompanyName      PSYCLE__BRANCH
	#define RC__LegalCopyright   PSYCLE__COPYRIGHT
	#define RC__License          PSYCLE__LICENSE

	#define RC__InternalName     PSYCLE__TAR_NAME
	#define RC__ProductName      PSYCLE__NAME
	#define RC__ProductVersion   PSYCLE__VERSION

	#define RC__OriginalFilename PSYCLE__TAR_NAME
	#define RC__FileDescription RC__ProductName " - Host"
	#define RC__FileVersion RC__ProductVersion

	#define RC__SpecialBuild PSYCLE__CONFIGURATION__DESCRIPTION
	#define RC__PrivateBuild PSYCLE__BUILD__DATE

	// Actual resource version info code (VS_VERSION_INFO) is in resources.rc2.
	// Using msvc's resource compiler, putting VS_VERSION_INFO in the file and #including it from resources.rc2 doesn't create the version info.
	// There's no reason this wouldn't work, it's weird.
	// Anyway, all the version information is set via the above parameters,
	// so that there's no need to change the resources.rc2 file.
#endif
