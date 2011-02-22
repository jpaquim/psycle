// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2007 johan boule <bohan@jabber.org>
// copyright 2004-2007 psycledelics http://psycle.pastnotecut.org

///\file
///\brief numeric types

#ifndef UNIVERSALIS__COMPILER__NUMERIC__INCLUDED
#define UNIVERSALIS__COMPILER__NUMERIC__INCLUDED
#pragma once

#include <universalis/stdlib/cstdint.hpp>

///\todo: <boost/integer.hpp> offers the same kind of templates. this makes this code redundant.

namespace universalis { namespace compiler {
	template<const unsigned int bits = 0> struct numeric
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
		typedef  int8_t        signed_int        ;
		typedef uint8_t      unsigned_int        ;
		typedef  int_fast8_t   signed_int_fastest;
		typedef uint_fast8_t unsigned_int_fastest;
	};
	template<> struct numeric<010<<1>
	{
		typedef  int16_t       signed_int         ;
		typedef uint16_t      unsigned_int        ;
		typedef  int_fast16_t   signed_int_fastest;
		typedef uint_fast16_t unsigned_int_fastest;
	};
	template<> struct numeric<010<<2>
	{
		typedef  int32_t        signed_int        ;
		typedef uint32_t      unsigned_int        ;
		typedef  int_fast32_t   signed_int_fastest;
		typedef uint_fast32_t unsigned_int_fastest;
		typedef float          floating_point        ;
		typedef floating_point floating_point_fastest;
	};
	template<> struct numeric<010<<3>
	{
		#if !defined BOOST_NO_INT64_T
			typedef  int64_t        signed_int        ;
			typedef uint64_t      unsigned_int        ;
			typedef  int_fast64_t   signed_int_fastest;
			typedef uint_fast64_t unsigned_int_fastest;
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
}}

#endif
