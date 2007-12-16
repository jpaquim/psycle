// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2004-2007 psycledelics http://psycle.pastnotecut.org ; johan boule <bohan@jabber.org>

///\interface universalis::operating_system::clocks
#pragma once
#include <universalis/compiler/cast.hpp>
#include <universalis/compiler/numeric.hpp>
#include <boost/operators.hpp>
#if defined DIVERSALIS__OPERATING_SYSTEM__POSIX
	#include <ctime>
	#include <cerrno>
	#include <cstring>
	#include <unistd.h>
#elif defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT
	#include <windows.h>
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
#include <cstdint>
#include <date_time>
#if defined BOOST_AUTO_TEST_CASE
	#include <universalis/operating_system/threads/sleep.hpp>
	#include <sstream>
#endif
#define UNIVERSALIS__COMPILER__DYNAMIC_LINK UNIVERSALIS__OPERATING_SYSTEM__CLOCKS
#include <universalis/compiler/dynamic_link/begin.hpp>
namespace universalis { namespace operating_system { namespace clocks {

// recommended: http://icl.cs.utk.edu/papi/custom/index.html?lid=62&slid=96

/******************************************************************************************/
#if defined universalis__operating_system__clocks__version__2
namespace clocks2 {
	/// a real time clock that counts the time elapsed since some unspecified origin.
	///
	/// The implementation reads, if available, the tick count register of some unspecified CPU.
	/// On most CPU architectures, the register is updated at a rate based on the frequency of the cycles, but often the count value and the tick events are unrelated,
	/// i.e. the value might not be incremented one by one. So the period corresponding to 1 count unit may be even smaller than the period of a CPU cycle, but should probably stay in the same order of magnitude.
	/// If the counter is increased by 4,000,000,000 over a second, and is 64-bit long, it is possible to count an uptime period in the order of a century without wrapping.
	/// The implementation for x86, doesn't work well at all on some of the CPUs whose frequency varies over time. This will eventually be fixed http://www.x86-secret.com/?option=newsd&nid=845.
	/// The implementation for mswindows is unpsecified on SMP systems.
	class UNIVERSALIS__COMPILER__DYNAMIC_LINK monotonic_wall {
		public:
			std::nanoseconds static current();
	};

	/// a real time clock that counts the UTC time elasped since the unix epoch (1970-01-01T00:00:00UTC).
	///
	/// This is the UTC time, and hence not monotonic since UTC has leap seconds to readjust with TAI time.
	class UNIVERSALIS__COMPILER__DYNAMIC_LINK utc_since_epoch {
		public:
			std::nanoseconds static current();
	};

	/// a virtual clock that counts the time spent by the CPU(s) in the current process.
	class UNIVERSALIS__COMPILER__DYNAMIC_LINK process {
		public:
			std::nanoseconds static current();
	};

	/// a virtual clock that counts the time spent by the CPU(s) in the current thread.
	class UNIVERSALIS__COMPILER__DYNAMIC_LINK thread {
		public:
			std::nanoseconds static current();
	};
}
#endif // defined universalis__operating_system__clocks__version__2

/******************************************************************************************/
typedef std::nanoseconds (*clock_function) ();
namespace detail {
	/// iso std time.
	/// returns the time since the Epoch (00:00:00 UTC, January 1, 1970), measured in seconds.
	/// test result: clock: std::time, min: 1s, avg: 1s, max: 1s
	UNIVERSALIS__COMPILER__DYNAMIC_LINK std::nanoseconds iso_std_time() throw(std::runtime_error);

	#if defined DIVERSALIS__OPERATING_SYSTEM__POSIX
		namespace posix_clocks {
			/// posix CLOCK_REALTIME.
			/// System-wide realtime clock.
			/// test result on colinux AMD64: clock: CLOCK_REALTIME, min: 1.4e-05s, avg: 1.5875e-05s, max: 0.000385s
			UNIVERSALIS__COMPILER__DYNAMIC_LINK std::nanoseconds realtime() throw(std::runtime_error);

