#ifndef PSYCLE__PLUGINS__GVERB__GVERB_DSP__INCLUDED
#define PSYCLE__PLUGINS__GVERB__GVERB_DSP__INCLUDED
#pragma once

#include <psycle/helpers/math/erase_all_nans_infinities_and_denormals.hpp>

/**********************************************************/
// diffuser

struct ty_diffuser {
	int size;
	float coeff;
	int idx;
	float *buf;
};
ty_diffuser * diffuser_make(int, float);
void diffuser_free(ty_diffuser *);
void diffuser_flush(ty_diffuser *);
inline float diffuser_do(ty_diffuser *p, float x) {
	float y,w;
	float* buf = &p->buf[p->idx];
	w = x - (*buf)*p->coeff;
	psycle::helpers::math::erase_all_nans_infinities_and_denormals(w);
	y = (*buf) + w*p->coeff;
	*buf = w;
	p->idx = (p->idx + 1) % p->size;
	///\todo: denormal check on y too?
	return y;
}

/**********************************************************/
// damper

struct ty_damper {
	float damping;
	float delay;
};
ty_damper * damper_make(float);
void damper_free(ty_damper *);
void damper_flush(ty_damper *);
inline void damper_set(ty_damper *p, float damping) {
	p->damping = damping;
}
inline float damper_do(ty_damper *p, float x) {
	float y = x*(1.0-p->damping) + p->delay*p->damping;
	psycle::helpers::math::erase_all_nans_infinities_and_denormals(y);
	p->delay = y;
	return(y);
}

/**********************************************************/
// fixeddelay

struct ty_fixeddelay {
	int size;
	int idx;
	float *buf;
};
ty_fixeddelay * fixeddelay_make(int);
void fixeddelay_free(ty_fixeddelay *);
void fixeddelay_flush(ty_fixeddelay *);
inline float fixeddelay_read(ty_fixeddelay *p, int n) {
	int i = (p->idx - n + p->size) % p->size;
	return(p->buf[i]);
}
inline void fixeddelay_write(ty_fixeddelay *p, float x) {
	p->buf[p->idx] = x;
	p->idx = (p->idx + 1) % p->size;
}

/**********************************************************/
//int isprime(int);
//int nearest_prime(int, float);
	
#endif
