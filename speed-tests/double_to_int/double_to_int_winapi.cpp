// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2004-2007 johan boule <bohan@jabber.org>
// copyright 2004-2007 psycledelics http://psycle.pastnotecut.org

#include <iostream>
#include <iomanip>
#include <cmath>
#include <windows.h>

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

#if defined _MSC_VER && defined _M_IX86
inline long int lrint(double d)
{
	long int i;
	__asm // note: this is not as fast as one might expect.
	{
		fld d;
		fistp i;
	}
	return i;
}
#endif

template<>
inline long int test<3>(double d)
{
	return lrint(d);
}

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
	std::cout << "measure: " << I << ": " << std::setprecision(15) << elapsed - empty_cost << "ms" << std::endl;
	return elapsed;
}

int main()
{
	double const empty_cost(measure<0>());
	#define measure(i) measure<i>(empty_cost)
		measure(1);
		measure(2);
		#if defined FE_TONEAREST
			int feround(::fegetround());
			::fesetround(FE_TONEAREST);
		#endif
		measure(3);
		#if defined FE_TONEAREST
			::fesetround(feround);
		#endif
	#undef measure
	return 0;
}
