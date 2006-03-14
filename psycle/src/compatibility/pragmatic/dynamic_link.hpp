///\file
///\brief shared dynamic-link library loader
#pragma once
#include INCLUDE(PROJECT/project.hpp)
#include INCLUDE(PROJECT/compilers.hpp)
#include INCLUDE(PROJECT/operating_systems.hpp)
#include "pragma.hpp"
#if !defined OPERATING_SYSTEM__MICROSOFT || defined COMPILER__GNU // gcc handles dll transparently :-)
	/// everything is nice and simple
	/// [bohan] but, see http://people.redhat.com/drepper/dsohowto.pdf (pdf sux ... is there a non pdf version?)
	///\todo /dev/null will fail on mingw outside of msys ... we need an empty file... or an already-included one... maybe #include __FILE__
	#define LIBRARY__BEGIN  "/dev/null"
	#define LIBRARY__EXPORT "/dev/null"
	#define LIBRARY__IMPORT "/dev/null"
	#define LIBRARY__END    "/dev/null"
#else
	#define EXPORT pragma(dllexport)
	#define IMPORT pragma(dllimport)
	#define LIBRARY__BEGIN  <operating_system/library/begin.hpp>
	#define LIBRARY__EXPORT <operating_system/library/export.hpp>
	#define LIBRARY__IMPORT <operating_system/library/import.hpp>
	#define LIBRARY__END    <operating_system/library/end.hpp>
	/**/#if 0
	// This horrible problem that bloats the headers hopefully
	// only occurs with microsoft's dll format with other compilers than GCC :-)
	// If you really insist on using another compiler on the microsoft operating system,
	// see:
		#include <operating_system/library/export.hpp>
		#include <operating_system/library/import.hpp>
	// then, starts bloating every header and source files with "appropriate" preprocessor directives.
	// Example of use:
		// in file foo/bar.cpp:
			#define LIBRARY__FOO__BAR
			#include "bar.hpp"
		// in file foo/bar.hpp:
			#pragma once
			#if defined LIBRARY__FOO__BAR
				#include LIBRARY__EXPORT
			#else
				#include LIBRARY__IMPORT
			#endif
			namespace foo
			{
				class bar
				{
				};
			}
	// Alternative way is to use:
		#include <operating_system/library/begin.hpp>
		#include <operating_system/library/end.hpp>
	// which attempt to be smarter by doing,
			#define class class pragma(dllexport)
	// or,
			#define class class pragma(dllimport)
	// so that everything is handled transparently.
	// Example of use:
		// in file foo/bar.cpp:
			#define FOO__BAR 1
			#include "bar.hpp"
		// in file foo/bar.hpp:
			#pragma once
			#define LIBRARY FOO__BAR
			#include LIBRARY__BEGIN
			namespace foo
			{
				class bar
				{
				};
			}
			#include LIBRARY__END
	/**/#endif
#endif
