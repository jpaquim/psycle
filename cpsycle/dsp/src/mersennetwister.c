/* 
	A C-program for MT19937, with initialization improved 2002/1/26.
	Coded by Takuji Nishimura and Makoto Matsumoto.

	Before using, initialize the state by using init_genrand(seed)  
	or init_by_array(init_key, key_length).

	Copyright (C) 1997 - 2002, Makoto Matsumoto and Takuji Nishimura,
	All rights reserved.                          

	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions
	are met:

		1. Redistributions of source code must retain the above copyright
		notice, this list of conditions and the following disclaimer.

		2. Redistributions in binary form must reproduce the above copyright
		notice, this list of conditions and the following disclaimer in the
		documentation and/or other materials provided with the distribution.

		3. The names of its contributors may not be used to endorse or promote 
		products derived from this software without specific prior written 
		permission.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
	"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
	LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
	A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
	CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
	EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
	PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
	PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
	LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
	NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
	SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


	Any feedback is very welcome.
	http://www.math.sci.hiroshima-u.ac.jp/~m-mat/MT/emt.html
	email: m-mat @ math.sci.hiroshima-u.ac.jp (remove space)
*/

// (c++-ified for psycle by dw aley)
// (64-bit compatibility by johan boule)
// (made thread-safe by eliminating static data by johan boule)

#include "mersennetwister.h"

#include <math.h>

/// initializes mt[N] with a seed
void psy_dsp_mersennetwister_init_genrand(psy_dsp_MersenneTwister* self,
	uint32_t s)
{
	self->mt[0] = s;
	for(self->mti = 1; self->mti < psy_dsp_MERSENNETWISTER_N; ++self->mti) {
		self->mt[self->mti] = 1812433253 * (self->mt[self->mti - 1] ^
			(self->mt[self->mti - 1] >> 30)) + self->mti;
		/// See Knuth TAOCP Vol2. 3rd Ed. P.106 for multiplier.
		/// In the previous versions, MSBs of the seed affect
		/// only MSBs of the array mt[].
	}
}

/// initialize by an array with array-length.
/// init_key is the array for initializing keys,
/// key_length is its length
void psy_dsp_mersennetwisterinit_by_array(psy_dsp_MersenneTwister* self,
	uint32_t* init_key, uintptr_t key_length)
{
	uintptr_t i;
	uintptr_t j;
	uintptr_t k;

	psy_dsp_mersennetwister_init_genrand(self, 19650218);
	i = 1;
	j = 0;
	k = (psy_dsp_MERSENNETWISTER_N > key_length)
		? psy_dsp_MERSENNETWISTER_N : key_length;
	for(; k; --k) {
		self->mt[i] = (self->mt[i] ^ ((self->mt[i-1] ^
			(self->mt[i-1] >> 30)) * 1664525)) + init_key[j] + j; // non linear
		self->mt[i] &= 0xffffffffU; // for WORDSIZE > 32 machines
		++i; ++j;
		if(i >= psy_dsp_MERSENNETWISTER_N) {
			self->mt[0] = self->mt[psy_dsp_MERSENNETWISTER_N - 1];
			i = 1;
		}
		if (j >= key_length) {
			j = 0;
		}
	}
	for(k = psy_dsp_MERSENNETWISTER_N - 1; k; --k) {
		self->mt[i] = (self->mt[i] ^ ((self->mt[i-1] ^
			(self->mt[i-1] >> 30)) * 1566083941)) - i; /* non linear */
		self->mt[i] &= 0xffffffffU; // for WORDSIZE > 32 machines
		++i;
		if(i >= psy_dsp_MERSENNETWISTER_N) {
			self->mt[0] = self->mt[psy_dsp_MERSENNETWISTER_N - 1];
			i = 1;
		}
	}
	self->mt[0] = 0x80000000U; // MSB is 1; assuring non-zero initial array
}

