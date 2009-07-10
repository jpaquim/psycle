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
		unsigned int width() const { return pixbuf_->get_width(); }
		unsigned int height() const { return pixbuf_->get_height(); }
		void fill(color c) { pixbuf_->fill(quantize(c)); }
		void inline put(unsigned int x, unsigned int y, color c);

	public:
		operator Glib::RefPtr<Gdk::Pixbuf>() { return pixbuf_; }
	private:
		Glib::RefPtr<Gdk::Pixbuf> pixbuf_;
	
	private:
		unsigned int row_stride_;
		typedef std::uint32_t rgba;
		union raw_data {
			std::uint8_t * bytes;
			rgba * words;
		} data_;
		rgba inline quantize(color c, std::uint8_t a = 0xff);
};

pixels::rgba pixels::quantize(color c, std::uint8_t a) {
	union raw_datum {
		std::uint8_t bytes[4];
		rgba word;
	} datum;
	datum.bytes[0] = 0xff * c.x;
	datum.bytes[1] = 0xff * c.y;
	datum.bytes[2] = 0xff * c.z;
	datum.bytes[3] = a;
	return datum.word;
}

void pixels::put(unsigned int x, unsigned int y, color c) {
	raw_data p;
	p.bytes = data_.bytes + y * row_stride_ + x * 3;
	*p.words = quantize(c, p.bytes[3]);
}

}

#endif
