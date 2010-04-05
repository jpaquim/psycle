// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2009-2009 psycledelics http://psycle.pastnotecut.org : johan boule

#ifndef RAYTRACE__PIXELS
#define RAYTRACE__PIXELS
#pragma once

#include "color.hpp"

#ifdef RAYTRACE__GL
	#include <GL/gl.h>
#else
	#include <gdkmm/pixbuf.h>
#endif

#include <universalis/stdlib/cstdint.hpp>
#include <psycle/helpers/math.hpp>
#include <cassert>

namespace raytrace {

class pixels {
	public:
		pixels(unsigned int width, unsigned int height);
		unsigned int width() const { return width_; }
		unsigned int height() const { return height_; }
		void fill(color c);
		void inline put(unsigned int x, unsigned int y, color c);

	public:
		operator char*() { return data_.bytes; }
	
	private:
		unsigned int width_, height_, row_stride_;
		typedef uint32_t rgba;
		union raw_data {
			char * bytes;
			rgba * words;
		} data_;
		rgba inline quantize(color c, uint8_t a = 0xff);
};

void pixels::put(unsigned int x, unsigned int y, color c) {
	raw_data p;
	p.bytes = data_.bytes + y * row_stride_ + x * sizeof *p.words;
	*p.words = quantize(c, p.bytes[3]);
}

pixels::rgba pixels::quantize(color c, uint8_t a) {
	union raw_datum {
		rgba word;
		uint8_t bytes[sizeof(rgba)];
	} datum;
	using psycle::helpers::math::lrint;
	datum.bytes[0] = lrint<uint8_t>(0xff * c.x);
	datum.bytes[1] = lrint<uint8_t>(0xff * c.y);
	datum.bytes[2] = lrint<uint8_t>(0xff * c.z);
	datum.bytes[3] = a;
	return datum.word;
}

}

#endif
