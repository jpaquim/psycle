// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2009-2009 psycledelics http://psycle.pastnotecut.org : johan boule

#ifndef RAYTRACE__MATRIX
#define RAYTRACE__MATRIX
#pragma once

namespace raytrace {

template<typename Value, unsigned int Size1, unsigned int Size2 = Size1>
class matrix {
	public:
		typedef Value value_type;
		typedef unsigned int size_type;
		size_type static const size1 = Size1;
		size_type static const size2 = Size2;

		value_type operator ()(size_type i, size_type j) const { return e[i][j]; }

		value_type & operator ()(size_type i, size_type j) { return e[i][j]; }
		
		void zero() {
			for(size_type i(0); i < size1; ++i)
				for(size_type j(0); j < size2; ++j)
					this->e[i][j] = 0;
		}
		
		void identity() {
			for(size_type i(0); i < size1; ++i)
				for(size_type j(0); j < size2; ++j)
					this->e[i][j] = i == j ? 1 : 0;
		}

		matrix() {}
		
		matrix(matrix const & other) {
			for(size_type i(0); i < size1; ++i)
				for(size_type j(0); j < size2; ++j)
					this->e[i][j] = other.e[i][j];
		}
		
		matrix & operator =(matrix const & other) {
			for(size_type i(0); i < size1; ++i)
				for(size_type j(0); j < size2; ++j)
					this->e[i][j] = other.e[i][j];
		}
		
		matrix operator +(matrix const & other) const {
			matrix m;
			for(size_type i(0); i < size1; ++i)
				for(size_type j(0); j < size2; ++j)
					m.e[i][j] = this->e[i][j] + other.e[i][j];
			return m;
		}

		matrix operator -(matrix const & other) const {
			matrix m;
			for(size_type i(0); i < size1; ++i)
				for(size_type j(0); j < size2; ++j)
					m.e[i][j] = this->e[i][j] - other.e[i][j];
			return m;
		}

		matrix & operator +=(matrix const & other) {
			for(size_type i(0); i < size1; ++i)
				for(size_type j(0); j < size2; ++j)
					this->e[i][j] += other.e[i][j];
			return *this;
		}

		matrix & operator -=(matrix const & other) {
			for(size_type i(0); i < size1; ++i)
				for(size_type j(0); j < size2; ++j)
					this->e[i][j] -= other.e[i][j];
			return *this;
		}

		matrix operator -() const {
			matrix m;
			for(size_type i(0); i < size1; ++i)
				for(size_type j(0); j < size2; ++j)
					m.e[i][j] = -this->e[i][j];
			return m;
		}

		matrix operator *(value_type r) const {
			matrix m;
			for(size_type i(0); i < size1; ++i)
				for(size_type j(0); j < size2; ++j)
					m.e[i][j] = this->e[i][j] * r;
			return m;
		}

		matrix & operator *=(value_type r) {
			for(size_type i(0); i < size1; ++i)
				for(size_type j(0); j < size2; ++j)
					this->e[i][j] *= r;
			return *this;
		}

		matrix<value_type, size2, size2> operator *(matrix<value_type, size2, size1> const & other) const {
			matrix<value_type, size2, size2> m;
			for(size_type i(0); i < size2; ++i)
				for(size_type j(0); j < size2; ++j) {
					m.e[i][j] = this->e[0][j] * other.e[i][0];
					for(size_type k(1); k < size1; ++k)
						m.e[i][j] += this->e[k][j] * other.e[i][k];
				}
			return m;
		}

	private:
		value_type e[size1][size2];
};

template<typename Value, unsigned int Size1, unsigned int Size2>
matrix<Value, Size1, Size2> inline operator *(Value r, matrix<Value, Size1, Size2> const & m) {
	matrix<Value, Size1, Size2> result;
	result = m * r;
	return result;
}

}

#endif
