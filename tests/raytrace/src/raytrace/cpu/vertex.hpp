// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2009-2009 psycledelics http://psycle.pastnotecut.org : johan boule

#ifndef RAYTRACE__VERTEX
#define RAYTRACE__VERTEX
#pragma once

#include "real.hpp"

namespace raytrace {

class vertex {
	public:
		vertex() {}
		vertex(real x, real y, real z) : x(x), y(y), z(z) {}
		real x, y, z;
};

}

#endif
