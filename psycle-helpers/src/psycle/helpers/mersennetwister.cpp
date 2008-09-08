// -*- mode:c++; indent-tabs-mode:t -*-
///\implementation psycle::helpers::dsp::MersenneTwister

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

#include <packageneric/pre-compiled.private.hpp>
#include <packageneric/module.private.hpp>
#include "mersennetwister.hpp"
#include <cmath>
namespace psycle { namespace helpers { namespace dsp {

/// the array for the state vector
std::uint32_t MersenneTwister::mt[N];

/// mti == N + 1 means mt[N] is not initialized
std::int32_t MersenneTwister::mti = N + 1;

/// initializes mt[N] with a seed
void MersenneTwister::init_genrand(std::uint32_t s) {
	mt[0] = s & 0xffffffffU;
	for(mti = 1; mti < N; ++mti) {
		mt[mti] = 1812433253 * (mt[mti - 1] ^ (mt[mti - 1] >> 30)) + mti;
		/// See Knuth TAOCP Vol2. 3rd Ed. P.106 for multiplier.
		/// In the previous versions, MSBs of the seed affect
		/// only MSBs of the array mt[].
		// 2002/01/09 modified by Makoto Matsumoto
		mt[mti] &= 0xffffffffU;
		/// for >32 bit machines
	}
}

/// initialize by an array with array-length.
/// init_key is the array for initializing keys,
/// key_length is its length
void MersenneTwister::init_by_array(std::uint32_t init_key[], unsigned int key_length) {
	std::int32_t i, j, k;
	init_genrand(19650218);
	i = 1; j = 0;
	k = N > key_length ? N : key_length;
	for(; k; --k) {
		mt[i] = (mt[i] ^ ((mt[i-1] ^ (mt[i-1] >> 30)) * 1664525)) + init_key[j] + j; // non linear
		mt[i] &= 0xffffffffU; // for WORDSIZE > 32 machines
		++i; ++j;
		if(i >= N) { mt[0] = mt[N - 1]; i = 1; }
		if(j >= key_length) j = 0;
	}
	for(k = N - 1; k; --k) {
		mt[i] = (mt[i] ^ ((mt[i-1] ^ (mt[i-1] >> 30)) * 1566083941)) - i; /* non linear */
		mt[i] &= 0xffffffffU; // for WORDSIZE > 32 machines
		++i;
		if(i >= N) { mt[0] = mt[N - 1]; i = 1; }
	}

	mt[0] = 0x80000000U; // MSB is 1; assuring non-zero initial array
}

/// generates a random number on [0,0xffffffff]-interval
std::uint32_t MersenneTwister::genrand_int32() {
	std::uint32_t y;
	static std::uint32_t mag01[2] = { 0, MATRIX_A };
	// mag01[x] = x * MATRIX_A  for x = 0, 1

	if(mti >= N) { // generate N words at one time
		std::int32_t kk;

		if(mti == N + 1) // if init_genrand() has not been called,
			init_genrand(5489); // a default initial seed is used

		for(kk = 0; kk < N - M; ++kk) {
			y = (mt[kk] & UPPER_MASK) | (mt[kk + 1] & LOWER_MASK);
			mt[kk] = mt[kk + M] ^ (y >> 1) ^ mag01[y & 1];
		}
		for(; kk < N - 1; ++kk) {
			y = (mt[kk] & UPPER_MASK) | (mt[kk + 1] & LOWER_MASK);
			mt[kk] = mt[kk + (M - N)] ^ (y >> 1) ^ mag01[y & 1];
		}
		y = (mt[N - 1] & UPPER_MASK) | (mt[0] & LOWER_MASK);
		mt[N - 1] = mt[M - 1] ^ (y >> 1) ^ mag01[y & 1];

		mti = 0;
	}
	
	y = mt[mti++];

	// Tempering
	y ^= (y >> 11);
	y ^= (y << 7) & 0x9d2c5680U;
	y ^= (y << 15) & 0xefc60000U;
	y ^= (y >> 18);

	return y;
}

/// generates a random number on [0, 0x7fffffff] interval
std::int32_t MersenneTwister::genrand_int31() {
	return static_cast<std::int32_t>(genrand_int32() >> 1);
}

/// generates a random number on [0, 1] real interval
double MersenneTwister::genrand_real1() {
	return genrand_int32() * (1.0 / 4294967295.0);
	// divided by 2 ^ 32 - 1
}

/// generates a random number on [0, 1) real interval
double MersenneTwister::genrand_real2() {
	return genrand_int32() * (1.0 / 4294967296.0);
	// divided by 2 ^ 32
}

/// generates a random number on (0, 1) real interval
double MersenneTwister::genrand_real3() {
	return (static_cast<double>(genrand_int32()) + 0.5) * (1.0 / 4294967296.0);
	// divided by 2 ^ 32
}

/// generates a random number on [0, 1) with 53-bit resolution
double MersenneTwister::genrand_res53() { 
	std::uint32_t a = genrand_int32() >> 5, b = genrand_int32() >> 6; 
	return (a * 67108864.0 + b) * (1.0 / 9007199254740992.0);
} 

// These real versions are due to Isaku Wada, 2002/01/09 added

/// gaussian-distribution variation, added by dw
void MersenneTwister::genrand_gaussian(double & out1, double & out2) {
	float x1;
	float x2;
	float w;
	do {
		x1 = this->genrand_real1() * 2 - 1;
		x2 = this->genrand_real1() * 2 - 1;
		w = x1 * x1 + x2 * x2;
	} while(w >= 1.0);
	w = std::sqrt((-2.0f * log(w)) / w);
	out1 = w * x1;
	out2 = w * x2;
}

}}}
