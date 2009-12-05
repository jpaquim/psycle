// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2009 members of the psycle project http://psycle.sourceforge.net

#ifndef PSYCLE__HELPERS__MATH__ERASE_DENORMALS__INCLUDED
#define PSYCLE__HELPERS__MATH__ERASE_DENORMALS__INCLUDED
#pragma once

namespace psycle { namespace helpers { namespace math {

inline float erase_denormals(float x) {
	x += 1.0e-30f;
	x -= 1.0e-30f;
	return x;
}

inline double erase_denormals(double x) {
	x += 1.0e-291;
	x -= 1.0e-291;
	return x;
}

inline float erase_denormals_inplace(float & x) {
	return x = erase_denormals(x);
}

inline double erase_denormals_inplace(double & x) {
	return x = erase_denormals(x);
}

inline float fast_erase_denormals(float x) {
	return x + 1.0e-30f;
}

inline double fast_erase_denormals(double x) {
	return x + 1.0e-291;
}

inline float fast_erase_denormals_inplace(float & x) {
	return x += 1.0e-30f;
}

inline double fast_erase_denormals_inplace(double & x) {
	return x += 1.0e-291;
}

}}}

#endif
