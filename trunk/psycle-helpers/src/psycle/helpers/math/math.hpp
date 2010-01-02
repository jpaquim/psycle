// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2004-2009 members of the psycle project http://psycle.sourceforge.net

#ifndef PSYCLE__HELPERS__MATH__MATH__INCLUDED
#define PSYCLE__HELPERS__MATH__MATH__INCLUDED
#pragma once

#include "constants.hpp"
#include "log.hpp"
#include "sin.hpp"
#include "sincos.hpp"
#include "sinseq.hpp"
#include "lrint.hpp"
#include "round.hpp"
#include "clip.hpp"
#include "erase_all_nans_infinities_and_denormals.hpp"
#include "erase_denormals.hpp"
#include <universalis/compiler.hpp>
#include <cmath>

/*************************************************************************************************

Summary of some of the C99 functions that do floating point rounding and conversions to integers:

"(int)" means it returns an integral number.
"(float)" means it returns a floating point number.

round to nearest integer, using the current rounding direction:
	(float) rint, rintf, rintl - raise the inexact exception when the result differs in value from the argument
	(float) nearbyint, nearbyintf, nearbyintl - don't raise the inexact exception
	(int) lrint, lrintf, lrintl, llrint, llrintf, llrintl
round to nearest integer, away from zero for halfway cases:
	(float) round, roundf, roundl
	(int) lround, lroundf, lroundl, llround, llroundf, llroundl
round to integer, towards zero:
	(float) trunc, truncf, truncl
	(int) static_cast, c-style cast, constructor-style cast
round to integer, towards -inf:
	(float) floor, floorf, floorl
round to integer, towards +inf:
	(float) ceil, ceilf, ceill
remainder/modulo:
	(float) fmod, fmodf, fmodl - quotient rounded towards zero to an integer
	(float) remainder, remainderf, remainderl - quotient rounded to the nearest integer.

*************************************************************************************************/

namespace psycle { namespace helpers { namespace math {

/// compares two floating point numbers for rough equality (difference less than epsilon by default).
template<typename Real> UNIVERSALIS__COMPILER__CONST
bool roughly_equals(Real const & a, Real const & b, Real const & tolerance = std::numeric_limits<Real>::epsilon()) {
	return std::abs(a - b) < tolerance;
}

/// compile-time factorial.
template<unsigned int i>
struct compile_time_factorial {
	unsigned int const static value = i * compile_time_factorial<i - 1>::value;
	BOOST_STATIC_ASSERT(value > 0); // makes constant overflows errors, not just warnings
};
///\internal template specialisation for compile-time factorial of 0.
template<> struct compile_time_factorial<0u> { unsigned int const static value = 1; };

template<typename Real> UNIVERSALIS__COMPILER__CONST
Real inline deci_bell_to_linear(Real const & deci_bell) {
	return std::pow(10u, deci_bell / 20);
}

template<typename Real> UNIVERSALIS__COMPILER__CONST
Real inline linear_to_deci_bell(Real const & linear) {
	return 20 * std::log10(linear);
}

}}}

#endif
