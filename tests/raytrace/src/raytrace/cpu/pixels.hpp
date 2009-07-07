// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2009-2009 psycledelics http://psycle.pastnotecut.org : johan boule

#ifndef RAYTRACE__PIXELS
#define RAYTRACE__PIXELS
#pragma once

#include "color.hpp"
#include <gdkmm/pixbuf.h>

namespace raytrace {

class pixels {
	public:
		pixels(unsigned int width, unsigned int height);
		Glib::RefPtr<Gdk::Pixbuf> pixbuf() { return pixbuf_; }
		unsigned int width() const { return pixbuf_->get_width(); }
		unsigned int height() const { return pixbuf_->get_height(); }
		void fill(color c) { pixbuf_->fill(c); }
		void inline put(unsigned int x, unsigned int y, color);
	private:
		Glib::RefPtr<Gdk::Pixbuf> pixbuf_;
};

void pixels::put(unsigned int x, unsigned int y, color c) {
	guint8 * p = pixbuf_->get_pixels();
	p += y * pixbuf_->get_rowstride();
	p += x * pixbuf_->get_n_channels();
	union u {
		guint8 r, g, b, x;
		color c;
	} u;
	u.c = c;
	u.x = *(p + pixbuf_->get_n_channels());
	*reinterpret_cast<color* >(p) = c;
}

}

#endif
