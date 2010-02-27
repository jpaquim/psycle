// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2004-2008 members of the psycle project http://psycle.pastnotecut.org ; johan boule <bohan@jabber.org>

///\file \brief universalis::os::clocks

#ifndef UNIVERSALIS__OS__CLOCKS__INCLUDED
#define UNIVERSALIS__OS__CLOCKS__INCLUDED
#pragma once

#include <universalis/stdlib/cstdint.hpp>
#include <universalis/stdlib/detail/duration.hpp> // cannot #include <universalis/stdlib/date_time.hpp> due to mutual dependency
#include "exception.hpp"

#if defined DIVERSALIS__OS__POSIX
	#include <ctime>
	#include <cerrno>
	#include <cstring>
	#include <unistd.h>
#elif defined DIVERSALIS__OS__MICROSOFT
	#include "include_windows_without_crap.hpp"
	#if defined DIVERSALIS__COMPILER__FEATURE__AUTO_LINK
		#pragma comment(lib, "kernel32") // win64?
	#endif

	#if defined DIVERSALIS__COMPILER__MICROSOFT
		#pragma warning(push)
		#pragma warning(disable:4201) // nonstandard extension used : nameless struct/union
	#endif

		#include <mmsystem.h>
		#if defined DIVERSALIS__COMPILER__FEATURE__AUTO_LINK
			#pragma comment(lib, "winmm")
		#endif

	#if defined DIVERSALIS__COMPILER__MICROSOFT
		#pragma warning(pop)
	#endif
#else
	#error unsupported operating system
#endif

#include <stdexcept>
#if defined BOOST_AUTO_TEST_CASE
	#include <universalis/stdlib/thread.hpp>
	#include <sstream>
#endif

namespace universalis { namespace os { namespace clocks {
	using namespace stdlib;

// recommended: http://icl.cs.utk.edu/papi/custom/index.html?lid=62&slid=96

/// a real time clock that counts the time elapsed since some unspecified origin.
///
/// The implementation reads, if available, the tick count register of some unspecified CPU.
/// On most CPU architectures, the register is updated at a rate based on the frequency of the cycles, but often the count value and the tick events are unrelated,
/// i.e. the value might not be incremented one by one. So the period corresponding to 1 count unit may be even smaller than the period of a CPU cycle, but should probably stay in the same order of magnitude.
/// If the counter is increased by 4,000,000,000 over a second, and is 64-bit long, it is possible to count an uptime period in the order of a century without wrapping.
/// The implementation for x86, doesn't work well at all on some of the CPUs whose frequency varies over time. This will eventually be fixed http://www.x86-secret.com/?option=newsd&nid=845.
/// The implementation for mswindows is unpsecified on SMP systems.
class UNIVERSALIS__DECL monotonic {
	public:
		nanoseconds static current();
};

/// a real time clock that counts the UTC time elasped since the unix epoch (1970-01-01T00:00:00UTC).
///
/// This is the UTC time, and hence not monotonic since UTC has leap seconds to readjust with TAI time.
class UNIVERSALIS__DECL utc_since_epoch {
	public:
		nanoseconds static current();
};

/// a virtual clock that counts the time spent by the CPU(s) in the current process.
class UNIVERSALIS__DECL process {
	public:
		nanoseconds static current();
};

/// a virtual clock that counts the time spent by the CPU(s) in the current thread.
class UNIVERSALIS__DECL thread {
	public:
		nanoseconds static current();
};

/******************************************************************************************/
#if defined BOOST_AUTO_TEST_CASE
	BOOST_AUTO_TEST_CASE(wall_clock_and_sleep_test) {
		typedef monotonic clock;
		nanoseconds const sleep_nanoseconds(milliseconds(250));
		nanoseconds const t0(clock::current());
		this_thread::sleep(sleep_nanoseconds);
		double const ratio(double((clock::current() - t0).get_count()) / sleep_nanoseconds.get_count());
		{
			std::ostringstream s; s << ratio;
			BOOST_MESSAGE(s.str());
		}
		BOOST_CHECK(0.66 < ratio && ratio < 1.33);
	}
#endif

/******************************************************************************************/
namespace detail {
	/// iso std time.
	/// returns the time since the Epoch (00:00:00 UTC, January 1, 1970), measured in seconds.
	/// test result: clock: std::time, min: 1s, avg: 1s, max: 1s
	UNIVERSALIS__DECL nanoseconds iso_std_time() throw(exception);