			/// posix CLOCK_MONOTONIC.
			/// Clock that cannot be set and represents monotonic time since some unspecified starting point.
			/// test result on colinux AMD64: clock: CLOCK_MONOTONIC, min: 1.4e-05s, avg: 1.5347e-05s, max: 0.000213s
			UNIVERSALIS__COMPILER__DYNAMIC_LINK std::nanoseconds monotonic() throw(std::runtime_error);

			/// posix CLOCK_PROCESS_CPUTIME_ID.
			/// High-resolution per-process timer from the CPU.
			/// Realized on many platforms using timers from the CPUs (TSC on i386,  AR.ITC on Itanium).
			/// test result on colinux AMD64: clock: CLOCK_PROCESS_CPUTIME_ID, min: 0.01s, avg: 0.01s, max: 0.01s
			UNIVERSALIS__COMPILER__DYNAMIC_LINK std::nanoseconds process_cpu_time() throw(std::runtime_error);

			/// posix CLOCK_THREAD_CPUTIME_ID.
			/// Thread-specific CPU-time clock.
			/// Realized on many platforms using timers from the CPUs (TSC on i386,  AR.ITC on Itanium).
			/// test result on colinux AMD64: clock: CLOCK_THREAD_CPUTIME_ID, min: 0.01s, avg: 0.01s, max: 0.01s
			UNIVERSALIS__COMPILER__DYNAMIC_LINK std::nanoseconds thread_cpu_time() throw(std::runtime_error);

			/// posix gettimeofday.
			/// test result on colinux AMD64: clock: gettimeofday, min: 1.3e-05s, avg: 1.6722e-05s, max: 0.001051s
			UNIVERSALIS__COMPILER__DYNAMIC_LINK std::nanoseconds time_of_day() throw(std::runtime_error);
		}
	#elif defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT
		namespace microsoft_clocks {
			/// wall clock.
			/// ::QueryPerformanceCounter() is realised using timers from the CPUs (TSC on i386,  AR.ITC on Itanium).
			/// test result on AMD64: clock res: QueryPerformancefrequency: 3579545Hz (3.6MHz)
			/// test result on AMD64: clock: QueryPerformanceCounter, min: 3.073e-006s, avg: 3.524e-006s, max: 0.000375746s
			UNIVERSALIS__COMPILER__DYNAMIC_LINK std::nanoseconds performance_counter_nanoseconds() throw(std::runtime_error);

			/// wall clock.
			/// ::timeGetTime() is equivalent to ::GetTickCount() but Microsoft very loosely tries to express the idea that
			/// it might be more precise, especially if calling ::timeBeginPeriod and ::timeEndPeriod.
			/// Possibly realised using the PIT/PIC PC hardware.
			/// test result: clock: mmsystem timeGetTime, min: 0.001s, avg: 0.0010413s, max: 0.084s.
			UNIVERSALIS__COMPILER__DYNAMIC_LINK std::nanoseconds mme_system_time_nanoseconds();

			/// wall clock.
			/// ::GetTickCount() returns the uptime (i.e., time elapsed since computer was booted), in milliseconds.
			/// Microsoft doesn't even specifies wether it's monotonic and as linear as possible, but we can probably assume so.
			/// This function returns a value which is read from a context value which is updated only on context switches,
			/// and hence is very inaccurate: it can lag behind the real clock value as much as 15ms, and sometimes more.
			/// Possibly realised using the PIT/PIC PC hardware.
			/// test result: clock: GetTickCount, min: 0.015s, avg: 0.015719s, max: 0.063s.
			UNIVERSALIS__COMPILER__DYNAMIC_LINK std::nanoseconds tick_count_nanoseconds();

			/// wall clock.
			/// test result: clock: GetSystemTimeAsFileTime, min: 0.015625s, avg: 0.0161875s, max: 0.09375s.
			UNIVERSALIS__COMPILER__DYNAMIC_LINK std::nanoseconds system_time_as_file_time_nanoseconds_since_epoch();

			/// virtual clock. kernel time not included.
			/// test result: clock: GetProcessTimes, min: 0.015625s, avg: 0.015625s, max: 0.015625s.
			UNIVERSALIS__COMPILER__DYNAMIC_LINK std::nanoseconds process_times_nanoseconds();

