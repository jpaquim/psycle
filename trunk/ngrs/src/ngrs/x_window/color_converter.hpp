// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2006 johan boule <bohan@jabber.org>
// copyright 2006 psycledelics development team <http://psycle.sourceforge.net>

#include <ngrs/color_converter.hpp>

namespace ngrs
{
	namespace x_window
	{	
		// instanciate a version of the color_converter template suitable for X Window (default parameters are fine)
		typedef ngrs::color_converter<> color_converter;
	}
	
}
