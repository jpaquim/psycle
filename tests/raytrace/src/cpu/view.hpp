// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2009-2009 psycledelics http://psycle.pastnotecut.org : johan boule

#ifndef RAYTRACE__VIEW
#define RAYTRACE__VIEW
#pragma once

#include "pixels.hpp"

namespace raytrace {

class view {
	public:
		view(pixels & pixels) : pixels_(pixels) {}
		void virtual update() = 0;
	private:
		pixels & pixels_;
};

}

#endif
