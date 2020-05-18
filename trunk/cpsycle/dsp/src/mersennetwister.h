#ifndef psy_dsp_MERSENNETWISTER_H
#define psy_dsp_MERSENNETWISTER_H

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

#include "../../detail/psydef.h"

/// Period parameters
#define psy_dsp_MERSENNETWISTER_N 624
/// Period parameters
#define psy_dsp_MERSENNETWISTER_M 397
/// constant vector a
#define psy_dsp_MERSENNETWISTER_MATRIX_A 0x9908b0dfU
/// most significant w-r bits
#define psy_dsp_MERSENNETWISTER_UPPER_MASK 0x80000000U
/// least significant r bits
#define psy_dsp_MERSENNETWISTER_LOWER_MASK 0x7fffffffU

/// mt19937 pseudo-random number generator
typedef struct psy_dsp_MersenneTwister {
	/// the array for the state vector
	uint32_t mt[psy_dsp_MERSENNETWISTER_N];
	uintptr_t mti;	
} psy_dsp_MersenneTwister;

INLINE void psy_dsp_MersenneTwister_init(psy_dsp_MersenneTwister* self)
{
	/// mti == N + 1 means mt[N] is not initialized
	self->mti = psy_dsp_MERSENNETWISTER_N + 1;
}

/// initializes mt[N] with a seed
void psy_dsp_mersennetwister_init_genrand(psy_dsp_MersenneTwister*,
	uint32_t s);
/// initialize by an array with array-length
void psy_dsp_mersennetwisterinit_by_array(psy_dsp_MersenneTwister*,
	uint32_t* init_key, uintptr_t key_length);
/// generates a random number on integer interval [0, 0xffffffff]
uint32_t psy_dsp_mersennetwister_genrand_int32(psy_dsp_MersenneTwister*);
/// generates a random number on integer interval [0, 0x7fffffff]
int32_t psy_dsp_mersennetwister_genrand_int31(psy_dsp_MersenneTwister*);
/// generates a random number on real interval [0, 1]
double psy_dsp_mersennetwister_genrand_real1(psy_dsp_MersenneTwister*);
/// generates a random number on real interval [0, 1[
double psy_dsp_mersennetwister_genrand_real2(psy_dsp_MersenneTwister*);
/// generates a random number on real interval ]0, 1[
double psy_dsp_mersennetwister_genrand_real3(psy_dsp_MersenneTwister*);
/// generates a random number on real interval [0, 1[ with 53-bit resolution
double psy_dsp_mersennetwister_genrand_res53(psy_dsp_MersenneTwister*);
/// generates two random numbers with gaussian probability distribution and
/// standard deviation of 1
void psy_dsp_mersennetwister_genrand_gaussian(psy_dsp_MersenneTwister*,
	double* out1, double* out2);

#endif /* psy_dsp_MERSENNETWISTER_H */
