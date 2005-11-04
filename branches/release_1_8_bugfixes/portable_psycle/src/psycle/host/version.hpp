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
#define PSYCLE__COPYRIGHT "Copyright (C) 2000-2005 Psycledelics (http://psycle.pastnotecut.org and http://psycle.sourceforge.net)"
#define PSYCLE__LICENSE "none, public domain"
#define PSYCLE__VERSION__MAJOR 1
#define PSYCLE__VERSION__MINOR 8
#define PSYCLE__VERSION__PATCH 0
#define PSYCLE__VERSION__QUALITY "Release"

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

#if 0
/*
$Log$
Revision 1.45  2005/07/24 16:59:02  jaz001
* fixed baground bitmap bug.
* changed default colours for new release
* rearranged the skin dialog buttons
* fixed but with skin vu-meters.
* muting note duplicator stops sending events.

Revision 1.44  2005/07/17 03:51:09  johan-boule
bugfix for midi controllers config dialog

Revision 1.43  2005/07/11 01:22:01  johan-boule
various little cleanup for 1.8 release

Revision 1.42  2005/07/10 21:08:20  jaz001
New defaults for machine parameters
New skin for machine parameters' knobs.
Importing configuration from older psycle's and psycle alpha.
removing unimplemented alk's mixer.
added comments about unfinished features.
fixed small but in "recent files".
Note: this is labeled 1.8.-1, since configuration will be saved as 1.8, but it might have still small changes for the official 1.8

Revision 1.41  2005/06/29 22:26:51  johan-boule
closes bug: sourceforge bug tracker [ 1229490 ] Velocity / volume data with MIDI keyboard

Revision 1.40  2005/06/27 15:52:39  johan-boule
Version 1.7.58
<alk> moved the checkbox for move cursor when pasting to the sequencer panel

Revision 1.39  2005/06/26 11:08:29  jaz001
1.8 RC1.1

Revision 1.38  2005/06/19 21:04:29  johan-boule
Version 1.7.56 (pre-release 1 of 1.8)
[jaz] fix bugs with Note Duplicator (load/save and deadlock).
[jaz] fix bug with render to wav, where in some specific situations where playing some instruments incorrectly.

Revision 1.37  2005/06/09 22:49:00  johan-boule
Version 1.7.55 (alpha)
just increasing the version number to reflect the presence of a bug fix concerning loading of skins

Revision 1.36  2005/06/07 18:16:55  johan-boule
Version 1.7.54 (alpha)
oops, fix for bug introduced in 1.7.53 concerning midi device selection in the configuration

Revision 1.35  2005/06/04 00:20:25  johan-boule
1)
closing sourceforge bug #1087788.
rearranged the midi variables which had the channel
number appended to their names into arrays, and replaced the
related repetitive code with loops.

2)
The interface of the registry class used to require we
specify the type and size of the each value we read or
write. I've changed it to use function overloading for types
like bool, all variants of int, and std::string, and to use
a template for other types so they just use REG_BINARY
(which means any type). So we can simply pass the variable
to load/save and its type and size are implicit ; it's less
error prone and looks nicer.

It works correctly, but i've noticed problems loading my
existing registry settings because some of the types used to
be REG_BINARY for values that should have been REG_DWORD
(and iirc vice-versa), and the registry class complains
about type missmatch. To work around it i've changed the
path of the "root" key to software/psycle/psycledelics-1.7
(it's actually determined from the values in
version.hpp).... the drawback is that people "loose" their
old settings.

Revision 1.34  2005/05/30 05:13:43  johan-boule
Version 1.7.51 (alpha)
change to let plugins find other .dll files that are in any intermediate dir between their own dir and the configured root dir for plugins

Revision 1.33  2005/05/29 23:21:23  johan-boule
Version 1.7.50 (alpha)
change to let plugins find other .dll files that are in the same dir as their own .dll
for example, blwtbl.dll can be put along with druttis plugins instead of psycle.exe's dir.

Revision 1.32  2005/05/29 00:34:58  johan-boule
cvs $File -> $Id

Revision 1.31  2005/05/25 20:20:35  johan-boule
Version 1.7.49 (alpha)
removed scary pointer and index stuff from the directsound config
(the device index value in the registry can be deleted as it's not used anymore)

Revision 1.30  2005/05/25 04:02:17  johan-boule
Version 1.7.48 (alpha)
fixed bug for fingersoup's directsound driver (null GUID)

Revision 1.29  2005/05/24 01:09:17  johan-boule
bug fix + special 0xff voice to send the same command to all voices

Revision 1.28  2005/05/23 23:41:23  johan-boule
to be tested...
change that allows the player to send commands (Machine::Tick function) from any pattern track with a given channel (or "voice") number:

if the machine is not a vst, and if the note is a "mcm" (hmm, poor name if it's not related with midi),
then, the fields in the event are interpreted as <channel> <machine> <command> <parameter>.
for example, 1122334444, means:
11 is the channel (or "voice"),
22 is the machine,
33 is the command, and
4444 is the parameter
The player calls the Machine::Tick function with those values, and the instrument value set to 0.

Revision 1.27  2005/05/18 06:55:47  johan-boule
releasing voskomo's blitz plugin

Revision 1.26  2005/05/14 15:32:57  johan-boule
bug fix
operating_system::exceptions::translated::what() returned the address of a temporary stack object

Revision 1.25  2005/05/12 22:18:39  johan-boule
increased version number for alpha release
includes jaz's bug fixes
fpu exceptions disabled in the configuration "by default" as it's only useful for crazy debugging hell

Revision 1.24  2005/05/12 14:55:49  johan-boule
Added handling of fpu exceptions.
It's a build configuration option, so it can be disabled.
It only catches fpu exceptions in the machines' work functions, but this could be extended to other functions as well.
Each exception type is reported only once per machine it occurs, by growing the fpu exception mask.
"Inexact result" fpu exceptions are initially already masked.

Also, made exceptions which happen in machines appear not only in the lof, but also in a message box, because many people wondered why some machines were automagically set to bypassed/muted (The message box wouldn't be needed if the MFC logging window worked.)

Revision 1.23  2005/05/01 14:10:59  jaz001
More Work on Sampulse (IT Filtering, improving commands, bugfixing).
Removed an cleaned (a bit) old sample sources (removed the never used sample layers)
Added a new internal machine "Note Duplicator". It just resends the Tick comand to the specificed machines
Some other small changes ( Changed the way interpolation is modified, and others)

Revision 1.22  2005/04/18 02:00:41  johan-boule
added src/configuration.hpp to psycle.vcproj

Revision 1.21  2005/04/10 20:11:37  johan-boule
releasing alpha version 1.7.39 -- with voskomo's gamefx plugin

Revision 1.20  2005/04/06 04:32:03  johan-boule
Attempt to use boost::read_write_mutex instead of an exclusive mutex for Song's lock.
Curiously, it dead-locks.
So, i made it a build option that can be enabled/disabled in src/configuration.hpp
The version info and the about box is modified to show what build options were used (multiline text).

Revision 1.19  2005/04/02 21:48:37  johan-boule
releasing alpha version 1.7.37

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