	/// iso std clock.
	/// returns an approximation of processor time used by the program
	/// test result on colinux AMD64: clock: std::clock, min: 0.01s, avg: 0.01511s, max: 0.02s
	UNIVERSALIS__DECL nanoseconds iso_std_clock() throw(exception);

	#if defined DIVERSALIS__OS__POSIX
		namespace posix {
			bool extern clock_gettime_supported, clock_getres_supported, monotonic_clock_supported, process_cputime_supported, thread_cputime_supported;
			::clockid_t extern monotonic_clock_id, process_cputime_clock_id, thread_cputime_clock_id;
			void config();

			///\todo CLOCK_REALTIME_HR
			/// posix CLOCK_REALTIME_HR.
			/// System-wide realtime clock.
			/// High resolution version of CLOCK_REALTIME.
			//UNIVERSALIS__DECL std::nanoseconds realtime_hr() throw(std::runtime_error);
			
			///\todo CLOCK_MONOTONIC_HR
			/// posix CLOCK_MONOTONIC_HR.
			/// Clock that cannot be set and represents monotonic time since some unspecified starting point.
			/// High resolution version of CLOCK_MONOTONIC.
			//UNIVERSALIS__DECL std::nanoseconds monotonic_hr() throw(std::runtime_error);

			/// posix CLOCK_REALTIME.
			/// System-wide realtime clock.
			/// test result on colinux AMD64: clock: CLOCK_REALTIME, min: 1.3e-05s, avg: 1.6006e-05s, max: 0.001359s
			UNIVERSALIS__DECL nanoseconds realtime() throw(exception);

			/// posix CLOCK_MONOTONIC.
			/// Clock that cannot be set and represents monotonic time since some unspecified starting point.
			/// test result on colinux AMD64: clock: CLOCK_MONOTONIC, min: 1.3e-05s, avg: 1.606e-05s, max: 0.001745s
			UNIVERSALIS__DECL nanoseconds monotonic() throw(exception);

			/// posix CLOCK_PROCESS_CPUTIME_ID.
			/// High-resolution per-process timer from the CPU.
			/// Realized on many platforms using timers from the CPUs (TSC on i386,  AR.ITC on Itanium).
			/// test result on colinux AMD64: clock: CLOCK_PROCESS_CPUTIME_ID, min: 0.01s, avg: 0.01s, max: 0.01s
			UNIVERSALIS__DECL nanoseconds process_cpu_time() throw(exception);

			/// posix CLOCK_THREAD_CPUTIME_ID.
			/// Thread-specific CPU-time clock.
			/// Realized on many platforms using timers from the CPUs (TSC on i386,  AR.ITC on Itanium).
			/// test result on colinux AMD64: clock: CLOCK_THREAD_CPUTIME_ID, min: 0.01s, avg: 0.01s, max: 0.01s
			UNIVERSALIS__DECL nanoseconds thread_cpu_time() throw(exception);

