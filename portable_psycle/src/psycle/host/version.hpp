#include "../../project.hpp"

#if 0
/*
$Log$
Revision 1.4  2005/01/24 17:36:59  johan-boule
no message

Revision 1.3  2005/01/24 17:36:22  johan-boule
there was a nasty backslash at the end of a line preventing the psycle namespace to close and creating a nested psycle namespace inside the psycle namespace itself.
new alpha version 1.7.26 released.

Revision 1.2  2005/01/24 15:54:03  johan-boule
path for resources.rc

Revision 1.1  2005/01/24 14:53:49  johan-boule
This commit is going to be annoying for other people.
I have moved the header files whose implementation was in the src dir from the include dir to the src dir, along with their source, implementation files.
You can retrieve the tagged revision set release_1_7_25 if needed to go back just before this change.
The include dir still contains header files for libraries that we don't compile ourself, like the boost lib, for which the cvs just distribute the compiled libraries, and the vst headers (there's no vst compiled library tho, they are just dumb declarations).
Other third-party code, namely, asio, winamp-2, foobar2000, and stk have .cpp files which we compile ourself, hence they are in the src dir.
Of course, all of psycle's own code in the the src dir, nothing of it is in the include dir anymore.

Revision 1.22  2005/01/23 11:24:26  johan-boule
new version 1.7.25

Revision 1.21  2005/01/23 11:04:13  johan-boule
new version scheme 1.7.24

Revision 1.20  2005/01/23 11:02:06  johan-boule
new version scheme 1.7.24

Revision 1.19  2005/01/23 10:59:12  johan-boule
version numbering again, huhu ha ho.

Revision 1.18  2005/01/23 10:39:04  johan-boule
version numbering again, huhu.

Revision 1.17  2005/01/23 10:36:05  johan-boule
version numbering again, huhu.

Revision 1.16  2005/01/23 10:07:17  johan-boule
typo

Revision 1.15  2005/01/23 10:06:16  johan-boule
typo

Revision 1.14  2005/01/23 10:05:13  johan-boule
typo

Revision 1.13  2005/01/23 10:03:01  johan-boule
version numbering

Revision 1.12  2005/01/23 09:41:25  johan-boule
version numbering

Revision 1.11  2005/01/23 09:32:32  johan-boule
comments about version numbering.. we could go to 1.7.8 maybe?

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

/// Versions are composed this way since 1.7.24:
/// M.m.p , where:
/// - M = major version number.
/// - m = minor version number:
///			- if even, then it's a stable release.
///			- if odd, then it's a beta or release candidate for next major version number.
/// - p = patch number:
///			- if the minor version number is even (stable release), then this patch number is used for bug fixes, so, it's hopefully rarely incremented.
///			- if the minor version number is odd (beta or RC), then this patch number is incremented very often, each cvs commit.

/// Other files that need to be updated accordingly:
/// - make/doxygen/doxygen.configuration: PROJECT_NUMBER
/// - doc/for-end-users/readme.txt
/// - doc/for-end-users/whatsnew.txt
/// [bohan] ... if only we were using a programmable build system like autoconf.
			
#define PSYCLE__BRANCH "" // mainline (psycledelics)
#define PSYCLE__VERSION__MAJOR 1
#define PSYCLE__VERSION__MINOR 7
#define PSYCLE__VERSION__PATCH 26 /* $Revision$ $Date$ */
#define PSYCLE__VERSION__QUALITY "alpha"

#define PSYCLE__VERSION \
	PSYCLE__BRANCH " " \
	PSYCLE__VERSION__QUALITY " " \
	STRINGIZED(PSYCLE__VERSION__MAJOR) "." \
	STRINGIZED(PSYCLE__VERSION__MINOR) "." \
	STRINGIZED(PSYCLE__VERSION__PATCH)

#if defined COMPILER__RESOURCE
	// http://msdn.microsoft.com/library/default.asp?url=/library/en-us/tools/tools/versioninfo_resource.asp

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
	// Actual resource code moved back to resources.rc ...
	// Dunno why, using msvc's resource compiler, #including this file from it doesn't create the version info ...
	// There's no reason this wouldn't work, it's weird.
	// Anyway, all the version information is set via the above parameters,
	// so that there's no need to change the resources.rc file.
#endif
