/*
	http://lol.zoy.org/blog/2011/3/20/understanding-fast-float-integer-conversions

	I've tried to measure the performance gain with the following program,
	compiled with gcc 4.4.3 on an intel core2 quad (4 cores), linux x86_64.
	This is the output i get with various compiler flags:

	# g++ -std=gnu++0x -march=native -O3 -funroll-loops -fopenmp int_to_float_test.cpp && ./a.out
	no conversion: sum: 4.29497e+09, time: 0.777653s
	specialized  : sum: 4.29497e+09, time: 0.792678s
	standard     : sum: 4.29497e+09, time: 0.793893s
	
	# g++ -std=gnu++0x -march=native -O3 -fopenmp int_to_float_test.cpp && ./a.out
	no conversion: sum: 4.29497e+09, time: 0.797142s
	specialized  : sum: 4.29497e+09, time: 1.11614s
	standard     : sum: 4.29497e+09, time: 0.788671s
	a.out: int_to_float_test.cpp:65: int main(): Assertion `t2 - t1 < t3 - t2' failed.
	
	# g++ -std=gnu++0x -march=native -O3 -funroll-loops int_to_float_test.cpp && ./a.out
	no conversion: sum: 4.29497e+09, time: 3.06334s
	specialized  : sum: 4.29497e+09, time: 3.16423s
	standard     : sum: 4.29497e+09, time: 3.14101s
	a.out: int_to_float_test.cpp:65: int main(): Assertion `t2 - t1 < t3 - t2' failed.

	# g++ -std=gnu++0x -march=native -O3 int_to_float_test.cpp && ./a.out
	no conversion: sum: 4.29497e+09, time: 3.17681s
	specialized  : sum: 4.29497e+09, time: 4.46758s
	standard     : sum: 4.29497e+09, time: 3.14915s
	a.out: int_to_float_test.cpp:65: int main(): Assertion `t2 - t1 < t3 - t2' failed.

	-- johan-boule@users.sourceforge.net
*/

#include <chrono>
#include <limits>
#include <iostream>
#undef NDEBUG
#include <cassert>

float inline uint8_t_to_float(std::uint8_t x) {
	union { float f; std::uint32_t i; } u; u.f = 32768.0f; u.i |= x;
	return (u.f - 32768.0f) * 256.0f ;
}

int main() {
	typedef std::uint8_t int_type;
	typedef float real;
	typedef std::chrono::high_resolution_clock clock;
	std::size_t const iterations = 10000000;
	
	typedef std::numeric_limits<int_type> limits;

	clock::time_point const t0 = clock::now();
	// without conversion
	real r0(0);
	#pragma omp parallel for
	for(std::size_t count = 0; count < iterations; ++count)
		for(real r = limits::min(); r < limits::max(); ++r)
			r0 += r;
	clock::time_point const t1 = clock::now();
	// specialized conversion
	real r1(0);
	#pragma omp parallel for
	for(std::size_t count = 0; count < iterations; ++count)
		for(int_type i = limits::min(); i < limits::max(); ++i)
			r1 += uint8_t_to_float(i);
	clock::time_point const t2 = clock::now();
	// standard conversion
	real r2(0);
	#pragma omp parallel for
	for(std::size_t count = 0; count < iterations; ++count)
		for(int_type i = limits::min(); i < limits::max(); ++i)
			r2 += i;
	clock::time_point const t3 = clock::now();

	std::cout <<
		"no conversion: sum: " << r0 << ", time: " << std::chrono::nanoseconds(t1 - t0).count() * 1e-9 << "s\n"
		"specialized  : sum: " << r1 << ", time: " << std::chrono::nanoseconds(t2 - t1).count() * 1e-9 << "s\n"
		"standard     : sum: " << r2 << ", time: " << std::chrono::nanoseconds(t3 - t2).count() * 1e-9 << "s\n";
	assert(r0 == r1 && r1 == r2);
	assert(t1 - t0 < t2 - t1);
	assert(t2 - t1 < t3 - t2);
}

