// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2006-2007 psycledelics http://psycle.pastnotecut.org

///\file
///\brief global public include for header files of the psycle::host project
#pragma once
#include <diversalis/compiler.hpp>
#if !defined DIVERSALIS__COMPILER__RESOURCE
	#include <diversalis/diversalis.hpp>
	#include <universalis/universalis.hpp>
#endif

#include "configuration.hpp"

#if !defined DIVERSALIS__COMPILER__RESOURCE

	#if defined PSYCLE__QUAQUAVERSALIS && defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT
		// include <windows.h> here so we don't have to deal with this again everywhere
		//\todo for mfx apps, inclusions of <afxwin.h> is currently only handled thanks to <packageneric/pre-compiled.private.hpp>
		//\todo this should be moved to a file that is always included whether or not we're using some pre-compiled headers that does it.
		#include <windows.h>
	#endif

	#if PSYCLE__CONFIGURATION__TYPE_SAFE
		#define PSYCLE__STRONG_TYPEDEF(type, name) BOOST_STRONG_TYPEDEF(type, name);
		#include <boost/strong_typedef.hpp>
	#else
		#define PSYCLE__STRONG_TYPEDEF(type, name) typedef type name;
	#endif

	#if PSYCLE__CONFIGURATION__TIGHT_MEMBER_ACCESS_CONTROL
		#define PSYCLE__PRIVATE private
		#define PSYCLE__PROTECTED protected
		#define PSYCLE__DEPRECATED(message) UNIVERSALIS__COMPILER__DEPRECATED(message)
	#else
		#define PSYCLE__PRIVATE public
		#define PSYCLE__PROTECTED public
		#define PSYCLE__DEPRECATED(message)
	#endif

	// psycle still makes some asumptions about the size of the compiler's numeric types
	// so we check everything here first
	#include <boost/static_assert.hpp>
	BOOST_STATIC_ASSERT((sizeof(         char) == 1)); // probably always true
	BOOST_STATIC_ASSERT((sizeof(    short int) == 2));
	BOOST_STATIC_ASSERT((sizeof(          int) == 4));
	BOOST_STATIC_ASSERT((sizeof(     long int) == 4));
	BOOST_STATIC_ASSERT((sizeof(long long int) == 8));
	BOOST_STATIC_ASSERT((sizeof(        float) == 4)); // probably always true (ieee754)
	BOOST_STATIC_ASSERT((sizeof(       double) == 8)); // probably always true (ieee754)
//	BOOST_STATIC_ASSERT((sizeof(  long double) == 10));
//	BOOST_STATIC_ASSERT((sizeof(  long double) == 12));

	///\name depecate a bunch of ms types
	/// this is mostly finished appart from mfc code of course
	///{
		//PSYCLE__DEPRECATED("same as std::uint8_t" ) typedef ::UCHAR UCHAR;
		//PSYCLE__DEPRECATED("same as std::uint16_t") typedef ::WORD WORD;
		//PSYCLE__DEPRECATED("same as std::uint32_t") typedef ::DWORD DWORD;
		//PSYCLE__DEPRECATED("same as std::uint32_t") typedef ::UINT UINT;
		//PSYCLE__DEPRECATED("same as std::uint32_t") typedef ::ULONG ULONG;
		//PSYCLE__DEPRECATED("same as std::uint64_t") typedef ::ULONGLONG ULONGLONG;
	///\}

#endif
