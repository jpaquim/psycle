// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2004-2009 members of the psycle project http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>

#ifndef PSYCLE__HELPERS__MATH__REMAINDER__INCLUDED
#define PSYCLE__HELPERS__MATH__REMAINDER__INCLUDED
#pragma once

#include <universalis/compiler.hpp>
#include <cmath>

namespace psycle { namespace helpers { namespace math {

/// computes a modulo with floating point numbers, like "numerator % denominator" would do with integers
template<typename Real> UNIVERSALIS__COMPILER__CONST
Real inline remainder(Real const & restrict numerator, Real const & restrict denominator) {
	Real result(numerator);
	if(result >= denominator) result -= denominator;
	if(result >= denominator) result = std::fmod(result, denominator);
	return result;
}

}}}

#endif
