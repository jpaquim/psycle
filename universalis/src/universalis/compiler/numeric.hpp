// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2007 johan boule <bohan@jabber.org>
// copyright 2004-2007 psycledelics http://psycle.pastnotecut.org

///\file
/// numeric types
#pragma once
#include <universalis/detail/project.hpp>
//	#include <inttypes.h> // posix/unix
//	#include <stdint.h> // C1999
//	#include <cstdint> // C++0x
#if 0
	#include <cstdint>
	namespace universalis { namespace compiler { namespace numerics { namespace detail { namespace implementation = std; } } } }
#else
	#include <boost/cstdint.hpp>
	namespace universalis { namespace compiler { namespace numerics { namespace detail { namespace implementation = boost; } } } }
#endif		
namespace universalis
{
	namespace compiler
	{
		template<unsigned int const bytes = 0> struct numeric
		{
			#if defined DIVERSALIS__COMPILER__FEATURE__TYPEOF
				typedef typeof( 0) unsigned_int;
				typedef typeof(-0)   signed_int;
				typedef typeof(0.) floating_point;
			#else
				typedef unsigned int unsigned_int;
				typedef   signed int   signed_int;
				typedef double floating_point;
			#endif
		};
		template<> struct numeric<010<<0>
		{
			typedef numerics::detail::implementation:: int8_t        signed_int        ;
			typedef numerics::detail::implementation::uint8_t      unsigned_int        ;
			typedef numerics::detail::implementation:: int_fast8_t   signed_int_fastest;
			typedef numerics::detail::implementation::uint_fast8_t unsigned_int_fastest;
		};
		template<> struct numeric<010<<1>
		{
			typedef numerics::detail::implementation:: int16_t       signed_int         ;
			typedef numerics::detail::implementation::uint16_t      unsigned_int        ;
			typedef numerics::detail::implementation:: int_fast16_t   signed_int_fastest;
			typedef numerics::detail::implementation::uint_fast16_t unsigned_int_fastest;
		};
		template<> struct numeric<010<<2>
		{
			typedef numerics::detail::implementation:: int32_t        signed_int        ;
			typedef numerics::detail::implementation::uint32_t      unsigned_int        ;
			typedef numerics::detail::implementation:: int_fast32_t   signed_int_fastest;
			typedef numerics::detail::implementation::uint_fast32_t unsigned_int_fastest;
			typedef float          floating_point        ;
			typedef floating_point floating_point_fastest;
		};
		template<> struct numeric<010<<3>
		{
			#if !defined BOOST_NO_INT64_T
				typedef numerics::detail::implementation:: int64_t        signed_int        ;
				typedef numerics::detail::implementation::uint64_t      unsigned_int        ;
				typedef numerics::detail::implementation:: int_fast64_t   signed_int_fastest;
				typedef numerics::detail::implementation::uint_fast64_t unsigned_int_fastest;
			#endif
			typedef double         floating_point        ;
			typedef floating_point floating_point_fastest;
		};
		#if defined DIVERSALIS__COMPILER__FEATURE__LONG_DOUBLE
			template<> struct numeric<010<<3|010<<1>
			{
				typedef long double    floating_point        ;
				typedef floating_point floating_point_fastest;
			};
		#endif
	}
}
