// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2006 johan boule <bohan@jabber.org>
// copyright 2006 psycledelics development team <http://psycle.sourceforge.net>

#include "color_converter.hpp"

namespace ngrs
{
	// everything is inlined
}

#if defined NGRS__COLOR_CONVERTER__TEST
	#include <cassert>
	int main()
	{
		ngrs::color_converter<> cc(0x07, 0x38, 0xc0); // bgr233
		assert(cc(0xff, 0xff, 0xff) == 0xff);
		assert(cc(0xff, 0x00, 0x00) == 0x07);
		assert(cc(0x00, 0xff, 0x00) == 0x38);
		assert(cc(0x00, 0x00, 0xff) == 0xc0);
		assert(cc(0x80, 0x80, 0x80) == 0xa4);
		assert(cc(0x80, 0x00, 0x00) == 0x04);
		assert(cc(0x00, 0x80, 0x00) == 0x20);
		assert(cc(0x00, 0x00, 0x80) == 0x80);
		return 0;
	}
#endif
