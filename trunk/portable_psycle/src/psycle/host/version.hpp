#include "../../project.hpp"

#if 0
/*
$Log$
Revision 1.18  2005/03/29 19:57:53  johan-boule
full rebuild fixes problem with sampler machine (was prolly a wrong file date)

Revision 1.17  2005/03/26 13:31:05  johan-boule
fixed an error in the description of the versioning scheme in <psycle/host/version.hpp>

Revision 1.16  2005/03/24 16:22:52  johan-boule
added special handling of labels in the machine parameter combo box for native plugins

Revision 1.15  2005/03/22 01:23:57  johan-boule
increased version patch number and released alpha build
(alk's fix for karlkox surround)

Revision 1.14  2005/03/12 02:37:02  johan-boule
increased version patch number and released alpha build
(sampler's keyboard handler fix)

Revision 1.12  2005/02/19 20:51:13  jaz001
* fix bug in song.hpp "_ptrack" and "_ptrackline" functions where it didn't return the correct value
* fix bug in XMSampler/XMInstrument where "&" and  "|" were being used erroneously in place of "&&"and "||".
* Added IT loader
* Converted XMSampler "Layers (waves) per instrument" to independent sample bank and instrument bank.
* Work in progress -> Convert XMSampler to the default Psycle sampler. (Psampler)

Revision 1.11  2005/02/07 10:48:20  johan-boule
bug fix

Revision 1.9  2005/01/29 18:25:43  johan-boule
fixing problems for msvc's resource editor

Revision 1.3  2005/01/24 17:36:22  johan-boule
there was a nasty backslash at the end of a line preventing the psycle namespace to close and creating a nested psycle namespace inside the psycle namespace itself.
new alpha version 1.7.26 released.

Revision 1.2  2005/01/24 15:54:03  johan-boule
path for resources.rc

Revision 1.4  2005/01/07 22:27:47  johan-boule
std::ostringstream.clear missuses fixed (3 occurences)

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
///			- if odd, then it's a beta or release candidate for next minor version number.
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
#define PSYCLE__VERSION__PATCH 36 /* $Revision$ $Date$ */
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
	#define RC__LegalCopyright "Copyright (C) 2000-2005 Psycledelics"
	#define RC__License "none, public domain"

	#define RC__ProductName "Psycle Modular Music Creation Studio"
	#define RC__InternalName "Psycle"
	#define RC__ProductVersion PSYCLE__VERSION ", " RC__DEBUG

	#define RC__FileDescription RC__ProductName " - Host"
	#define RC__OriginalFilename "psycle.exe"
	#define RC__FileVersion RC__ProductVersion
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
