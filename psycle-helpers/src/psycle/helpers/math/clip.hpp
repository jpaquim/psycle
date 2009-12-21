// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2004-2009 members of the psycle project http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>

#ifndef PSYCLE__HELPERS__MATH__CLIP__INCLUDED
#define PSYCLE__HELPERS__MATH__CLIP__INCLUDED
#pragma once

#include "rint.hpp"
#include <universalis/compiler.hpp>
#include <limits>
#include <boost/static_assert.hpp>
namespace psycle { namespace helpers { namespace math {

/// ensures a value stays between two bounds
template<typename X> UNIVERSALIS__COMPILER__CONST
X inline clipped(X const & minimum, X const & value, X const & maximum) {
	// it looks a bit dumb to write a function to do that code,
	// but maybe someone will find an optimised way to do this.
	return value < minimum ? minimum : value > maximum ? maximum : value;
}

/// combines float to signed integer conversion with clipping.
template<typename SignedIntegralResult, const unsigned int bits, typename Real> UNIVERSALIS__COMPILER__CONST
SignedIntegralResult inline clipped_lrint(Real x) {
	// check that Result is signed
	BOOST_STATIC_ASSERT((std::numeric_limits<SignedIntegralResult>::is_signed));
	BOOST_STATIC_ASSERT((std::numeric_limits<SignedIntegralResult>::is_integer));

	int const max((1u << (bits - 1)) - 1); // The compiler is able to compute this statically.
	int const min(-max - 1);
	return lrint<SignedIntegralResult>(clipped(Real(min), x, Real(max)));
}

/// combines float to signed integer conversion with clipping.
template<typename SignedIntegralResult, typename Real> UNIVERSALIS__COMPILER__CONST
SignedIntegralResult inline clipped_lrint(Real x) {
	return clipped_lrint<SignedIntegralResult, (sizeof(SignedIntegralResult) << 3)>(x);
}

}}}

#endif
