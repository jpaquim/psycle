// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2009-2009 psycledelics http://psycle.pastnotecut.org : johan boule

#ifndef RAYTRACE__PIXELS
#define RAYTRACE__PIXELS
#pragma once

#include "color.hpp"
#include <gdkmm/pixbuf.h>
#include <cstdint>
#include <cassert>

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
		union raw_data {
			std::uint8_t * bytes;
			color * c;
		} data_;
		unsigned int row_stride_;
};

void pixels::put(unsigned int x, unsigned int y, color c) {
	raw_data p;
	p.bytes = data_.bytes + y * row_stride_ + x * 3;
	union raw_datum {
		std::uint8_t bytes[4];
		color c;
	} datum;
	datum.c = c;
	datum.bytes[3] = p.bytes[3];
	*p.c = datum.c;
}

}

#endif
