// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2011 members of the psycle project http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>

#ifndef PSYCLE__HELPERS__MATH_H
#define PSYCLE__HELPERS__MATH_H
#pragma once

#include "erase_all_nans_infinities_and_denormals.hpp"
#include "../../restrict.hpp"
#include <cmath>

namespace psycle { namespace helpers { namespace math {

double const pi = 3.14159265358979323846;

/******************************************************************************************/
/// sincos - computes both the sine and the cosine at the same time
template<typename Real>
void inline sincos(
	Real x,
	Real& UNIVERSALIS__COMPILER__RESTRICT_REF sine,
	Real& UNIVERSALIS__COMPILER__RESTRICT_REF cosine
) noexcept {
	// some compilers are able to optimise those two calls into one.
	sine = std::sin(x);
	cosine = std::cos(x);
}

}}}

#endif /* PSYCLE__HELPERS__MATH_H */

