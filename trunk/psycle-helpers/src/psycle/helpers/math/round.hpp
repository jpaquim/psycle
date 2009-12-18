// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2004-2009 members of the psycle project http://psycle.sourceforge.net

#ifndef PSYCLE__HELPERS__MATH__ROUND__INCLUDED
#define PSYCLE__HELPERS__MATH__ROUND__INCLUDED
#pragma once

#include <diversalis/stdlib.hpp>
#if DIVERSALIS__STDLIB__MATH < 199901
	#include <diversalis/compiler.hpp>
	#include <diversalis/cpu.hpp>
#endif
#include <universalis/compiler.hpp>
#include <universalis/stdlib/cstdint.hpp>
#include <cmath>
#include "rint.hpp"

namespace psycle { namespace helpers { namespace math {

/// converts a floating point number to an integer by rounding to the nearest integer.
/// note: it is unspecified whether rounding x.5 rounds up or down.
template<typename Real> UNIVERSALIS__COMPILER__CONST
std::int32_t inline rounded(Real x) {
	return x > 0 ? std::floor(x + Real(0.5)) : std::ceil(x - Real(0.5));
}

#if DIVERSALIS__STDLIB__MATH >= 199901
	
	template<> UNIVERSALIS__COMPILER__CONST
	std::int32_t inline rounded<>(long double ld) {
		return lrint<std::int32_t>(::roundl(ld));
	}

	template<> UNIVERSALIS__COMPILER__CONST
	std::int32_t inline rounded<>(double d) {
		return lrint<std::int32_t>(::round(d));
	}

	template<> UNIVERSALIS__COMPILER__CONST
	std::int32_t inline rounded<>(float f) {
		return lrint<std::int32_t>(::roundf(f));
	}

#else

	template<> UNIVERSALIS__COMPILER__CONST
	std::int32_t inline rounded<>(double d) {
		BOOST_STATIC_ASSERT((sizeof d == 8));
		union result_union
		{
			double d;
			std::int32_t i;
		} result;
		result.d = d + 6755399441055744.0; // 2^51 + 2^52
		return result.i;
	}
	
	template<> UNIVERSALIS__COMPILER__CONST
	std::int32_t inline rounded<>(float f) {
		return rounded(double(f));
	}

#endif

#if defined BOOST_AUTO_TEST_CASE
	BOOST_AUTO_TEST_CASE(rounded_test) {
		BOOST_CHECK(rounded(+2.6) == +3);
		BOOST_CHECK(rounded(+1.4) == +1);
		BOOST_CHECK(rounded(-2.6) == -3);
		BOOST_CHECK(rounded(-1.4) == -1);
		BOOST_CHECK(rounded(+2.6f) == +3);
		BOOST_CHECK(rounded(+1.4f) == +1);
		BOOST_CHECK(rounded(-2.6f) == -3);
		BOOST_CHECK(rounded(-1.4f) == -1);
	}
#endif

}}}

#endif
