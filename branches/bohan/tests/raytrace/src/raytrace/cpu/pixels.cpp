// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2009-2009 psycledelics http://psycle.pastnotecut.org : johan boule

#include "pixels.hpp"
#include <cassert>

namespace raytrace {

pixels::pixels(unsigned int width, unsigned int height) :
	width_(width),
	height_(height),
	row_stride_(width * sizeof *data_.words)
{
	data_.bytes = new char[width * height * sizeof *data_.words];
}

void pixels::fill(color c) {
	for(unsigned int x = 0; x < width_; ++x)
		for(unsigned int y = 0; y < height_; ++y)
			put(x, y, c);
}

}
