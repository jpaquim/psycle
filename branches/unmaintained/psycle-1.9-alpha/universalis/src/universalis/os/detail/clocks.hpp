// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2004-2011 members of the psycle project http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>

#pragma once

#include <universalis/os/clocks.hpp>

#if defined DIVERSALIS__OS__POSIX
	#include <ctime>
	#include <cerrno>
	#include <cstring>
	#include <unistd.h>
#elif defined DIVERSALIS__OS__MICROSOFT
	#include <universalis/os/include_windows_without_crap.hpp>
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

#ifdef BOOST_AUTO_TEST_CASE
	#include <sstream>
	#include <string>
	#include <universalis/os/exception.hpp>
	#include <universalis/stdlib/detail/chrono/measure_clock.hpp>
#endif

namespace universalis { namespace os { namespace clocks { namespace detail {

// recommended: http://icl.cs.utk.edu/papi/custom/index.html?lid=62&slid=96

/// iso std time.
/// returns the time since the Epoch (00:00:00 UTC, January 1, 1970), measured in seconds.
/// test result: clock: std::time, min: 1s, avg: 1s, max: 1s
struct iso_std_time : public basic_clock<iso_std_time, false> {
	static UNIVERSALIS__DECL time_point now();
};

/// iso std clock.
/// returns an approximation of processor time used by the program
/// test result on colinux AMD64: clock: std::clock, min: 0.01s, avg: 0.01511s, max: 0.02s
struct iso_std_clock : public basic_clock<iso_std_clock, true> {
	static UNIVERSALIS__DECL time_point now();
};

#if defined DIVERSALIS__OS__POSIX
	namespace posix {
		bool extern clock_gettime_supported, clock_getres_supported, monotonic_clock_supported, process_cpu_time_supported, thread_cpu_time_supported;
		::clockid_t extern monotonic_clock_id, process_cpu_time_clock_id, thread_cpu_time_clock_id;
		void config();

		#if 0
			///\todo CLOCK_REALTIME_HR
			/// posix CLOCK_REALTIME_HR.
			/// System-wide realtime clock.
			/// High resolution version of CLOCK_REALTIME.
			struct real_time_hr : public basic_clock<real_time_hr, false> {
				static UNIVERSALIS__DECL time_point now();
			};

			///\todo CLOCK_MONOTONIC_HR
			/// posix CLOCK_MONOTONIC_HR.
			/// Clock that cannot be set and represents monotonic time since some unspecified starting point.
			/// High resolution version of CLOCK_MONOTONIC.
			struct monotonic_hr : public basic_clock<monotonic_hr, true> {
				static UNIVERSALIS__DECL time_point now();
			};
		#endif


		/// posix CLOCK_REALTIME.
		/// System-wide realtime clock.
		/// test result on colinux AMD64: clock: CLOCK_REALTIME, min: 1.3e-05s, avg: 1.6006e-05s, max: 0.001359s
		struct real_time : public basic_clock<real_time, false> {
			static UNIVERSALIS__DECL time_point now();
		};

		/// posix CLOCK_MONOTONIC.
		/// Clock that cannot be set and represents monotonic time since some unspecified starting point.
		/// test result on colinux AMD64: clock: CLOCK_MONOTONIC, min: 1.3e-05s, avg: 1.606e-05s, max: 0.001745s
		struct monotonic : public basic_clock<monotonic, true> {
			static UNIVERSALIS__DECL time_point now();
		};

		/// posix CLOCK_PROCESS_CPUTIME_ID.
		/// High-resolution per-process timer from the CPU.
		/// Realized on many platforms using timers from the CPUs (TSC on i386,  AR.ITC on Itanium).
		/// test result on colinux AMD64: clock: CLOCK_PROCESS_CPUTIME_ID, min: 0.01s, avg: 0.01s, max: 0.01s
		struct process_cpu_time : public basic_clock<process_cpu_time, true> {
			static UNIVERSALIS__DECL time_point now();
		};

