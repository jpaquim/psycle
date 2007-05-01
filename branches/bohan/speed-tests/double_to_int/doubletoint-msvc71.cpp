// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2004-2007 johan boule <bohan@jabber.org>
// copyright 2004-2007 psycledelics http://psycle.pastnotecut.org

#include <iostream>
#include <iomanip>
#include <windows.h>

namespace universalis
{
	namespace operating_system
	{
		void error(int const & code = errno)
		{
			std::cerr << "error: " << code << ": " << ::strerror(code) << std::endl;
		}
	}
}

template<unsigned int I>
inline long int test(double d)
{
	return 123;
}

template<>
inline long int test<1>(double d)
{
	union u
	{
		double d;
		long int i;
	} u;
	u.d = d + 6755399441055744.0;
	return u.i;
}

template<>
inline long int test<2>(double d)
{
	return static_cast<long int>(d);
}

#if !defined _MSC_VER || _MSC_VER >= 1400 //  lrint is not available in MSVC 7.1.
template<>
inline long int test<3>(double d)
{
	return ::lrint(d);
}
#endif

template<typename T, unsigned int I>
T loop()
{
	T result = T();
	for(unsigned int i(500000); i > 0; --i)
	{
		double d(1000);
		while(d >= -1000)
		{
			result += test<I>(d);
			d -= 1;
		}
	}
	return result;
}

template<unsigned int I>
double measure(double empty_cost = 0)
{
	const unsigned long start=timeGetTime();
	std::cout << loop<double, I>() << std::endl;
	const unsigned long end=timeGetTime();

	double const elapsed(end-start);
	std::cout << "measure: " << I << ": " << std::setprecision(15) << elapsed - empty_cost << "s" << std::endl;
	return elapsed;
}

int main()
{
#if 0
	std::cout << test<1>(1.1) << std::endl;
	std::cout << test<1>(1.9) << std::endl;
	std::cout << test<2>(1.1) << std::endl;
	std::cout << test<2>(1.9) << std::endl;
	std::cout << test<1>(-1.1) << std::endl;
	std::cout << test<1>(-1.9) << std::endl;
	std::cout << test<2>(-1.1) << std::endl;
	std::cout << test<2>(-1.9) << std::endl;
#endif

	double const empty_cost(measure<0>());
#define measure(i) measure<i>(empty_cost)
	measure(1);
	measure(2);
#if defined FE_TONEAREST
	int feround(::fegetround());
	::fesetround(FE_TONEAREST);
#endif
#if !defined _MSC_VER || _MSC_VER >= 1400
	measure(3);
#endif
#if defined FE_TONEAREST
	::fesetround(feround);
#endif
#undef measure
	return 0;
}
