// -*- mode:c++; indent-tabs-mode:t -*-
// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2004-2008 members of the psycle project http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>

#pragma once
#include <cmath>
namespace psycle { namespace helpers { namespace math {

/// the pi constant as a 64-bit floating point number
double const pi(
	#if defined M_PI
		M_PI
	#else
		3.14159265358979323846264338327950288
	#endif
);

/// the pi constant as a 32-bit floating point number
float const pi_f(float(pi));

}}}
