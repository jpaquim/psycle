#include <psycle/engine/detail/project.hpp>
#include <universalis/compiler/stringized.hpp>

///\file
///\brief the version number of the psycle playback engine.

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

#define PSYCLE__ENGINE__TAR_NAME "psycle"
#define PSYCLE__ENGINE__NAME "Psycle Modular Music Creation Studio"
#define PSYCLE__ENGINE__BRANCH "psycledelics"
#define PSYCLE__ENGINE__COPYRIGHT "copyright 2000-2007 psycledelics (http://psycle.pastnotecut.org and http://psycle.sourceforge.net) ; see the AUTHORS file"
#define PSYCLE__ENGINE__LICENSE "none, public domain"
#define PSYCLE__ENGINE__VERSION__MAJOR 1
#define PSYCLE__ENGINE__VERSION__MINOR 9
#define PSYCLE__ENGINE__VERSION__PATCH 13
#define PSYCLE__ENGINE__VERSION__QUALITY "alpha"

/// identifies what sources the build comes from.
#define PSYCLE__ENGINE__VERSION \
	PSYCLE__ENGINE__BRANCH " " \
	UNIVERSALIS__COMPILER__STRINGIZED(PSYCLE__ENGINE__VERSION__MAJOR) "." \
	UNIVERSALIS__COMPILER__STRINGIZED(PSYCLE__ENGINE__VERSION__MINOR) "." \
	UNIVERSALIS__COMPILER__STRINGIZED(PSYCLE__ENGINE__VERSION__PATCH) " " \
	PSYCLE__ENGINE__VERSION__QUALITY 

/// identifies both what sources the build comes from, and what build options were used.
#define PSYCLE__ENGINE__BUILD__IDENTIFIER \
	"version: "  PSYCLE__ENGINE__VERSION  UNIVERSALIS__OPERATING_SYSTEM__EOL \
	"build configuration options:"  UNIVERSALIS__OPERATING_SYSTEM__EOL  PSYCLE__ENGINE__CONFIGURATION__DESCRIPTION  UNIVERSALIS__OPERATING_SYSTEM__EOL \
	"built on: "  PSYCLE__ENGINE__BUILD__DATE

#if defined DIVERSALIS__COMPILER__RESOURCE && !defined DIVERSALIS__COMPILER__GNU // fine with gcc/mingw's winres compiler, which uses gcc's preprocessor
	/// __DATE__ and __TIME__ doesn't work with msvc's resource compiler
	#define PSYCLE__ENGINE__BUILD__DATE "a sunny day"
#else
	#define PSYCLE__ENGINE__BUILD__DATE __DATE__ ", " __TIME__
#endif
