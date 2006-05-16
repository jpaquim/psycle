/***************************************************************************
 *   Copyright (C) 2006 johan boule <bohan@jabber.org>   *
 *   Copyright (C) 2006 psycledelics development team <http://psycle.sourceforge.net>   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

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
