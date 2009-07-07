// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2009-2009 psycledelics http://psycle.pastnotecut.org : johan boule

#ifndef RAYTRACE__SCENE
#define RAYTRACE__SCENE
#pragma once

#include "color.hpp"

namespace raytrace {

class scene {
	public:
		color virtual trace(unsigned int x, unsigned int y) = 0;
};

}

#endif