		/// posix CLOCK_THREAD_CPUTIME_ID.
		/// Thread-specific CPU-time clock.
		/// Realized on many platforms using timers from the CPUs (TSC on i386,  AR.ITC on Itanium).
		/// test result on colinux AMD64: clock: CLOCK_THREAD_CPUTIME_ID, min: 0.01s, avg: 0.01s, max: 0.01s
		struct thread_cpu_time : public basic_clock<thread_cpu_time, true> {
			static UNIVERSALIS__DECL time_point now();
		};

		/// posix gettimeofday.
		/// test result on colinux AMD64: clock: gettimeofday, min: 1.3e-05s, avg: 1.5878e-05s, max: 0.001719s
		struct time_of_day : public basic_clock<time_of_day, false> {
			static UNIVERSALIS__DECL time_point now();
		};
	}
#elif defined DIVERSALIS__OS__MICROSOFT
	namespace microsoft {
		/// wall clock.
		/// ::QueryPerformanceCounter() is realised using timers from the CPUs (TSC on i386,  AR.ITC on Itanium).
		/// test result on AMD64: clock res: QueryPerformancefrequency: 3579545Hz (3.6MHz)
		/// test result on AMD64: clock: QueryPerformanceCounter, min: 3.073e-006s, avg: 3.524e-006s, max: 0.000375746s
		struct performance_counter : public basic_clock<performance_counter, false> {
			static UNIVERSALIS__DECL time_point now();
		};

		/// wall clock.
		/// ::timeGetTime() is equivalent to ::GetTickCount() but Microsoft very loosely tries to express the idea that
		/// it might be more precise, especially if calling ::timeBeginPeriod and ::timeEndPeriod.
		/// Possibly realised using the PIT/PIC PC hardware.
		/// test result: clock: mmsystem timeGetTime, min: 0.001s, avg: 0.0010413s, max: 0.084s.
		struct mme_system_time : public basic_clock<mme_system_time, true> {
			static UNIVERSALIS__DECL time_point now();
		};

		/// wall clock.
		/// ::GetTickCount() returns the uptime (i.e., time elapsed since computer was booted), in milliseconds.
		/// Microsoft doesn't even specifies wether it's monotonic and as linear as possible, but we can probably assume so.
		/// This function returns a value which is read from a context value which is updated only on context switches,
		/// and hence is very inaccurate: it can lag behind the real clock value as much as 15ms, and sometimes more.
		/// Possibly realised using the PIT/PIC PC hardware.
		/// test result: clock: GetTickCount, min: 0.015s, avg: 0.015719s, max: 0.063s.
		struct tick_count : public basic_clock<tick_count, true> {
			static UNIVERSALIS__DECL time_point now();
		};

		/// wall clock.
		/// test result: clock: GetSystemTimeAsFileTime, min: 0.015625s, avg: 0.0161875s, max: 0.09375s.
		struct system_time_as_file_time_since_epoch : public basic_clock<system_time_as_file_time_since_epoch, true> {
			static UNIVERSALIS__DECL time_point now();
		};

		/// virtual clock. kernel time not included.
		/// test result: clock: GetProcessTimes, min: 0.015625s, avg: 0.015625s, max: 0.015625s.
		struct process_time : public basic_clock<process_time, true> {
			static UNIVERSALIS__DECL time_point now();
		};

		/// virtual clock. kernel time not included.
		/// The implementation of mswindows' ::GetThreadTimes() is completly broken: http://blog.kalmbachnet.de/?postid=28
		/// test result: clock: GetThreadTimes, min: 0.015625s, avg: 0.015625s, max: 0.015625s.
		struct thread_time : public basic_clock<thread_time, true> {
			static UNIVERSALIS__DECL time_point now();
		};
	}
#endif // defined DIVERSALIS__OS__MICROSOFT

/******************************************************************************************/
#ifdef BOOST_AUTO_TEST_CASE
	namespace test {
		#if defined DIVERSALIS__OS__POSIX && (_POSIX_TIMERS > 0 || defined _SC_TIMERS )
			void display_clock_resolution(std::string const & clock_name, ::clockid_t clock) {
				::timespec t;
				if(::clock_getres(clock, &t)) throw exception(UNIVERSALIS__COMPILER__LOCATION__NO_CLASS);
				using namespace stdlib::chrono;
				nanoseconds const ns(seconds(t.tv_sec) + nanoseconds(t.tv_nsec));
				std::ostringstream s;
				s
					<< "clock: " << clock_name
					<< ", resolution: " << ns.count() * 1e-9 << 's';
				BOOST_MESSAGE(s.str());
			}
		#endif

