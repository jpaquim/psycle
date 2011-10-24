// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2004-2011 members of the psycle project http://psycle.sourceforge.net

#ifndef PSYCLE__HELPERS__MATH__MATH__INCLUDED
#define PSYCLE__HELPERS__MATH__MATH__INCLUDED
#pragma once

#include "constants.hpp"
#include "log.hpp"
#include "sin.hpp"
#include "sincos.hpp"
#include "sinseq.hpp"
#include "lrint.hpp"
#include "lround.hpp"
#include "clip.hpp"
#include "erase_all_nans_infinities_and_denormals.hpp"
#include "erase_denormals.hpp"
#include <universalis.hpp>
#include <cmath>

/*************************************************************************************************

Summary of some of the C99 functions that do floating point rounding and conversions to integers:

"(int)" means it returns an integral number.
"(float)" means it returns a floating point number.

round to integer, using the current rounding direction (fesetround FE_TONEAREST, FE_TOWARDZERO, FE_DOWNWARD, FE_UPWARD):
	(float) rint, rintf, rintl - raise the inexact exception when the result differs in value from the argument
	(float) nearbyint, nearbyintf, nearbyintl - don't raise the inexact exception
	(int) lrint, lrintf, lrintl, llrint, llrintf, llrintl
round to nearest integer, away from zero for halfway cases (fesetround FE_TONEAREST):
	(float) round, roundf, roundl
	(int) lround, lroundf, lroundl, llround, llroundf, llroundl
round to integer, towards zero (fesetround FE_TOWARDZERO):
	(float) trunc, truncf, truncl
	(int) static_cast, c-style cast, constructor-style cast
round to integer, towards -inf (fesetround FE_DOWNWARD):
	(float) floor, floorf, floorl
round to integer, towards +inf (fesetround FE_UPWARD):
	(float) ceil, ceilf, ceill
remainder/modulo:
	(float) fmod, fmodf, fmodl - quotient rounded towards zero to an integer
	(float) remainder, remainderf, remainderl - quotient rounded to the nearest integer.

*************************************************************************************************/

namespace psycle { namespace helpers { namespace math {

/// compares two floating point numbers for rough equality (difference less than epsilon by default).
template<typename Real> UNIVERSALIS__COMPILER__CONST
bool inline roughly_equals(Real a, Real b, Real tolerance = std::numeric_limits<Real>::epsilon()) {
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
Real inline deci_bell_to_linear(Real deci_bell) {
	///\todo merge with psycle::helpers::dsp::dB2Amp
	return std::pow(10u, deci_bell / Real(20));
}

template<typename Real> UNIVERSALIS__COMPILER__CONST
Real inline linear_to_deci_bell(Real linear) {
	///\todo merge with psycle::helpers::dsp::dB
	return Real(20) * std::log10(linear);
}

}}}

#endif
