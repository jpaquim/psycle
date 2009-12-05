// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2004-2009 members of the psycle project http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>

#ifndef PSYCLE__HELPERS__MATH__CLIP__INCLUDED
#define PSYCLE__HELPERS__MATH__CLIP__INCLUDED
#pragma once

#include <universalis/compiler.hpp>
namespace psycle { namespace helpers { namespace math {

/// ensures a value stays between two bounds
template<typename Real> UNIVERSALIS__COMPILER__CONST
Real inline clipped(Real const & minimum, Real const & value, Real const & maximum) {
	// it looks a bit dumb to write a function to do that code,
	// but maybe someone will find an optimised way to do this.
	if(minimum > value) return minimum;
	else if(value > maximum) return maximum;
	else return value;
}

}}}

#endif