			/// virtual clock. kernel time not included.
			/// The implementation of mswindows' ::GetThreadTimes() is completly broken: http://blog.kalmbachnet.de/?postid=28
			/// test result: clock: GetThreadTimes, min: 0.015625s, avg: 0.015625s, max: 0.015625s.
			UNIVERSALIS__COMPILER__DYNAMIC_LINK std::nanoseconds thread_times_nanoseconds();
		}
	#endif // defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT

	#if defined BOOST_AUTO_TEST_CASE
		namespace test {
			/// measures the resolution of a clock and displays the result
			void measure_clock_resolution(std::string const & clock_name, clock_function clock, unsigned int count = 6) {
				std::nanoseconds min(std::days(1)), avg, max;
				for(unsigned int i(0); i < count; ++i) {
					unsigned long long int timeout(0);
					std::nanoseconds t;
					std::nanoseconds const t0(clock());
					do {
						t = clock();
						++timeout;
					} while(t == t0 && timeout < 1000 * 1000 * 10);
					if(t == t0) t = t0 + std::days(1); // reports the timeout as a bogus big value
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

			BOOST_AUTO_TEST_CASE(clocks_test)
			{
				measure_clock_resolution("std::time", iso_std_time, 1);
				#if defined DIVERSALIS__OPERATING_SYSTEM__POSIX
					BOOST_MESSAGE("posix clocks");
					using namespace posix_clocks;
					measure_clock_resolution("CLOCK_REALTIME", realtime);
					measure_clock_resolution("CLOCK_MONOTONIC", monotonic);
					measure_clock_resolution("CLOCK_PROCESS_CPUTIME_ID", process_cpu_time);
					measure_clock_resolution("CLOCK_THREAD_CPUTIME_ID", thread_cpu_time);
					measure_clock_resolution("gettimeofday", time_of_day);
				#elif defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT
					BOOST_MESSAGE("microsoft clocks");
					using namespace microsoft_clocks;
					{
						::LARGE_INTEGER frequency;
						::QueryPerformanceFrequency(&frequency);
						std::ostringstream s; s << "clock res: QueryPerformancefrequency: " << frequency.QuadPart << "Hz";
						BOOST_MESSAGE(s.str());
					}
					measure_clock_resolution("QueryPerformanceCounter", performance_counter_nanoseconds, 10000);
					measure_clock_resolution("mmsystem timeGetTime", mme_system_time_nanoseconds);
					measure_clock_resolution("GetTickCount", tick_count_nanoseconds);
					measure_clock_resolution("GetSystemTimeAsFileTime", system_time_as_file_time_nanoseconds_since_epoch);
					measure_clock_resolution("GetProcessTimes", process_times_nanoseconds);
					measure_clock_resolution("GetThreadTimes", thread_times_nanoseconds);
				#endif
			}
		}
	#endif // defined BOOST_AUTO_TEST_CASE
} // namespace detail

/******************************************************************************************/
/// a time value in SI units (i.e. seconds)
typedef universalis::compiler::numeric<>::floating_point real_time;

/// a time value with an implementation-defined internal representation.
/// convertible to/from real_time
class opaque_time
:
	public universalis::compiler::cast::underlying_value_wrapper
	<
			#if defined DIVERSALIS__OPERATING_SYSTEM__POSIX
				::timespec
			#else
				std::/*u*/int64_t
			#endif
	>,
	private
		boost::additive< opaque_time, boost::less_than_comparable<opaque_time> >
{
	public:
		opaque_time() {}
		opaque_time(underlying_type const & underlying) : underlying_wrapper_type(underlying) {}

		opaque_time & operator=(opaque_time const & other) {
			static_cast<underlying_type&>(*this) = static_cast<underlying_type const &>(other);
			return *this;
		}
		
		opaque_time & operator+=(opaque_time const & other) {
			underlying_type & u(*this);
			underlying_type const & o(other);
			#if defined DIVERSALIS__OPERATING_SYSTEM__POSIX
				u.tv_sec += o.tv_sec;
				u.tv_nsec += o.tv_nsec;
				if(u.tv_nsec >= 1000000000) { u.tv_nsec -= 1000000000; ++u.tv_sec; }
			#else
				u += o;
			#endif
			return *this;
		}

		opaque_time & operator-=(opaque_time const & other) {
			underlying_type & u(*this);
			underlying_type const & o(other);
			#if defined DIVERSALIS__OPERATING_SYSTEM__POSIX
				u.tv_sec -= o.tv_sec;
				if(u.tv_nsec > o.tv_nsec) u.tv_nsec -= o.tv_nsec;
				else { --u.tv_sec; u.tv_nsec += 1000000000 - o.tv_nsec; }
			#else
				u -= o;
			#endif
			return *this;
		}

		bool operator<(opaque_time const & other) const {
			underlying_type const & u(*this);
			underlying_type const & o(other);
			#if defined DIVERSALIS__OPERATING_SYSTEM__POSIX
				if(u.tv_sec < o.tv_sec) return true;
				if(u.tv_sec > o.tv_sec) return false;
				return u.tv_nsec < o.tv_nsec;
			#else
				return u < o;
			#endif
		}

		///\name conversion from/to real_time
		///\{
			/// conversion from real_time.
			opaque_time static from_real_time(real_time seconds) {
				opaque_time result;
				underlying_type & u(result);
				#if defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT
					u = static_cast<underlying_type>(seconds * 1e7); // ::FILETIME resolution
				#else
					u.tv_sec =  static_cast<time_t>(seconds);
					u.tv_nsec = static_cast<long int>((seconds - u.tv_sec) * 1e9);
				#endif
				return result;
			}
			
			/// conversion to real_time.
			///\return the time value in SI units (i.e. seconds).
			real_time to_real_time() const {
				real_time result;
				underlying_type const & u(*this);
				#if defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT
					result = u * 1e-7; // ::FILETIME resolution
				#else
					result = u.tv_sec + u.tv_nsec * 1e-9;
				#endif
				return result;
			}
		///\}
};

/// a real time clock that counts the time elapsed since some unspecified origin.
///
/// The implementation reads, if available, the tick count register of some unspecified CPU.
/// On most CPU architectures, the register is updated at a rate based on the frequency of the cycles, but often the count value and the tick events are unrelated,
/// i.e. the value might not be incremented one by one. So the period corresponding to 1 count unit may be even smaller than the period of a CPU cycle, but should probably stay in the same order of magnitude.
/// If the counter is increased by 4,000,000,000 over a second, and is 64-bit long, it is possible to count an uptime period in the order of a century without wrapping.
/// The implementation for x86, doesn't work well at all on some of the CPUs whose frequency varies over time. This will eventually be fixed http://www.x86-secret.com/?option=newsd&nid=845.
/// The implementation for mswindows is unpsecified on SMP systems.
class UNIVERSALIS__COMPILER__DYNAMIC_LINK monotonic_wall {
	public:
		opaque_time static current();
};

/// a real time clock that counts the UTC time elasped since the unix epoch (1970-01-01T00:00:00UTC).
///
/// This is the UTC time, and hence not monotonic since UTC has leap seconds to readjust with TAI time.
class UNIVERSALIS__COMPILER__DYNAMIC_LINK utc_since_epoch {
	public:
		opaque_time static current();
};

/// a virtual clock that counts the time spent by the CPU(s) in the current process (kernel included?).
class UNIVERSALIS__COMPILER__DYNAMIC_LINK process {
	public:
		opaque_time static current();
};

/// a virtual clock that counts the time spent by the CPU(s) in the current thread (kernel included?).
class UNIVERSALIS__COMPILER__DYNAMIC_LINK thread {
	public:
		opaque_time static current();
};

#if defined BOOST_AUTO_TEST_CASE
	BOOST_AUTO_TEST_CASE(wall_clock_test)
	{
		typedef monotonic_wall clock;
		real_time const sleep_seconds(0.25);
		opaque_time const start(clock::current());
		universalis::operating_system::threads::sleep(sleep_seconds);
		double const ratio((clock::current() - start).to_real_time() / sleep_seconds);
		{
			std::ostringstream s; s << ratio;
			BOOST_MESSAGE(s.str());
		}
		BOOST_CHECK(0.75 < ratio && ratio < 1.25);
	}
#endif

}}}
#include <universalis/compiler/dynamic_link/end.hpp>
