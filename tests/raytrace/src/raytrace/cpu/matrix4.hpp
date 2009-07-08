// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2009-2009 psycledelics http://psycle.pastnotecut.org : johan boule

#ifndef RAYTRACE__MATRIX4
#define RAYTRACE__MATRIX4
#pragma once

#include "real.hpp"

namespace raytrace {

class matrix4 {
	public:
		typedef unsigned int size_type;
		size_type static const size = 4;

		real operator()(size_type i, size_type j) const { return e[i][j]; }

		real & operator()(size_type i, size_type j) { return e[i][j]; }
		
		matrix4() {}

		matrix4(matrix4 const & other) : e(other.e) {}
		
		martix & operator =(matrix4 const & other) const { this->e = other.e; }
		
		matrix4 operator +(matrix4 const & other) const {
			matrix4 m;
			for(size_type i(0); i < size; ++i)
				for(size_type j(0); j < size; ++j)
					m.e[i][j] = this->e[i][j] + other.e[i][j];
			return m;
		}

		matrix4 operator -(matrix4 const & other) const {
			matrix4 m;
			for(size_type i(0); i < size; ++i)
				for(size_type j(0); j < size; ++j)
					m.e[i][j] = this->e[i][j] - other.e[i][j];
			return m;
		}

		matrix4 & operator +=(matrix4 const & other) {
			for(size_type i(0); i < size; ++i)
				for(size_type j(0); j < size; ++j)
					this->e[i][j] += other.e[i][j];
			return *this;
		}

		matrix4 & operator -=(matrix4 const & other) {
			for(size_type i(0); i < size; ++i)
				for(size_type j(0); j < size; ++j)
					this->e[i][j] -= other.e[i][j];
			return *this;
		}

		matrix4 operator -() const {
			matrix4 m;
			for(size_type i(0); i < size; ++i)
				for(size_type j(0); j < size; ++j)
					m.e[i][j] = -this->e[i][j];
			return m;
		}

		matrix4 operator *(real r) const {
			matrix4 m;
			for(size_type i(0); i < size; ++i)
				for(size_type j(0); j < size; ++j)
					m.e[i][j] = this->e[i][j] * r;
			return m;
		}

		matrix4 & operator *=(real r) {
			for(size_type i(0); i < size; ++i)
				for(size_type j(0); j < size; ++j)
					this->e[i][j] *= r;
			return *this;
		}

		matrix4 operator *(matrix4 const & other) const {
			matrix4 m;
			for(size_type i(0); i < size; ++i)
				for(size_type j(0); j < size; ++j) {
					m.e[i][j] = this->e[0][j] * other.e[i][0];
					for(size_type k(1); k < size; ++k)
						m.e[i][j] += this->e[k][j] * other.e[i][k];
			return m;
		}

		real det() {
		}

	private:
		real e[size][size];
};

matrix4 inline operator *(real r, matrix4 const & m) {
	matrix4 result;
	result = m * r;
	return result;
}

}

#endif
