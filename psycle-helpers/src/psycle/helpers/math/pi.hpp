// This program is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
// You should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
// copyright 2004-2009 members of the psycle project http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>

#ifndef PSYCLE__HELPERS__MATH__PI__INCLUDED
#define PSYCLE__HELPERS__MATH__PI__INCLUDED
#pragma once

#include <cmath>

namespace psycle { namespace helpers { namespace math {

/// the pi constant as a 64-bit floating point number
double const pi(
	#if defined M_PI
		M_PI
	#else
		3.14159265358979323846264338327950288
	#endif
);

/// the pi constant as a 32-bit floating point number
float const pi_f = float(pi);

}}}
#endif
