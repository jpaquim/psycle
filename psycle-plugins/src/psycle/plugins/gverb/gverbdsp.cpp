/*

		Copyright (C) 1999 Juhana Sadeharju
						kouhia at nic.funet.fi

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

	*/

#include "gverbdsp.h"
#include <cstdio>
#include <cmath>
#include <cstdlib>
#include <cstring>

ty_diffuser *diffuser_make(int size, float coeff) {
	ty_diffuser *p = (ty_diffuser *) std::malloc(sizeof(ty_diffuser));
	p->size = size;
	p->coeff = coeff;
	p->idx = 0;
	p->buf = (float *) std::malloc(size * sizeof(float));
	for(int i = 0; i < size; ++i) p->buf[i] = 0.0;
	return p;
}

void diffuser_free(ty_diffuser *p) {
	std::free(p->buf);
	std::free(p);
}

void diffuser_flush(ty_diffuser *p) {
	std::memset(p->buf, 0, p->size * sizeof(float));
}

ty_damper *damper_make(float damping) {
	ty_damper *p = (ty_damper *) std::malloc(sizeof(ty_damper));
	p->damping = damping;
	p->delay = 0.0f;
	return p;
}

void damper_free(ty_damper *p) {
	std::free(p);
}

void damper_flush(ty_damper *p) {
	p->delay = 0.0f;
}

ty_fixeddelay *fixeddelay_make(int size) {
	ty_fixeddelay *p = (ty_fixeddelay *) std::malloc(sizeof(ty_fixeddelay));
	p->size = size;
	p->idx = 0;
	p->buf = (float *) std::malloc(size * sizeof(float));
	for(int i = 0; i < size; ++i) p->buf[i] = 0.0;
	return p;
}

void fixeddelay_free(ty_fixeddelay *p) {
	std::free(p->buf);
	std::free(p);
}

void fixeddelay_flush(ty_fixeddelay *p) {
	std::memset(p->buf, 0, p->size * sizeof(float));
}

#if 0
	int isprime(int n) {
		if(n == 2) return true;
		if((n & 1) == 0) return false;
		const unsigned int lim = (int) std::sqrt((float)n);
		for(unsigned int i = 3; i <= lim; i += 2)
			if((n % i) == 0) return false;
		return true;
	}

	/// relative error: new prime will be in range [n - n * rerror, n + n * rerror];
	int nearest_prime(int n, float rerror) {
		if(isprime(n)) return n;
		// assume n is large enough and n*rerror enough smaller than n
		int bound = n * rerror;
		for(int k = 1; k <= bound; ++k) {
			if(isprime(n + k)) return n + k;
			if(isprime(n - k)) return n - k;
		}
		return -1;
	}
#endif