		BOOST_AUTO_TEST_CASE(measure_clocks_os_detail_test) {
			using stdlib::chrono::test::measure_clock_against_sleep;
			using stdlib::chrono::test::measure_clock_resolution;
			#if 0 // takes too much time since resolution is only 1 second.
				measure_clock_against_sleep<iso_std_time>(stdlib::chrono::seconds(4));
			#endif
			#if 0 // illogic for this clock
				measure_clock_against_sleep<iso_std_clock>();
			#endif
			measure_clock_resolution<iso_std_time>(1);
			measure_clock_resolution<iso_std_clock>(100);
			#if defined DIVERSALIS__OS__POSIX
				BOOST_MESSAGE("posix clocks");
				using namespace posix;
				config();
				measure_clock_against_sleep<time_of_day>();
				measure_clock_resolution<time_of_day>();
				measure_clock_against_sleep<real_time>();
				measure_clock_resolution<real_time>();
				if(monotonic_clock_supported) {
					measure_clock_against_sleep<monotonic>();
					measure_clock_resolution<monotonic>();
				}
				if(process_cpu_time_supported) {
					#if 0 // illogic for this clock
						measure_clock_against_sleep<process_cpu_time>();
					#endif
					measure_clock_resolution<process_cpu_time>();
				}
				if(thread_cpu_time_supported) {
					#if 0 // illogic for this clock
						measure_clock_against_sleep<thread_cpu_time>();
					#endif
					measure_clock_resolution<thread_cpu_time>();
				}
				if(clock_getres_supported) {
					#if _POSIX_TIMERS > 0 || defined _SC_TIMERS
						BOOST_MESSAGE("posix clock_getres");
						display_clock_resolution("CLOCK_REALTIME", CLOCK_REALTIME);
						#if _POSIX_MONOTONIC_CLOCK > 0 || defined _SC_MONOTONIC_CLOCK
							if(monotonic_clock_supported) display_clock_resolution("CLOCK_MONOTONIC", CLOCK_MONOTONIC);
						#endif
						#if _POSIX_CPUTIME > 0 || defined _SC_CPUTIME
							if(process_cpu_time_supported) display_clock_resolution("CLOCK_PROCESS_CPUTIME_ID", CLOCK_PROCESS_CPUTIME_ID);
						#endif
						#if _POSIX_THREAD_CPUTIME > 0 || defined _SC_THREAD_CPUTIME
							if(thread_cpu_time_supported) display_clock_resolution("CLOCK_THREAD_CPUTIME_ID", CLOCK_THREAD_CPUTIME_ID);
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
				measure_clock_against_sleep<performance_counter>();
				measure_clock_against_sleep<mme_system_time>();
				measure_clock_against_sleep<system_time_as_file_time_since_epoch>();
				measure_clock_against_sleep<tick_count>();
				#if 0 // illogic for these clocks
					measure_clock_against_sleep<process_time>();
					measure_clock_against_sleep<thread_time>();
				#endif
				measure_clock_resolution<performance_counter>(100000);
				measure_clock_resolution<mme_system_time>(1000);
				measure_clock_resolution<system_time_as_file_time_since_epoch>(10);
				measure_clock_resolution<tick_count>(10);
				measure_clock_resolution<process_time>(10);
				measure_clock_resolution<thread_time>(10);
				{
					::LARGE_INTEGER frequency;
					::QueryPerformanceFrequency(&frequency);
					std::ostringstream s; s << "clock resolution: QueryPerformanceFrequency: " << frequency.QuadPart << "Hz";
					BOOST_MESSAGE(s.str());
				}
			#endif
		}
	} // namespace test
#endif // defined BOOST_AUTO_TEST_CASE

}}}}
