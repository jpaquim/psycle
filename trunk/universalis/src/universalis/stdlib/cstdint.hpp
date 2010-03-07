// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2004-2010 members of the psycle project http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>

///\file \brief cstdint standard header

#ifndef UNIVERSALIS__STDLIB__STDINT__INCLUDED
#define UNIVERSALIS__STDLIB__STDINT__INCLUDED
#pragma once

#include <universalis/detail/project.hpp>
#if 0
	// what we would like to include in an ideal world
	#include <cstdint>

	// C1999
	#if __STDC_VERSION__ >= 199901
		#include <stdint.h>
	#endif

	// most unix systems had the equivalent inttypes.h for a long time
	#if defined DIVERSALIS__OS__UNIX
		#include <inttypes.h>
	#endif

#else
	// see also http://www.azillionmonkeys.com/qed/pstdint.h

	// boost takes care of all the mess for us
	#include <boost/cstdint.hpp>
	#include <climits> // for UINT_MAX and ULLONG_MAX
	namespace universalis { namespace stdlib {
		namespace stdint {
			using boost::int8_t;
			using boost::int_least8_t;
			using boost::int_fast8_t;
			using boost::uint8_t;
			using boost::uint_least8_t;
			using boost::uint_fast8_t;
			
			using boost::int16_t;
			using boost::int_least16_t;
			using boost::int_fast16_t;
			using boost::uint16_t;
			using boost::uint_least16_t;
			using boost::uint_fast16_t;

			// don't let boost use 'long int'
			#if defined DIVERSALIS__OS__MICROSOFT && \
				UINT_MAX == 0xffffffffu && ULLONG_MAX == 0xffffffffffffffffull
				typedef signed int int32_t;
				typedef unsigned int uint32_t;
				typedef signed long long int64_t;
				typedef unsigned long long uint64_t;
			#else
				using boost::int32_t;
				using boost::uint32_t;
				using boost::int64_t;
				using boost::uint64_t;
			#endif

			//using boost::int32_t;
			using boost::int_least32_t;
			using boost::int_fast32_t;
			//using boost::uint32_t;
			using boost::uint_least32_t;
			using boost::uint_fast32_t;
		
			//using boost::int64_t;
			using boost::int_least64_t;
			using boost::int_fast64_t;
			//using boost::uint64_t;
			using boost::uint_least64_t;
			using boost::uint_fast64_t;

			using boost::intmax_t;
			using boost::uintmax_t;
		}
		using namespace stdint;
	}}
#endif

/****************************************************************************/
// injection in std namespace
namespace std { using namespace universalis::stdlib::stdint; }

/****************************************************************************/
// injection in root namespace
using namespace universalis::stdlib::stdint;

#endif
