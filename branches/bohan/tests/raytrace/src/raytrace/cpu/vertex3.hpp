// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2009-2009 psycledelics http://psycle.pastnotecut.org : johan boule

#ifndef RAYTRACE__VERTEX3
#define RAYTRACE__VERTEX3
#pragma once

#include "real.hpp"
#include <cmath>

namespace raytrace {

class vertex3 {
	public:
		real x, y, z;

		vertex3() {}

		vertex3(real x, real y, real z) : x(x), y(y), z(z) {}

		vertex3(vertex3 const & other) : x(other.x), y(other.y), z(other.z) {}
		
		vertex3 & operator =(vertex3 const & other) {
			this->x = other.x;
			this->y = other.y;
			this->z = other.z;
			return *this;
		}
		
		void operator()(real x, real y, real z) {
			this->x = x;
			this->y = y;
			this->z = z;
		}

		vertex3 operator +(vertex3 const & other) const {
			vertex3 v;
			v.x = this->x + other.x;
			v.y = this->y + other.y;
			v.z = this->z + other.z;
			return v;
		}

		vertex3 operator -(vertex3 const & other) const {
			vertex3 v;
			v.x = this->x - other.x;
			v.y = this->y - other.y;
			v.z = this->z - other.z;
			return v;
		}

		vertex3 & operator +=(vertex3 const & other) {
			this->x += other.x;
			this->y += other.y;
			this->z += other.z;
			return *this;
		}

		vertex3 & operator -=(vertex3 const & other) {
			this->x -= other.x;
			this->y -= other.y;
			this->z -= other.z;
			return *this;
		}

		vertex3 operator -() const {
			vertex3 v;
			v.x = -this->x;
			v.y = -this->y;
			v.z = -this->z;
			return v;
		}

		vertex3 operator *(real r) const {
			vertex3 v;
			v.x = this->x * r;
			v.y = this->y * r;
			v.z = this->z * r;
			return v;
		}

		vertex3 & operator *=(real r) {
			this->x *= r;
			this->y *= r;
			this->z *= r;
			return *this;
		}

		real operator *(vertex3 const & other) const {
			real r;
			r = this->x * other.x + this->y * other.y + this->z * other.z;
			return r;
		}

		vertex3 operator ^(vertex3 const & other) const {
			vertex3 v;
			v.x = this->y * other.z - this->z * other.y;
			v.y = this->z * other.x - this->x * other.z;
			v.z = this->x * other.y - this->y * other.x;
			return v;
		}
		
		real mag() const {
			real r;
			r = std::sqrt(mag2());
			return r;
		}
		
		real mag2() const {
			real r;
			r = x * x + y * y + z * z;
			return r;
		}

		void normalize() {
			*this *= 1 / mag();
		}
};

vertex3 inline operator *(real r, vertex3 const & v) {
	vertex3 result;
	result = v * r;
	return result;
}

}

#endif
