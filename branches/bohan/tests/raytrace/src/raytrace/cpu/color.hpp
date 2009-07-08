// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2009-2009 psycledelics http://psycle.pastnotecut.org : johan boule

#ifndef RAYTRACE__COLOR
#define RAYTRACE__COLOR
#pragma once

#include "vertex3.hpp"

namespace raytrace {

class color : public vertex3 {
	public:
		color() {}

		color(real x, real y, real z) : vertex3(x, y, z) {}

		color(color const & other) : vertex3(other) {}
};

}

#endif
