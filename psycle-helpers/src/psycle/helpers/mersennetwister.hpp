///\interface psycle::helpers::dsp::MersenneTwister

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

#pragma once
#include <cstdint>
namespace psycle { namespace helpers { namespace dsp {

/// Period parameters
std::int32_t const N = 624;
/// Period parameters
std::int32_t const M = 397;
/// constant vector a
std::uint32_t const MATRIX_A = 0x9908b0dfU;
/// most significant w-r bits
std::uint32_t const UPPER_MASK = 0x80000000U;
/// least significant r bits
std::uint32_t const LOWER_MASK = 0x7fffffffU;

/// mt19937 pseudo-random number generator
class MersenneTwister {
public:
	MersenneTwister() {}
	virtual ~MersenneTwister() {}

	/// initializes mt[N] with a seed
	void init_genrand(std::uint32_t s);

	/// initialize by an array with array-length
	void init_by_array(std::uint32_t init_key[], unsigned int key_length);

	/// generates a random number on [0,0xffffffff]-interval
	std::uint32_t genrand_int32();

	/// generates a random number on [0,0x7fffffff]-interval
	std::int32_t genrand_int31();

	/// generates a random number on [0,1]-real-interval
	double genrand_real1();

	/// generates a random number on [0,1)-real-interval
	double genrand_real2();

	/// generates a random number on (0,1)-real-interval
	double genrand_real3();

	/// generates a random number on [0,1) with 53-bit resolution
	double genrand_res53(); 

	/// generates two random numbers with gaussian probability distribution and standard deviation of 1
	void genrand_gaussian(double & out1, double & out2);

private:
	/// the array for the state vector
	static std::uint32_t mt[N];
	static std::int32_t mti;
};

}}}
