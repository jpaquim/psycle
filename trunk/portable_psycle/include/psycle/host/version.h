#include "../../../src/project.h"

#if 0
/*
$Log$
Revision 1.10  2005/01/23 09:04:48  johan-boule
new version

Revision 1.9  2005/01/19 23:16:55  johan-boule
no more .rar binary distro, it's 7-zip now :-)

Revision 1.8  2005/01/19 23:11:11  johan-boule
no message

Revision 1.7  2005/01/14 22:34:42  johan-boule
new version

Revision 1.6  2005/01/14 22:09:14  jaz001
fix for resources not being editable inside MSVC

Revision 1.5  2005/01/13 23:18:50  johan-boule
no message

Revision 1.4  2005/01/07 22:27:47  johan-boule
std::ostringstream.clear missuses fixed (3 occurences)

Revision 1.3  2005/01/01 20:13:48  johan-boule
version patch 18

Revision 1.2  2005/01/01 20:05:49  johan-boule
no message

Revision 1.1  2004/12/22 22:43:41  johan-boule
fix closing bug [ 1087782 ] psycle MFC's version number is spread in several places
*/
#endif
		  
///\file
///\brief the version number of the psycle host application.

/// Versions are composed this way:
/// C.M.m.p , where:
/// - C = codebase generation.
/// - M = major version number.
/// - m = minor version number: if 0, release. if different than 0, release candidate for next major version number.
/// - p = patch number.

/// Other files that need to be updated accordingly:
/// - make/doxygen/doxygen.configuration: PROJECT_NUMBER
/// - doc/for-end-users/readme.txt
/// - doc/for-end-users/whatsnew.txt
/// [bohan] ... if only we were using a scriptable build system like autoconf.
			
#define PSYCLE__VERSION__CODEBASE 1
#define PSYCLE__VERSION__MAJOR 7
#define PSYCLE__VERSION__MINOR 7
#define PSYCLE__VERSION__PATCH 23 /* $Revision$ $Date$ */
#define PSYCLE__VERSION__QUALITY "alpha"

#define PSYCLE__VERSION \
	PSYCLE__VERSION__QUALITY " " \
	STRINGIZED(PSYCLE__VERSION__CODEBASE) "." \
	STRINGIZED(PSYCLE__VERSION__MAJOR) "." \
	STRINGIZED(PSYCLE__VERSION__MINOR) "." \
	STRINGIZED(PSYCLE__VERSION__PATCH)

#if defined COMPILER__RESOURCE
	#define RC__CompanyName "Psycledelics"
	#define RC__LegalCopyright "(public domain) Copyright (C) 2000-2005 Psycledelics"
	#define RC__License "none, public domain"

	#define RC__ProductName "Psycle Modular Music Creation Studio"
	#define RC__InternalName "Psycle"
	#define RC__ProductVersion PSYCLE__VERSION ", " RC__DEBUG

	#define RC__FileDescription "Psycle Modular Music Creation Studio - Host"
	#define RC__OriginalFilename "psycle.exe"
	#define RC__FileVersion PSYCLE__VERSION ", " RC__DEBUG
	#define RC__SpecialBuild "compiler build tool chain:\r\n" "msvc 7.1"
	#define RC__PrivateBuild "built every sunnyday" //__DATE__ __TIME__

	#if defined NDEBUG
		#define RC__DEBUG "compiled with debugging disabled"
	#else
		#define RC__DEBUG "compiled with debugging enabled"
	#endif

	// [bohan]
	// Actual resource code moved back to psycle.rc ...
	// Dunno why, using msvc's resource compiler, #including this file from it doesn't create the version info ...
	// There's no reason this wouldn't work, it's weird.
	// Anyway, all the version information is set via the above parameters,
	// so that there's no need to change the psycle.rc file.
#endif