			/// posix gettimeofday.
			/// test result on colinux AMD64: clock: gettimeofday, min: 1.3e-05s, avg: 1.5878e-05s, max: 0.001719s
			UNIVERSALIS__DECL nanoseconds time_of_day() throw(exception);
		}
	#elif defined DIVERSALIS__OS__MICROSOFT
		namespace microsoft {
			/// wall clock.
			/// ::QueryPerformanceCounter() is realised using timers from the CPUs (TSC on i386,  AR.ITC on Itanium).
			/// test result on AMD64: clock res: QueryPerformancefrequency: 3579545Hz (3.6MHz)
			/// test result on AMD64: clock: QueryPerformanceCounter, min: 3.073e-006s, avg: 3.524e-006s, max: 0.000375746s
			UNIVERSALIS__DECL nanoseconds performance_counter() throw(exception);

			/// wall clock.
			/// ::timeGetTime() is equivalent to ::GetTickCount() but Microsoft very loosely tries to express the idea that
			/// it might be more precise, especially if calling ::timeBeginPeriod and ::timeEndPeriod.
			/// Possibly realised using the PIT/PIC PC hardware.
			/// test result: clock: mmsystem timeGetTime, min: 0.001s, avg: 0.0010413s, max: 0.084s.
			UNIVERSALIS__DECL nanoseconds mme_system_time() throw(exception);

			/// wall clock.
			/// ::GetTickCount() returns the uptime (i.e., time elapsed since computer was booted), in milliseconds.
			/// Microsoft doesn't even specifies wether it's monotonic and as linear as possible, but we can probably assume so.
			/// This function returns a value which is read from a context value which is updated only on context switches,
			/// and hence is very inaccurate: it can lag behind the real clock value as much as 15ms, and sometimes more.
			/// Possibly realised using the PIT/PIC PC hardware.
			/// test result: clock: GetTickCount, min: 0.015s, avg: 0.015719s, max: 0.063s.
			UNIVERSALIS__DECL nanoseconds tick_count() throw(exception);

			/// wall clock.
			/// test result: clock: GetSystemTimeAsFileTime, min: 0.015625s, avg: 0.0161875s, max: 0.09375s.
			UNIVERSALIS__DECL nanoseconds system_time_as_file_time_since_epoch() throw(exception);

			/// virtual clock. kernel time not included.
			/// test result: clock: GetProcessTimes, min: 0.015625s, avg: 0.015625s, max: 0.015625s.
			UNIVERSALIS__DECL nanoseconds process_time() throw(exception);

			/// virtual clock. kernel time not included.
			/// The implementation of mswindows' ::GetThreadTimes() is completly broken: http://blog.kalmbachnet.de/?postid=28
			/// test result: clock: GetThreadTimes, min: 0.015625s, avg: 0.015625s, max: 0.015625s.
			UNIVERSALIS__DECL nanoseconds thread_time() throw(exception);
		}
	#endif // defined DIVERSALIS__OS__MICROSOFT

	#if defined BOOST_AUTO_TEST_CASE
		namespace test {
			typedef nanoseconds (*clock_function) ();

			/// measures the resolution of a clock and displays the result
			void measure_clock_resolution(std::string const & clock_name, clock_function clock, unsigned int count = 1000000) {
				nanoseconds min(std::days(1)), avg, max;
				for(unsigned int i(0); i < count; ++i) {
					unsigned long long int timeout(0);
					nanoseconds t;
					nanoseconds const t0(clock());
					do {
						t = clock();
						++timeout;
					} while(t == t0 && timeout < 1000 * 1000 * 10);
					if(t == t0) t = t0 + days(1); // reports the timeout as a bogus big value
					t -= t0;
					if(t < min) min = t;
					if(t > max) max = t;
					avg += t;
				}
				avg /= count;
				std::ostringstream s;
				s
					<< "clock: " << clock_name
					<< ", min: " << min.get_count() * 1e-9 << 's'
					<< ", avg: " << avg.get_count() * 1e-9 << 's'
					<< ", max: " << max.get_count() * 1e-9 << 's';
				BOOST_MESSAGE(s.str());
			}

