// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2009-2009 psycledelics http://psycle.pastnotecut.org : johan boule

#ifndef RAYTRACE__VERTEX
#define RAYTRACE__VERTEX
#pragma once

#include "real.hpp"
#include <cmath>

namespace raytrace {

class vertex {
	public:
		real x, y, z;

		vertex() {}

		vertex(real x, real y, real z) : x(x), y(y), z(z) {}

		vertex(vertex const & other) : x(other.x), y(other.y), z(other.z) {}
		
		vertex & operator =(vertex const & other) {
			this->x = other.x;
			this->y = other.y;
			this->z = other.z;
			return *this;
		}

		vertex operator +(vertex const & other) const {
			vertex v;
			v.x = this->x + other.x;
			v.y = this->y + other.y;
			v.z = this->z + other.z;
			return v;
		}

		vertex operator -(vertex const & other) const {
			vertex v;
			v.x = this->x - other.x;
			v.y = this->y - other.y;
			v.z = this->z - other.z;
			return v;
		}

		vertex & operator +=(vertex const & other) {
			this->x += other.x;
			this->y += other.y;
			this->z += other.z;
			return *this;
		}

		vertex & operator -=(vertex const & other) {
			this->x -= other.x;
			this->y -= other.y;
			this->z -= other.z;
			return *this;
		}

		vertex operator -() const {
			vertex v;
			v.x = -this->x;
			v.y = -this->y;
			v.z = -this->z;
			return v;
		}

		vertex operator *(real r) const {
			vertex v;
			v.x = this->x * r;
			v.y = this->y * r;
			v.z = this->z * r;
			return v;
		}

		vertex & operator *=(real r) {
			this->x *= r;
			this->y *= r;
			this->z *= r;
			return *this;
		}

		real operator *(vertex const & other) const {
			real r;
			r = this->x * other.x + this->y * other.y + this->z * other.z;
			return r;
		}

		vertex operator ^(vertex const & other) const {
			vertex v;
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

}

#endif
