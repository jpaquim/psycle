// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2009-2009 psycledelics http://psycle.pastnotecut.org : johan boule

#include "pixels.hpp"
#include <cassert>

namespace raytrace {

pixels::pixels(unsigned int width, unsigned int height) {
	pixbuf_ = Gdk::Pixbuf::create(Gdk::COLORSPACE_RGB, false, 8, width, height);
	assert(pixbuf_->get_n_channels() == 3);
	data_.bytes = pixbuf_->get_pixels();
	row_stride_ = pixbuf_->get_rowstride();
}

}