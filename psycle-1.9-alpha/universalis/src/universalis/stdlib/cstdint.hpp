// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2004-2011 members of the psycle project http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>

///\file \brief cstdint standard header

#pragma once
#include <universalis/detail/project.hpp>
#ifdef DIVERSALIS__COMPILER__FEATURE__CXX0X
	#include <cstdint>
	namespace universalis { namespace stdlib {
		using std::int8_t;
		using std::int_least8_t;
		using std::int_fast8_t;
		using std::uint8_t;
		using std::uint_least8_t;
		using std::uint_fast8_t;

		using std::int16_t;
		using std::int_least16_t;
		using std::int_fast16_t;
		using std::uint16_t;
		using std::uint_least16_t;
		using std::uint_fast16_t;

		using std::int32_t;
		using std::int_least32_t;
		using std::int_fast32_t;
		using std::uint32_t;
		using std::uint_least32_t;
		using std::uint_fast32_t;

		using std::int64_t;
		using std::int_least64_t;
		using std::int_fast64_t;
		using std::uint64_t;
		using std::uint_least64_t;
		using std::uint_fast64_t;

		using std::intmax_t;
		using std::uintmax_t;
	}}
#else
	#include <boost/cstdint.hpp>
	#ifdef DIVERSALIS__OS__MICROSOFT
		#include <climits> // for UINT_MAX and ULLONG_MAX
	#endif
	namespace universalis { namespace stdlib {
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
		///\todo come back to this.. it's too ugly!
		#if defined DIVERSALIS__OS__MICROSOFT && UINT_MAX == 0xffffffffu && ULLONG_MAX == 0xffffffffffffffffull
			typedef signed int int32_t;
			typedef unsigned int uint32_t;
			typedef signed long long int int64_t;
			typedef unsigned long long int uint64_t;
		#else
			using boost::int32_t;
			using boost::uint32_t;
			using boost::int64_t;
			using boost::uint64_t;
		#endif

		using boost::int_least32_t;
		using boost::int_fast32_t;
		using boost::uint_least32_t;
		using boost::uint_fast32_t;

		using boost::int_least64_t;
		using boost::int_fast64_t;
		using boost::uint_least64_t;
		using boost::uint_fast64_t;

		using boost::intmax_t;
		using boost::uintmax_t;
	}}
#endif