			#if defined DIVERSALIS__OS__POSIX && (_POSIX_TIMERS > 0 || defined _SC_TIMERS )
				void display_clock_resolution(std::string const & clock_name, ::clockid_t clock) throw(std::runtime_error) {
					::timespec t;
					if(::clock_getres(clock, &t))
					{
						std::ostringstream s; s << exceptions::desc();
						throw std::runtime_error(s.str().c_str());
					}
					nanoseconds const ns(seconds(t.tv_sec) + nanoseconds(t.tv_nsec));
					std::ostringstream s;
					s
						<< "clock: " << clock_name
						<< ", resolution: " << ns.get_count() * 1e-9 << 's';
					BOOST_MESSAGE(s.str());
				}
			#endif
			
			BOOST_AUTO_TEST_CASE(clocks_test) {
				measure_clock_resolution("std::time", iso_std_time, 1);
				measure_clock_resolution("std::clock", iso_std_clock, 100);
				#if defined DIVERSALIS__OS__POSIX
					BOOST_MESSAGE("posix clocks");
					using namespace posix;
					config();
					measure_clock_resolution("CLOCK_REALTIME", realtime);
					if(monotonic_clock_supported) measure_clock_resolution("CLOCK_MONOTONIC", monotonic);
					if(process_cputime_supported) measure_clock_resolution("CLOCK_PROCESS_CPUTIME_ID", process_cpu_time);
					if(thread_cputime_supported) measure_clock_resolution("CLOCK_THREAD_CPUTIME_ID", thread_cpu_time);
					measure_clock_resolution("gettimeofday", time_of_day);

					if(clock_getres_supported) {
						#if _POSIX_TIMERS > 0 || defined _SC_TIMERS
							BOOST_MESSAGE("posix clock_getres");
							display_clock_resolution("CLOCK_REALTIME", CLOCK_REALTIME);
							#if _POSIX_MONOTONIC_CLOCK > 0 || defined _SC_MONOTONIC_CLOCK
								if(monotonic_clock_supported) display_clock_resolution("CLOCK_MONOTONIC", CLOCK_MONOTONIC);
							#endif
							#if _POSIX_CPUTIME > 0 || defined _SC_CPUTIME
								if(process_cputime_supported) display_clock_resolution("CLOCK_PROCESS_CPUTIME_ID", CLOCK_PROCESS_CPUTIME_ID);
							#endif
							#if _POSIX_THREAD_CPUTIME > 0 || defined _SC_THREAD_CPUTIME
								if(thread_cputime_supported) display_clock_resolution("CLOCK_THREAD_CPUTIME_ID", CLOCK_THREAD_CPUTIME_ID);
							#endif
						#endif
					} else {
							std::ostringstream s;
							s << "CLOCKS_PER_SEC: " << CLOCKS_PER_SEC;
							BOOST_MESSAGE(s.str());
					}
				#elif defined DIVERSALIS__OS__MICROSOFT
					BOOST_MESSAGE("microsoft clocks");
					using namespace microsoft;
					{
						::LARGE_INTEGER frequency;
						::QueryPerformanceFrequency(&frequency);
						std::ostringstream s; s << "clock resolution: QueryPerformanceFrequency: " << frequency.QuadPart << "Hz";
						BOOST_MESSAGE(s.str());
					}
					try {
						measure_clock_resolution("QueryPerformanceCounter", performance_counter, 100000);
					} catch(std::runtime_error const & e) {
						BOOST_MESSAGE(e.what());
					}
					measure_clock_resolution("mmsystem timeGetTime", mme_system_time, 1000);
					measure_clock_resolution("GetTickCount", tick_count, 10);
					measure_clock_resolution("GetSystemTimeAsFileTime", system_time_as_file_time_since_epoch, 10);
					measure_clock_resolution("GetProcessTimes", process_time, 10);
					measure_clock_resolution("GetThreadTimes", thread_time, 10);
				#endif
			}
		} // namespace test
	#endif // defined BOOST_AUTO_TEST_CASE
} // namespace detail
}}}

#endif
