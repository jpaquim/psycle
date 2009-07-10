// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2009-2009 psycledelics http://psycle.pastnotecut.org : johan boule

#ifndef RAYTRACE__MATRIX3
#define RAYTRACE__MATRIX3
#pragma once

#include "matrix.hpp"
#include "real.hpp"
#include "vertex3.hpp"

namespace raytrace {

typedef matrix<real, 3> matrix3;

vertex3 inline operator *(matrix3 const & m, vertex3 const & v) {
	vertex3 result;
	result.x = v.x * m(0, 0) + v.y * m(1, 0) + v.z * m(2, 0);
	result.y = v.x * m(0, 1) + v.y * m(1, 1) + v.z * m(2, 1);
	result.z = v.x * m(0, 2) + v.y * m(1, 2) + v.z * m(2, 2);
	return result;
}

vertex3 inline operator *(vertex3 const & v, matrix3 const & m) {
	vertex3 result;
	result.x = v.x * m(0, 0) + v.y * m(0, 1) + v.z * m(0, 2);
	result.y = v.x * m(1, 0) + v.y * m(1, 1) + v.z * m(1, 2);
	result.z = v.x * m(2, 0) + v.y * m(2, 1) + v.z * m(2, 2);
	return result;
}

}

#endif
