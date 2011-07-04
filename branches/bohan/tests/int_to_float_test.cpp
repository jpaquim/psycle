/*
	http://lol.zoy.org/blog/2011/3/20/understanding-fast-float-integer-conversions

	I've tried to measure the performance gain with the following program,
	compiled with "g++ -std=gnu++0x -march=native -O3", gcc 4.4.3 on an intel core2, linux x86_64.
	Unless i've overlooked something, there seem to be no way to beat the compiler at the uint8->float conversion;
	this is the output i get:

	specialized: 3.22567e+09
	standard   : 3.22567e+09
	0.047753s < 0.031519s
	int_to_float: /tmp/int_to_float.cpp:39: int main(): Assertion `t2 - t1 < t3 - t2' failed.

	-- johan-boule@users.sourceforge.net
*/

#include <chrono>
#include <limits>
#include <stdexcept>
#include <iostream>
#undef NDEBUG
#include <cassert>

float inline uint8_t_to_float(uint8_t x) {
	union { float f; uint32_t i; } u; u.f = 32768.0f; u.i |= x;
	return (u.f - 32768.0f) * 256.0f ;
}

int main() {
	typedef uint8_t Integer;
	typedef float Real;
	typedef std::chrono::high_resolution_clock clock;
	std::size_t const iterations = 100000;
	
	typedef std::numeric_limits<Integer> limits;

	clock::time_point const t1 = clock::now();
	Real r1(0);
	for(std::size_t count = 0; count < iterations; ++count)
		for(Integer i = limits::min(); i < limits::max(); ++i)
			r1 += uint8_t_to_float(i);
	clock::time_point const t2 = clock::now();
	Real r2(0);
	for(std::size_t count = 0; count < iterations; ++count)
		for(Integer i = limits::min(); i < limits::max(); ++i)
			r2 += i;
	clock::time_point const t3 = clock::now();

	std::cout << "specialized: " << r1 << '\n';
	std::cout << "standard   : " << r2 << '\n';
	std::cout <<
		std::chrono::nanoseconds(t2 - t1).count() * 1e-9 << "s < " <<
		std::chrono::nanoseconds(t3 - t2).count() * 1e-9 << "s\n";
	assert(r1 == r2);
	assert(t2 - t1 < t3 - t2);
}

