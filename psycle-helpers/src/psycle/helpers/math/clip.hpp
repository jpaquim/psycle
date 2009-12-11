// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2004-2009 members of the psycle project http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>

#ifndef PSYCLE__HELPERS__MATH__CLIP__INCLUDED
#define PSYCLE__HELPERS__MATH__CLIP__INCLUDED
#pragma once

#include <universalis/compiler.hpp>
namespace psycle { namespace helpers { namespace math {

/// ensures a value stays between two bounds
template<typename X> UNIVERSALIS__COMPILER__CONST
X inline clipped(X const & minimum, X const & value, X const & maximum) {
	// it looks a bit dumb to write a function to do that code,
	// but maybe someone will find an optimised way to do this.
	if(minimum > value) return minimum;
	else if(value > maximum) return maximum;
	else return value;
}


/// ensures a value stays between 16 bit boundaries
template<typename X> UNIVERSALIS__COMPILER__CONST
X inline clipped16(X const & value) {
	return clipped<X>(-32768, value, 32767);
}

/// ensures a value stays between 24 bit boundariess
template<typename X> UNIVERSALIS__COMPILER__CONST
X inline clipped24(X const & value) {
	return clipped<X>(-8388608, value, 8388607);
}


}}}

#endif
