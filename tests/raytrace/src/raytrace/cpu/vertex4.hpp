// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2009-2009 psycledelics http://psycle.pastnotecut.org : johan boule

#ifndef RAYTRACE__VERTEX4
#define RAYTRACE__VERTEX4
#pragma once

#include "real.hpp"
#include "vertex3.hpp"
#include <cmath>

namespace raytrace {

class vertex4 {
	public:
		real x, y, z, w;
		
		vertex4() {}

		vertex4(real x, real y, real z, real w) : x(x), y(y), z(z), w(w) {}
		
		vertex4(vertex4 const & other) : x(other.x), y(other.y), z(other.z), w(other.w) {}

		vertex4(vertex3 const & v): x(v.x), y(v.y), z(v.z), w(1) {}
		
		vertex4 & operator =(vertex4 const & other) {
			this->x = other.x;
			this->y = other.y;
			this->z = other.z;
			this->w = other.w;
			return *this;
		}
		
		void operator()(real x, real y, real z) {
			this->x = x;
			this->y = y;
			this->z = z;
			this->w = w;
		}

		vertex4 operator +(vertex4 const & other) const {
			vertex4 v;
			v.x = this->x + other.x;
			v.y = this->y + other.y;
			v.z = this->z + other.z;
			v.w = this->w + other.w;
			return v;
		}

		vertex4 operator -(vertex4 const & other) const {
			vertex4 v;
			v.x = this->x - other.x;
			v.y = this->y - other.y;
			v.z = this->z - other.z;
			v.w = this->w - other.w;
			return v;
		}

		vertex4 & operator +=(vertex4 const & other) {
			this->x += other.x;
			this->y += other.y;
			this->z += other.z;
			this->w += other.w;
			return *this;
		}

		vertex4 & operator -=(vertex4 const & other) {
			this->x -= other.x;
			this->y -= other.y;
			this->z -= other.z;
			this->w -= other.w;
			return *this;
		}

		vertex4 operator -() const {
			vertex4 v;
			v.x = -this->x;
			v.y = -this->y;
			v.z = -this->z;
			v.w = -this->w;
			return v;
		}

		vertex4 operator *(real r) const {
			vertex4 v;
			v.x = this->x * r;
			v.y = this->y * r;
			v.z = this->z * r;
			v.w = this->w * r;
			return v;
		}

		vertex4 & operator *=(real r) {
			this->x *= r;
			this->y *= r;
			this->z *= r;
			this->w *= r;
			return *this;
		}

		real operator *(vertex4 const & other) const {
			real r;
			r = this->x * other.x + this->y * other.y + this->z * other.z + this->w * other.w;
			return r;
		}
};

vertex4 inline operator *(real r, vertex4 const & v) {
	vertex4 result;
	result = v * r;
	return result;
}

}

#endif
