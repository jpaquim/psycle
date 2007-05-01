// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2004-2007 johan boule <bohan@jabber.org>
// copyright 2004-2007 psycledelics http://psycle.pastnotecut.org

#include <ctime>
#include <cerrno>
#include <cstring>
#include <iostream>
#include <iomanip>
#include <cmath>
#include <fenv.h> // ISO C 1999
#include <unistd.h>

namespace universalis
{
	namespace operating_system
	{
		void error(int const & code = errno)
		{
			std::cerr << "error: " << code << ": " << ::strerror(code) << std::endl;
		}

		///\internal
		namespace detail
		{
			bool supported(int const & option)
			{
				long int result(::sysconf(option));
				if(result < -1) error();
				return result > 0;
			}
		}

		namespace clock
		{
			::clockid_t best()
			{
				using namespace detail;
					
				std::cout << "TIMERS ... " << std::endl;
				#if !defined _POSIX_TIMERS
					#warning !defined _POSIX_TIMERS
					std::cout << "\t !defined _POSIX_TIMERS" << std::endl;
				#elif _POSIX_TIMERS == -1
					#warning _POSIX_TIMERS == -1
					std::cout << "_POSIX_TIMERS == -1" << std::endl;
					#define UNIVERSALIS__OPERATING_SYSTEM__CLOCK__DETAIL 0
				#elif _POSIX_TIMERS == 0
					std::cout << "\t defined _POSIX_TIMERS && _POSIX_TIMERS == 0" << std::endl;
				#elif _POSIX_TIMERS > 0
					std::cout << "\t _POSIX_TIMERS > 0" << std::endl;
					#if defined __CYGWIN__ // cygwin defines this because it has clock_gettime, but it doesn't have clock_getres.
						#warning _POSIX_TIMERS > 0, but this is cygwin. \
							Cygwin only partially implements this posix option: it has ::clock_gettime, but not ::clock_getres. \
							Moreover, ::sysconf(_SC_TIMERS) would return 0, which condradicts _POSIX_TIMERS > 0. \
							We will consider that it simply implements nothing.
					#else
						#define UNIVERSALIS__OPERATING_SYSTEM__CLOCK__DETAIL 1
					#endif
				#endif
				#if !defined _SC_TIMERS
					#warning !defined _SC_TIMERS
					std::cout << "\t !defined _SC_TIMERS" << std::endl;
					#if !defined UNIVERSALIS__OPERATING_SYSTEM__CLOCK__DETAIL
						#define UNIVERSALIS__OPERATING_SYSTEM__CLOCK__DETAIL 0
					#endif
				#else
					std::cout << "\t _SC_TIMERS: " << supported(_SC_TIMERS) << std::endl;
				#endif
				
				::clockid_t clock;
				bool got_clock(false);
				
				std::cout << "CPUTIME ... " << std::endl;
				#if !defined _POSIX_CPUTIME
					#warning !defined _POSIX_CPUTIME
					std::cout << "\t !defined _POSIX_CPUTIME" << std::endl;
				#elif _POSIX_CPUTIME == -1
					#warning _POSIX_CPUTIME == -1
					std::cout << "\t _POSIX_CPUTIME == -1" << std::endl;
					#undef  UNIVERSALIS__OPERATING_SYSTEM__CLOCK__DETAIL
					#define UNIVERSALIS__OPERATING_SYSTEM__CLOCK__DETAIL 0
				#elif _POSIX_CPUTIME == 0
					std::cout << "\t defined _POSIX_CPUTIME && _POSIX_CPUTIME == 0" << std::endl;
				#elif _POSIX_CPUTIME > 0
					std::cout << "\t _POSIX_CPUTIME > 0" << std::endl;
					#undef  UNIVERSALIS__OPERATING_SYSTEM__CLOCK__DETAIL
					#define UNIVERSALIS__OPERATING_SYSTEM__CLOCK__DETAIL 1
				#endif
				#if !defined _SC_CPUTIME
					#warning !defined _SC_CPUTIME
					std::cout << "\t !defined _SC_CPUTIME" << std::endl;
					#if !defined UNIVERSALIS__OPERATING_SYSTEM__CLOCK__DETAIL
						#define UNIVERSALIS__OPERATING_SYSTEM__CLOCK__DETAIL 0
					#endif
				#else
				{
					bool const result(supported(_SC_CPUTIME));
					std::cout << "\t _SC_CPUTIME: " << result << std::endl;
					if(result)
					{
						std::cout << "using CLOCK_PROCESS_CPUTIME_ID" << std::endl;
						clock = CLOCK_PROCESS_CPUTIME_ID;
						got_clock = true;
					}
				}
				#endif
			
				std::cout << "MONOTONIC_CLOCK ..." << std::endl;
				#if !defined _POSIX_MONOTONIC_CLOCK
					#warning !defined _POSIX_MONOTONIC_CLOCK
					std::cout << "\t !defined _POSIX_MONOTONIC_CLOCK" << std::endl;
				#elif _POSIX_MONOTONIC_CLOCK == -1
					#warning _POSIX_MONOTONIC_CLOCK == -1
					std::cout << "\t _POSIX_MONOTONIC_CLOCK == -1" << std::endl;
					#undef  UNIVERSALIS__OPERATING_SYSTEM__CLOCK__DETAIL
					#define UNIVERSALIS__OPERATING_SYSTEM__CLOCK__DETAIL 0
				#elif _POSIX_MONOTONIC_CLOCK == 0
					std::cout << "\t defined _POSIX_MONOTONIC_CLOCK && _POSIX_MONOTONIC_CLOCK == 0" << std::endl;
				#elif _POSIX_MONOTONIC_CLOCK > 0
					std::cout << "\t _POSIX_MONOTONIC_CLOCK > 0" << std::endl;
					#undef  UNIVERSALIS__OPERATING_SYSTEM__CLOCK__DETAIL
					#define UNIVERSALIS__OPERATING_SYSTEM__CLOCK__DETAIL 1
				#endif
				#if !defined _SC_MONOTONIC_CLOCK
					#warning !defined _SC_MONOTONIC_CLOCK
					std::cout << "\t !defined _SC_MONOTONIC_CLOCK" << std::endl;
					#if !defined UNIVERSALIS__OPERATING_SYSTEM__CLOCK__DETAIL
						#define UNIVERSALIS__OPERATING_SYSTEM__CLOCK__DETAIL 0
					#endif
				#else
				{
					bool const result(supported(_SC_MONOTONIC_CLOCK));
					std::cout << "\t _SC_MONOTONIC_CLOCK: " << result << std::endl;
					if(result && !got_clock)
					{
						std::cout << "using CLOCK_MONOTONIC" << std::endl;
						clock = CLOCK_MONOTONIC;
						got_clock = true;
					}
				}
				#endif
			
				if(!got_clock)
				{
					std::cout << "using CLOCK_REALTIME as a fallback." << std::endl;
					clock = CLOCK_REALTIME;
				}
				
				return clock;
			}
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

template<>
inline long int test<3>(double d)
{
	return ::lrint(d);
}

template<>
inline long int test<4>(double d)
{
	return lround(d);
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
double measure(::clockid_t clock, double empty_cost = 0)
{
	timespec start;
	if(::clock_gettime(clock, &start))
	{
		universalis::operating_system::error();
		return -1;
	}
	std::cout << loop<double, I>() << std::endl;
	timespec end;
	if(::clock_gettime(clock, &end))
	{
		universalis::operating_system::error();
		return -1;
	}
	timespec result;
	result.tv_sec = end.tv_sec - start.tv_sec;
	if(end.tv_nsec >= start.tv_nsec) result.tv_nsec = end.tv_nsec - start.tv_nsec;
	else
	{
		result.tv_nsec = 1000000000 - start.tv_nsec + end.tv_nsec;
		--result.tv_sec;
	}
	double const elapsed(result.tv_sec + 1e-9 * result.tv_nsec);
	std::cout << "measure: " << I << ": " << std::setprecision(15) << elapsed - empty_cost << "s" << std::endl;
	return elapsed;
}

int main()
{
	::clockid_t clock(universalis::operating_system::clock::best());
	timespec result;
	#if UNIVERSALIS__OPERATING_SYSTEM__CLOCK__DETAIL
		std::cout << "using ::clock_getres ... " << std::endl;
		if(::clock_getres(clock, &result))
		{
			universalis::operating_system::error();
			return 1;
		}
	#else
		std::cout << "using CLOCKS_PER_SEC ... " << std::endl;
		result.tv_sec = 1 / CLOCKS_PER_SEC;
		result.tv_nsec = static_cast<long int>(1e9 / CLOCKS_PER_SEC);
	#endif
	std::cout << "clock resolution: " << std::setprecision(15) << result.tv_sec + 1e-9 * result.tv_nsec << "s" << std::endl;
	double const empty_cost(measure<0>(clock));
	#define measure(i) measure<i>(clock, empty_cost)
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
	measure(4);
	#undef measure
	return 0;
}