/// generates a random number on [0,0xffffffff]-interval
uint32_t psy_dsp_mersennetwister_genrand_int32(psy_dsp_MersenneTwister* self)
{
	uint32_t y;

	if (self->mti >= psy_dsp_MERSENNETWISTER_N) { // generate N words at one time
		uint32_t const static mag01[2] = { 0, psy_dsp_MERSENNETWISTER_MATRIX_A };
		uintptr_t kk;
		uint32_t y;

		if (self->mti == psy_dsp_MERSENNETWISTER_N + 1) { // if init_genrand() has not been called,		
			psy_dsp_mersennetwister_init_genrand(self, 5489); // a default initial seed is used
		}	
		// mag01[x] = x * MATRIX_A  for x = 0, 1							
		for(kk = 0; kk < psy_dsp_MERSENNETWISTER_N - psy_dsp_MERSENNETWISTER_M; ++kk) {
			uint32_t y = (self->mt[kk] & psy_dsp_MERSENNETWISTER_UPPER_MASK)
				| (self->mt[kk + 1] & psy_dsp_MERSENNETWISTER_LOWER_MASK);
			self->mt[kk] = self->mt[kk + psy_dsp_MERSENNETWISTER_M] ^ (y >> 1) ^ mag01[y & 1];
		}
		for(; kk < psy_dsp_MERSENNETWISTER_N - 1; ++kk) {
			y = (self->mt[kk] & psy_dsp_MERSENNETWISTER_UPPER_MASK) |
				(self->mt[kk + 1] & psy_dsp_MERSENNETWISTER_LOWER_MASK);
			self->mt[kk] = self->mt[kk + (psy_dsp_MERSENNETWISTER_M - psy_dsp_MERSENNETWISTER_N)]
				^ (y >> 1) ^ mag01[y & 1];
		}		
		y = (self->mt[psy_dsp_MERSENNETWISTER_N - 1] & psy_dsp_MERSENNETWISTER_UPPER_MASK) |
			(self->mt[0] & psy_dsp_MERSENNETWISTER_LOWER_MASK);
		self->mt[psy_dsp_MERSENNETWISTER_N - 1] = self->mt[psy_dsp_MERSENNETWISTER_M - 1] ^
			(y >> 1) ^ mag01[y & 1];
		
		self->mti = 0;
	}	
	y = self->mt[self->mti++];
	// Tempering
	y ^= (y >> 11);
	y ^= (y << 7) & 0x9d2c5680U;
	y ^= (y << 15) & 0xefc60000U;
	y ^= (y >> 18);

	return y;
}

/// generates a random number on [0, 0x7fffffff] interval
int32_t psy_dsp_mersennetwister_genrand_int31(psy_dsp_MersenneTwister* self)
{
	return (int32_t)(psy_dsp_mersennetwister_genrand_int32(self) >> 1);
}

// These real versions are due to Isaku Wada, 2002/01/09 added

/// generates a random number on [0, 1] real interval
double psy_dsp_mersennetwister_genrand_real1(psy_dsp_MersenneTwister* self)
{
	return psy_dsp_mersennetwister_genrand_int32(self) * (1.0 / 4294967295.0);
	// divided by 2 ^ 32 - 1
}

/// generates a random number on [0, 1) real interval
double psy_dsp_mersennetwister_genrand_real2(psy_dsp_MersenneTwister* self)
{
	return psy_dsp_mersennetwister_genrand_int32(self) * (1.0 / 4294967296.0);
	// divided by 2 ^ 32
}

/// generates a random number on (0, 1) real interval
double psy_dsp_mersennetwister_genrand_real3(psy_dsp_MersenneTwister* self)
{
	return ((double)(psy_dsp_mersennetwister_genrand_int32(self)) + 0.5) *
		(1.0 / 4294967296.0);
	// divided by 2 ^ 32
}

/// generates a random number on [0, 1) with 53-bit resolution
double psy_dsp_mersennetwister_genrand_res53(psy_dsp_MersenneTwister* self)
{
	uint32_t a = psy_dsp_mersennetwister_genrand_int32(self) >> 5;
	uint32_t b = psy_dsp_mersennetwister_genrand_int32(self) >> 6;
	return (a * 67108864.0 + b) * (1.0 / 9007199254740992.0);
} 

/// gaussian-distribution variation, added by dw
void psy_dsp_mersennetwister_genrand_gaussian(psy_dsp_MersenneTwister* self,
	double* out1, double* out2)
{
	double x1;
	double x2;
	double w;
	do {
		x1 = psy_dsp_mersennetwister_genrand_real1(self) * 2 - 1;
		x2 = psy_dsp_mersennetwister_genrand_real1(self) * 2 - 1;
		w = x1 * x1 + x2 * x2;
	} while(w >= 1.0f);
	w = sqrt((-2.0f * log(w)) / w);
	*out1 = w * x1;
	*out2 = w * x2;
}
