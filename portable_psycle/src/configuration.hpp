
// Note: this public configuration was not generated by an autoconf configuration script.

#pragma once

/// support for nt-based branch only.
#define OPERATING_SYSTEM__MICROSOFT__BRANCH__NT
#undef OPERATING_SYSTEM__MICROSOFT__BRANCH__NT

///\todo If we want to support non-nt based windows,
///\todo defining the version by date doesn't work since, until year 2000, microsoft used to maintain 
///\todo two operating systems at the same time: nt-based windows, and non-nt based windows.
///\todo Those were in fact two different branches, not just different versions, i.e. it's not possible to compare them with a numerical comparison on a version number.
///\todo (for example, nt-3.1, released in 1993, exposes features in its windows subsystem api that millenium, released in 2001 doesn't have)
///\todo If we simply don't support the non-nt based branch, there's no problem.
#define OPERATING_SYSTEM__VERSION__MICROSOFT__COMPATIBILITY \
         OPERATING_SYSTEM__VERSION__MICROSOFT__COMPATIBILITY__1998



/// [bohan] added implementation of psycle::host::Song's lock using boost 1.3's read_write_mutex.
/// [bohan]
/// [bohan] I used a temporary #define (to be removed) to enable this new implementation of the gui<->audio thread synchronization.
/// [bohan] Once the new implementation is known to work well,
/// [bohan] we can remove this #define, which will trigger some #error in the places of the code that are concerned.
/// [bohan] Where the #error occurred, we can removed the old implementation.
/// [bohan]
/// [bohan] to enable this new implementation,
/// [bohan] #define PSYCLE__CONFIGURATION__OPTION__ENABLE__READ_WRITE_MUTEX 1
/// [bohan]
/// [bohan] to disable this new implementation, do not undefine the preprocessor symbol (which will triggers the #error's), but rather
/// [bohan] #define PSYCLE__CONFIGURATION__OPTION__ENABLE__READ_WRITE_MUTEX 0
#define PSYCLE__CONFIGURATION__OPTION__ENABLE__READ_WRITE_MUTEX 0

/// JAZ: Define to 1 to enable the volume column for XMSampler. It will also make the machine column in the pattern to show
///      the values of the volume column instead.
#define PSYCLE__CONFIGURATION__OPTION__VOLUME_COLUMN 0

/// Test for RMS Vu's
//#define PSYCLE__CONFIGURATION__RMS_VUS 1

/// unmasks fpu exceptions
/// [JAZ] : I have experienced crashes with this option enabled, which didn't seem to come from the code itself.
/// [JAZ]   It could be that the exception code handling has a bug somewhere.
#define PSYCLE__CONFIGURATION__OPTION__ENABLE__FPU_EXCEPTIONS 0


/// the compiler used to build
#if defined COMPILER__MICROSOFT
	#define PSYCLE__COMPILER__BUILD  "msvc"  COMPILER__VERSION__MAJOR  COMPILER__VERSION__MINOR
#elif defined COMPILER__GNU
	#define PSYCLE__COMPILER__BUILD  "gcc"  COMPILER__VERSION__MAJOR  COMPILER__VERSION__MINOR  COMPILER__VERSION__PATCH
#else
	#define PSYCLE__COMPILER__BUILD  "no autoconf => no information"
#endif



/// string describing the configuration options.
#define PSYCLE__CONFIGURATION__OPTIONS(EOL) \
	"compiler build tool chain = " PSYCLE__COMPILER__BUILD EOL \
	"read_write_mutex = " STRINGIZED(PSYCLE__CONFIGURATION__OPTION__ENABLE__READ_WRITE_MUTEX) EOL \
	"fpu exceptions = " STRINGIZED(PSYCLE__CONFIGURATION__OPTION__ENABLE__FPU_EXCEPTIONS) EOL \
	"volume column = " STRINGIZED(PSYCLE__CONFIGURATION__OPTION__VOLUME_COLUMN) EOL \
	"debugging = " PSYCLE__CONFIGURATION__OPTION__ENABLE__DEBUG

