// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2004-2011 members of the psycle project http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>

#ifndef PSYCLE__HELPERS__MATH__ROUND__INCLUDED
#define PSYCLE__HELPERS__MATH__ROUND__INCLUDED
#pragma once

#include <diversalis.hpp>
#include <cmath>
#include <limits>

namespace psycle { namespace helpers { namespace math {

/// converts a floating point number to an integer by rounding to the nearest integer.
/// note: it is unspecified whether rounding x.5 rounds up or down.
template<typename IntegralResult, typename Real> UNIVERSALIS__COMPILER__CONST_FUNCTION
IntegralResult inline round(Real x) {
	// check that the Result type is an integral number
	BOOST_STATIC_ASSERT((std::numeric_limits<IntegralResult>::is_integer));

	return x > 0 ? std::floor(x + Real(0.5)) : std::ceil(x - Real(0.5));
}

#if DIVERSALIS__STDLIB__MATH >= 199901
	
	// signed long long int

	template<> UNIVERSALIS__COMPILER__CONST_FUNCTION signed long long int inline round<>(long double ld) { return ::llroundl(ld); }
	template<> UNIVERSALIS__COMPILER__CONST_FUNCTION signed long long int inline round<>(double d) { return ::llround(d); }
	template<> UNIVERSALIS__COMPILER__CONST_FUNCTION signed long long int inline round<>(float f) { return ::llroundf(f); }

	// unsigned long long int

	template<> UNIVERSALIS__COMPILER__CONST_FUNCTION unsigned long long int inline round<>(long double ld) { return ::llroundl(ld); }
	template<> UNIVERSALIS__COMPILER__CONST_FUNCTION unsigned long long int inline round<>(double d) { return ::llround(d); }
	template<> UNIVERSALIS__COMPILER__CONST_FUNCTION unsigned long long int inline round<>(float f) { return ::llroundf(f); }

	// signed long int

	template<> UNIVERSALIS__COMPILER__CONST_FUNCTION signed long int inline round<>(long double ld) { return ::lroundl(ld); }
	template<> UNIVERSALIS__COMPILER__CONST_FUNCTION signed long int inline round<>(double d) { return ::lround(d); }
	template<> UNIVERSALIS__COMPILER__CONST_FUNCTION signed long int inline round<>(float f) { return ::lroundf(f); }

	// unsigned long int

	template<> UNIVERSALIS__COMPILER__CONST_FUNCTION unsigned long int inline round<>(long double ld) { return ::lroundl(ld); }
	template<> UNIVERSALIS__COMPILER__CONST_FUNCTION unsigned long int inline round<>(double d) { return ::lround(d); }
	template<> UNIVERSALIS__COMPILER__CONST_FUNCTION unsigned long int inline round<>(float f) { return ::lroundf(f); }

	// signed int

	template<> UNIVERSALIS__COMPILER__CONST_FUNCTION signed int inline round<>(long double ld) { return ::lroundl(ld); }
	template<> UNIVERSALIS__COMPILER__CONST_FUNCTION signed int inline round<>(double d) { return ::lround(d); }
	template<> UNIVERSALIS__COMPILER__CONST_FUNCTION signed int inline round<>(float f) { return ::lroundf(f); }

	// unsigned int

	template<> UNIVERSALIS__COMPILER__CONST_FUNCTION unsigned int inline round<>(long double ld) { return ::lroundl(ld); }
	template<> UNIVERSALIS__COMPILER__CONST_FUNCTION unsigned int inline round<>(double d) { return ::lround(d); }
	template<> UNIVERSALIS__COMPILER__CONST_FUNCTION unsigned int inline round<>(float f) { return ::lroundf(f); }

	// signed short

	template<> UNIVERSALIS__COMPILER__CONST_FUNCTION signed short inline round<>(long double ld) { return ::lroundl(ld); }
	template<> UNIVERSALIS__COMPILER__CONST_FUNCTION signed short inline round<>(double d) { return ::lround(d); }
	template<> UNIVERSALIS__COMPILER__CONST_FUNCTION signed short inline round<>(float f) { return ::lroundf(f); }

	// unsigned short

	template<> UNIVERSALIS__COMPILER__CONST_FUNCTION unsigned short inline round<>(long double ld) { return ::lroundl(ld); }
	template<> UNIVERSALIS__COMPILER__CONST_FUNCTION unsigned short inline round<>(double d) { return ::lround(d); }
	template<> UNIVERSALIS__COMPILER__CONST_FUNCTION unsigned short inline round<>(float f) { return ::lroundf(f); }

	// signed char

	template<> UNIVERSALIS__COMPILER__CONST_FUNCTION signed char inline round<>(long double ld) { return ::lroundl(ld); }
	template<> UNIVERSALIS__COMPILER__CONST_FUNCTION signed char inline round<>(double d) { return ::lround(d); }
	template<> UNIVERSALIS__COMPILER__CONST_FUNCTION signed char inline round<>(float f) { return ::lroundf(f); }

	// unsigned char

	template<> UNIVERSALIS__COMPILER__CONST_FUNCTION unsigned char inline round<>(long double ld) { return ::lroundl(ld); }
	template<> UNIVERSALIS__COMPILER__CONST_FUNCTION unsigned char inline round<>(double d) { return ::lround(d); }
	template<> UNIVERSALIS__COMPILER__CONST_FUNCTION unsigned char inline round<>(float f) { return ::lroundf(f); }

	// char

	template<> UNIVERSALIS__COMPILER__CONST_FUNCTION char inline round<>(long double ld) { return ::lroundl(ld); }
	template<> UNIVERSALIS__COMPILER__CONST_FUNCTION char inline round<>(double d) { return ::lround(d); }
	template<> UNIVERSALIS__COMPILER__CONST_FUNCTION char inline round<>(float f) { return ::lroundf(f); }

#endif

/******************************************************************************************/
#ifdef BOOST_AUTO_TEST_CASE
	namespace test {
		BOOST_AUTO_TEST_CASE(round_test) {
			BOOST_CHECK(round<long int>(+2.6) == +3);
			BOOST_CHECK(round<long int>(+1.4) == +1);
			BOOST_CHECK(round<long int>(-2.6) == -3);
			BOOST_CHECK(round<long int>(-1.4) == -1);
			BOOST_CHECK(round<long int>(+2.6f) == +3);
			BOOST_CHECK(round<long int>(+1.4f) == +1);
			BOOST_CHECK(round<long int>(-2.6f) == -3);
			BOOST_CHECK(round<long int>(-1.4f) == -1);
		}
	}
#endif

}}}

#endif
