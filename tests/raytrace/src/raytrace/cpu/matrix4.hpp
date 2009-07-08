// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2009-2009 psycledelics http://psycle.pastnotecut.org : johan boule

#ifndef RAYTRACE__MATRIX4
#define RAYTRACE__MATRIX4
#pragma once

#include "matrix.hpp"
#include "real.hpp"
#include "vertex4.hpp"
#include "vertex3.hpp"

namespace raytrace {

typedef matrix<real, 4, 4> matrix4;

matrix4 inline operator *(matrix4::value_type r, matrix4 const & m) {
	matrix4 result;
	result = m * r;
	return result;
}

vertex4 inline operator *(matrix4 const & m, vertex4 const & v) {
	vertex4 result;
	result.x = v.x * m(0, 0) + v.y * m(1, 0) + v.z * m(2, 0) + v.w * m(3, 0);
	result.y = v.x * m(0, 1) + v.y * m(1, 1) + v.z * m(2, 1) + v.w * m(3, 1);
	result.z = v.x * m(0, 2) + v.y * m(1, 2) + v.z * m(2, 2) + v.w * m(3, 2);
	result.w = v.x * m(0, 3) + v.y * m(1, 3) + v.z * m(2, 3) + v.w * m(3, 3);
	return result;
}

vertex4 inline operator *(matrix4 const & m, vertex3 const & v) {
	vertex4 result;
	result.x = v.x * m(0, 0) + v.y * m(0, 1) + v.z * m(0, 2) + m(0, 3);
	result.y = v.x * m(1, 0) + v.y * m(1, 1) + v.z * m(1, 2) + m(1, 3);
	result.z = v.x * m(2, 0) + v.y * m(2, 1) + v.z * m(2, 2) + m(2, 3);
	result.w = v.x * m(3, 0) + v.y * m(3, 1) + v.z * m(3, 2) + m(3, 3);
	return result;
}

vertex4 inline operator *(vertex4 const & v, matrix4 const & m) {
	vertex4 result;
	result.x = v.x * m(0, 0) + v.y * m(0, 1) + v.z * m(0, 2) + v.w * m(0, 3);
	result.y = v.x * m(1, 0) + v.y * m(1, 1) + v.z * m(1, 2) + v.w * m(1, 3);
	result.z = v.x * m(2, 0) + v.y * m(2, 1) + v.z * m(2, 2) + v.w * m(2, 3);
	result.w = v.x * m(3, 0) + v.y * m(3, 1) + v.z * m(3, 2) + v.w * m(3, 3);
	return result;
}

vertex4 inline operator *(vertex3 const & v, matrix4 const & m) {
	vertex4 result;
	result.x = v.x * m(0, 0) + v.y * m(0, 1) + v.z * m(0, 2) + m(0, 3);
	result.y = v.x * m(1, 0) + v.y * m(1, 1) + v.z * m(1, 2) + m(1, 3);
	result.z = v.x * m(2, 0) + v.y * m(2, 1) + v.z * m(2, 2) + m(2, 3);
	result.w = v.x * m(3, 0) + v.y * m(3, 1) + v.z * m(3, 2) + m(3, 3);
	return result;
}

}

#endif
