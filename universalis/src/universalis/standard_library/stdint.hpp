// -*- mode:c++; indent-tabs-mode:t -*-
// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2004-2008 members of the psycle project http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>

///\file \brief cstdint standard header
#pragma once
#include <universalis/detail/project.hpp>
#if 0
	// what we would like to include in an ideal world
	#include <cstdint>

	// C1999
	#if __STDC_VERSION__ >= 199901
		#include <stdint.h>
	#endif

	// some unix systems had the equivalent inttypes.h for a long time
	#include <diversalis/operating_system.hpp>
	#if defined DIVERSALIS__OPERATING_SYSTEM__UNIX
		#include <inttypes.h>
	#endif
#else
	// boost takes care of all the mess for us
	#include <boost/cstdint.hpp>
	namespace std {
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
		
		using boost::int32_t;
		using boost::int_least32_t;
		using boost::int_fast32_t;
		using boost::uint32_t;
		using boost::uint_least32_t;
		using boost::uint_fast32_t;
		
		using boost::int64_t;
		using boost::int_least64_t;
		using boost::int_fast64_t;
		using boost::uint64_t;
		using boost::uint_least64_t;
		using boost::uint_fast64_t;
		
		using boost::intmax_t;
		using boost::uintmax_t;
	}
#endif
