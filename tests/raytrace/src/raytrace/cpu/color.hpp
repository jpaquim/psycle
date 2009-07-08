// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2009-2009 psycledelics http://psycle.pastnotecut.org : johan boule

#ifndef RAYTRACE__COLOR
#define RAYTRACE__COLOR
#pragma once

#include "real.hpp"

namespace raytrace {

class color {
	public:
		color() {}
		color(real r, real g, real b) : r(r), g(g), b(b) {}
		real r, g, b;
};

}

#endif
