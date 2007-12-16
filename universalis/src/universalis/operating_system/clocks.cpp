// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2004-2007 psycledelics http://psycle.pastnotecut.org ; johan boule <bohan@jabber.org>

///\implementation universalis::operating_system::clocks
#include <packageneric/pre-compiled.private.hpp>
#include <packageneric/module.private.hpp>
#include <universalis/detail/project.private.hpp>
#include "clocks.hpp"
#include <universalis/operating_system/exceptions/code_description.hpp>
#if defined DIVERSALIS__OPERATING_SYSTEM__POSIX
	#include <unistd.h>
	#include <sys/time.h>
#endif
#include <ctime>
namespace universalis { namespace operating_system { namespace clocks {

// recommended: http://icl.cs.utk.edu/papi/custom/index.html?lid=62&slid=96

#if defined DIVERSALIS__OPERATING_SYSTEM__POSIX

	namespace {

		void error(int const & code = errno) {
			std::cerr << "error: " << code << ": " << ::strerror(code) << std::endl;
		}

		bool supported(int const & option) {
			long int result(::sysconf(option));
			if(result < -1) error();
			return result > 0;
		}

		::clockid_t static wall_clock_id, process_clock_id, thread_clock_id;

		void best() {
			bool timers_supported, cpu_time_supported, monotonic_clock_supported;

			wall_clock_id = process_clock_id = thread_clock_id = CLOCK_REALTIME;

			#if !_POSIX_TIMERS && defined DIVERSALIS__COMPILER__GNU
				#warning will use posix sysconf at runtime to determine whether this OS supports timers: !_POSIX_TIMERS
			#elif _POSIX_TIMERS == -1 && defined DIVERSALIS__COMPILER__GNU
				#warning this OS does not support posix timers: _POSIX_TIMERS == -1
				#define UNIVERSALIS__OPERATING_SYSTEM__CLOCKS__DETAIL 0
			#elif _POSIX_TIMERS > 0
				// beware: cygwin defines this because it has clock_gettime, but it doesn't have clock_getres.
				#if defined DIVERSALIS__OPERATING_SYSTEM__CYGWIN && defined DIVERSALIS__COMPILER__GNU
					#warning ignoring posix timers on cygwin: _POSIX_TIMERS > 0, but this is cygwin. \
						Cygwin only partially implements this posix option: it has ::clock_gettime, but not ::clock_getres. \
						Moreover, ::sysconf(_SC_TIMERS) would return 0, which condradicts _POSIX_TIMERS > 0. \
						We will consider that it simply implements nothing.
				#else
					#define UNIVERSALIS__OPERATING_SYSTEM__CLOCKS__DETAIL 1
				#endif
			#endif
			#if !defined _SC_TIMERS && defined DIVERSALIS__COMPILER__GNU
				#warning cannot use posix sysconf at runtime to determine whether this OS supports timers: !defined _SC_TIMERS
				#if !defined UNIVERSALIS__OPERATING_SYSTEM__CLOCKS__DETAIL
					#define UNIVERSALIS__OPERATING_SYSTEM__CLOCKS__DETAIL 0
					timers_supported = false;
				#endif
			#else
				timers_supported = supported(_SC_TIMERS);
			#endif
			
			// CPUTIME
			#if !_POSIX_CPUTIME && defined DIVERSALIS__COMPILER__GNU
				#warning will use posix sysconf at runtime to determine whether this OS supports cpu time: !_POSIX_CPUTIME
			#elif _POSIX_CPUTIME == -1 && defined DIVERSALIS__COMPILER__GNU
				#warning this OS does not support posix cpu time: _POSIX_CPUTIME == -1
				#undef  UNIVERSALIS__OPERATING_SYSTEM__CLOCKS__DETAIL
				#define UNIVERSALIS__OPERATING_SYSTEM__CLOCKS__DETAIL 0
			#elif _POSIX_CPUTIME > 0
				#undef  UNIVERSALIS__OPERATING_SYSTEM__CLOCKS__DETAIL
				#define UNIVERSALIS__OPERATING_SYSTEM__CLOCKS__DETAIL 1
			#endif
			#if !defined _SC_CPUTIME && defined DIVERSALIS__COMPILER__GNU
				#warning cannot use posix sysconf at runtime to determine whether this OS supports cpu time: !defined _SC_CPUTIME
				#if !defined UNIVERSALIS__OPERATING_SYSTEM__CLOCKS__DETAIL
					#define UNIVERSALIS__OPERATING_SYSTEM__CLOCKS__DETAIL 0
					cpu_time_supported = false;
				#endif
			#else
				cpu_time_supported = supported(_SC_CPUTIME);
				if(cpu_time_supported) {
					::clockid_t clock_id;
					if(clock_getcpuclockid(0, &clock_id) == ENOENT) {
						// this SMP system makes CLOCK_PROCESS_CPUTIME_ID and CLOCK_THREAD_CPUTIME_ID inconsistent
						cpu_time_supported = false;
					} else {
						wall_clock_id = process_clock_id = thread_clock_id = clock_id;//CLOCK_PROCESS_CPUTIME_ID;
					}
				}
			#endif
		
			// MONOTONIC_CLOCK
			#if !_POSIX_MONOTONIC_CLOCK && defined DIVERSALIS__COMPILER__GNU
				#warning will use posix sysconf at runtime to determine whether this OS supports monotonic clock: !_POSIX_MONOTONIC_CLOCK
			#elif _POSIX_MONOTONIC_CLOCK == -1 && defined DIVERSALIS__COMPILER__GNU
				#warning this OS does not support posix monotonic clock: _POSIX_MONOTONIC_CLOCK == -1
				#undef  UNIVERSALIS__OPERATING_SYSTEM__CLOCKS__DETAIL
				#define UNIVERSALIS__OPERATING_SYSTEM__CLOCKS__DETAIL 0
			#elif _POSIX_MONOTONIC_CLOCK > 0
				#undef  UNIVERSALIS__OPERATING_SYSTEM__CLOCKS__DETAIL
				#define UNIVERSALIS__OPERATING_SYSTEM__CLOCKS__DETAIL 1
			#endif
			#if !defined _SC_MONOTONIC_CLOCK && defined DIVERSALIS__COMPILER__GNU
				#warning cannot use posix sysconf at runtime to determine whether this OS supports monotonic clock: !defined _SC_MONOTONIC_CLOCK
				#if !defined UNIVERSALIS__OPERATING_SYSTEM__CLOCKS__DETAIL
					#define UNIVERSALIS__OPERATING_SYSTEM__CLOCKS__DETAIL 0
				#endif
			#else
			{
				monotonic_clock_supported = supported(_SC_MONOTONIC_CLOCK);
				if(monotonic_clock_supported) {
					if(!cpu_time_supported) wall_clock_id = process_clock_id = thread_clock_id = CLOCK_MONOTONIC;
				}
			}
			#endif
		}
	}

	#if defined BOOST_AUTO_TEST_CASE
		BOOST_AUTO_TEST_CASE(clock_best_test)
		{
			::clockid_t clock(universalis::operating_system::clocks::best());
			timespec result;
			#if UNIVERSALIS__OPERATING_SYSTEM__CLOCKS__DETAIL
				BOOST_MESSAGE("using ::clock_getres");
				if(::clock_getres(clock, &result))
				{
					std::ostringstream s; s << exceptions::code_description();
					throw std::runtime_error(s.str().c_str());
				}
			#else
				BOOST_MESSAGE("using CLOCKS_PER_SEC");
				result.tv_sec = 1 / CLOCKS_PER_SEC;
				result.tv_nsec = static_cast<long int>(1e9 / CLOCKS_PER_SEC);
			#endif
			std::ostringstream s;
			s << "clock resolution: " << result.tv_sec << "s + " << result.tv_nsec * 1e-9 << "s";
			BOOST_MESSAGE(s.str());
		}
	#endif
#endif // defined DIVERSALIS__OPERATING_SYSTEM__POSIX

opaque_time monotonic_wall::current() {
	opaque_time result;
	opaque_time::underlying_type & u(result);
	#if defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT
		::LARGE_INTEGER counter, frequency;
		if(::QueryPerformanceCounter(&counter) && ::QueryPerformanceFrequency(&frequency)) {
			u = counter.QuadPart * 1e7 / frequency.QuadPart; // to ::FILETIME
		} else { // The CPU has no tick count register.
			#if 1
				/// Use the PIT/PIC PC hardware via mswindows' ::timeGetTime() or ::GetTickCount() instead.
				class set_timer_resolution {
					private: ::UINT milliseconds;
					public:
						set_timer_resolution() {
							// tries to get the best possible resolution, starting with 1ms
							milliseconds = 1;
							retry:
							if(::timeBeginPeriod(milliseconds) == TIMERR_NOCANDO) {
								if(++milliseconds < 50) goto retry;
								else milliseconds = 0; // give up :-(
							}
						}
						~set_timer_resolution() throw() {
							if(!milliseconds) return; // wasn't set
							if(::timeEndPeriod(milliseconds) == TIMERR_NOCANDO) return; // cannot throw in a destructor
							//throw std::runtime_error(GetLastErrorString());
						}
				};
				static set_timer_resolution once;
				result = ::timeGetTime() * 1000000; // milliseconds to nanoseconds
					// ::timeGetTime() equivalent to ::GetTickCount() but Microsoft very loosely tries to express the idea that it might be more precise, especially if calling ::timeBeginPeriod and ::timeEndPeriod.
					//
					// ::GetTickCount()
					// uptime (i.e., time elapsed since computer was booted), in milliseconds. Microsoft doesn't even specifies wether it's monotonic and as linear as possible, but we can probably assume so.
					// This function returns a value which is read from a context value which is updated only on context switches, and hence is very inaccurate: it can lag behind the real clock value as much as 15ms.
			#else
				::FILETIME ft;
				::GetSystemTimeAsFileTime(&ft);

				::LARGE_INTEGER li;
				li.LowPart  = ft.dwLowDateTime;
				li.HighPart = ft.dwHighDateTime;

				std::int64_t i = li.QuadPart; // in 100-nanosecond intervals
				
				// microsoft disregards the unix/posix epoch time, so we need to apply an offset
				std::int64_t const microsoft_file_time_to_unix_epoch_offset(116444736000000000LL);
				i -= microsoft_file_time_to_unix_epoch_offset;

				result = i * 100; // conversion from 100-nanosecond intervals to nanoseconds
			#endif
		}
	#else
		static bool once(false); if(!once) best();
		::clock_gettime(wall_clock_id, &u);
	#endif
	return result;
}

opaque_time process::current() {
	opaque_time result;
	opaque_time::underlying_type & u(result);
	#if defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT
		::FILETIME creation, exit, kernel, user;
		::GetProcessTimes(::GetCurrentProcess(), &creation, &exit, &kernel, &user);
		union winapi_is_badly_designed // real unit 1e-7 seconds
		{
			::FILETIME file_time;
			::LARGE_INTEGER large_integer;
		} u1, u2;
		u1.file_time = user;
		u2.file_time = kernel;
		u = u1.large_integer.QuadPart + u2.large_integer.QuadPart;
	#else
		static bool once(false); if(!once) best();
		::clock_gettime(process_clock_id, &u);
	#endif
	return result;
}

opaque_time thread::current() {
	opaque_time result;
	opaque_time::underlying_type & u(result);
	#if defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT
		#if 0 // The implementation of mswindows' ::GetThreadTimes() is completly broken: http://blog.kalmbachnet.de/?postid=28
			::FILETIME creation, exit, kernel, user;
			::GetThreadTimes(::GetCurrentThread(), &creation, &exit, &kernel, &user);
			union winapi_is_badly_designed // real unit 1e-7 seconds
			{
				::FILETIME file_time;
				::LARGE_INTEGER large_integer;
			} u1, u2;
			u1.file_time = user;
			u2.file_time = kernel;
			u = u1.large_integer.QuadPart + u2.large_integer.QuadPart;
		#else // Use the wall clock instead, which majorates all virtual subclocks.
			u = monotonic_wall::current().underlying();
		#endif
	#else
		static bool once(false); if(!once) best();
		::clock_gettime(thread_clock_id, &u);
	#endif
	return result;
}

/******************************************************************************************/
typedef std::nanoseconds (*clock_function) ();
namespace detail {
	/// iso std time.
	/// returns the time since the Epoch (00:00:00 UTC, January 1, 1970), measured in seconds.
	std::nanoseconds iso_std_time() throw(std::runtime_error) {
		std::time_t const t(std::time(0));
		if(t < 0) {
			std::ostringstream s; s << exceptions::code_description();
			throw std::runtime_error(s.str().c_str());
		}
		std::nanoseconds ns = std::seconds(t);
		return ns;
	}

	#if defined DIVERSALIS__OPERATING_SYSTEM__POSIX
		namespace posix_clocks {
			namespace {
				std::nanoseconds get(::clockid_t clock) throw(std::runtime_error) {
					::timespec t;
					if(::clock_gettime(clock, &t)) {
						std::ostringstream s; s << exceptions::code_description();
						throw std::runtime_error(s.str().c_str());
					}
					std::nanoseconds ns(std::nanoseconds(t.tv_nsec) + std::seconds(t.tv_sec));
					return ns;
				}
			}

			/// posix CLOCK_REALTIME.
			/// System-wide realtime clock.
			std::nanoseconds realtime() throw(std::runtime_error) { return get(CLOCK_REALTIME); }

			/// posix CLOCK_MONOTONIC.
			/// Clock that cannot be set and represents monotonic time since some unspecified starting point.
			std::nanoseconds monotonic() throw(std::runtime_error) { return get(CLOCK_MONOTONIC); }

			/// posix CLOCK_PROCESS_CPUTIME_ID.
			/// High-resolution per-process timer from the CPU.
			/// Realized on many platforms using timers from the CPUs (TSC on i386,  AR.ITC on Itanium).
			std::nanoseconds process_cpu_time() throw(std::runtime_error) { return get(CLOCK_PROCESS_CPUTIME_ID); }

			/// posix CLOCK_THREAD_CPUTIME_ID.
			/// Thread-specific CPU-time clock.
			/// Realized on many platforms using timers from the CPUs (TSC on i386,  AR.ITC on Itanium).
			std::nanoseconds thread_cpu_time() throw(std::runtime_error) { return get(CLOCK_THREAD_CPUTIME_ID); }

			/// posix gettimeofday.
			std::nanoseconds time_of_day() throw(std::runtime_error) {
				::timeval t;
				if(::gettimeofday(&t, 0)) { // second argument passed is 0 for no timezone
					std::ostringstream s; s << exceptions::code_description();
					throw std::runtime_error(s.str().c_str());
				}
				std::nanoseconds ns(std::microseconds(t.tv_usec) + std::seconds(t.tv_sec));
				return ns;
			}
		}
	#elif defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT
		namespace microsoft_clocks {
			/// clock: QueryPerformanceCounter.
			/// wall clock.
			/// ::QueryPerformanceCounter() is realised using timers from the CPUs (TSC on i386,  AR.ITC on Itanium).
			std::nanoseconds performance_counter_nanoseconds() throw(std::runtime_error) {
				::LARGE_INTEGER counter, frequency;
				if(!::QueryPerformanceCounter(&counter) || !::QueryPerformanceFrequency(&frequency)) {
					std::ostringstream s; s << exceptions::code_description();
					throw std::runtime_error(s.str().c_str());
				}
				// tested on AMD64: QueryPerformancefrequency: 3579545Hz (3.6MHz)
				std::nanoseconds ns(counter.QuadPart * 1000 * 1000 * 1000 / frequency.QuadPart);
				return ns;
			}

			/// clock: mmsystem timeGetTime, min: 0.001s, avg: 0.0010413s, max: 0.084s.
			/// wall clock.
			/// ::timeGetTime() is equivalent to ::GetTickCount() but Microsoft very loosely tries to express the idea that
			/// it might be more precise, especially if calling ::timeBeginPeriod and ::timeEndPeriod.
			/// Possibly realised using the PIT/PIC PC hardware.
			std::nanoseconds mme_system_time_nanoseconds() {
				class set_timer_resolution {
					private: ::UINT milliseconds;
					public:
						set_timer_resolution() {
							// tries to get the best possible resolution, starting with 1ms (which is the minimum supported anyway)
							milliseconds = 1;
							retry:
							if(::timeBeginPeriod(milliseconds) == TIMERR_NOCANDO) {
								if(++milliseconds < 50) goto retry;
								else milliseconds = 0; // give up :-(
							}
						}
						~set_timer_resolution() throw() {
							if(!milliseconds) return; // wasn't set
							if(::timeEndPeriod(milliseconds) == TIMERR_NOCANDO)
								return; // cannot throw in a destructor
								//throw std::runtime_error(GetLastErrorString());
						}
				} static once;
				std::nanoseconds ns(1000LL * 1000 * ::timeGetTime());
				return ns;
			}

			/// clock: GetTickCount, min: 0.015s, avg: 0.015719s, max: 0.063s.
			/// wall clock.
			/// ::GetTickCount() returns the uptime (i.e., time elapsed since computer was booted), in milliseconds.
			/// Microsoft doesn't even specifies wether it's monotonic and as linear as possible, but we can probably assume so.
			/// This function returns a value which is read from a context value which is updated only on context switches,
			/// and hence is very inaccurate: it can lag behind the real clock value as much as 15ms, and sometimes more.
			/// Possibly realised using the PIT/PIC PC hardware.
			std::nanoseconds tick_count_nanoseconds() {
				std::nanoseconds ns(1000LL * 1000 * ::GetTickCount());
				return ns;
			}

			/// clock: GetSystemTimeAsFileTime, min: 0.015625s, avg: 0.0161875s, max: 0.09375s.
			/// wall clock.
			std::nanoseconds system_time_as_file_time_nanoseconds_since_epoch() {
				union winapi_is_badly_designed {
					::FILETIME file_time;
					::LARGE_INTEGER large_integer;
				} u;
				::GetSystemTimeAsFileTime(&u.file_time);
				u.large_integer.QuadPart -= 116444736000000000LL; // microsoft disregards the unix/posix epoch time, so we need to apply an offset
				std::nanoseconds ns(100 * u.large_integer.QuadPart);
				return ns;
			}

			/// clock: GetProcessTimes, min: 0.015625s, avg: 0.015625s, max: 0.015625s.
			/// virtual clock. kernel time not included.
			std::nanoseconds process_times_nanoseconds() {
				union winapi_is_badly_designed {
					::FILETIME file_time;
					::LARGE_INTEGER large_integer;
				} creation, exit, kernel, user;
				::GetProcessTimes(::GetCurrentProcess(), &creation.file_time, &exit.file_time, &kernel.file_time, &user.file_time);
				std::nanoseconds ns(100 * user.large_integer.QuadPart);
				return ns;
			}

			/// clock: GetThreadTimes, min: 0.015625s, avg: 0.015625s, max: 0.015625s.
			/// virtual clock. kernel time not included.
			/// The implementation of mswindows' ::GetThreadTimes() is completly broken: http://blog.kalmbachnet.de/?postid=28
			std::nanoseconds thread_times_nanoseconds() {
				union winapi_is_badly_designed {
					::FILETIME file_time;
					::LARGE_INTEGER large_integer;
				} creation, exit, kernel, user;
				::GetThreadTimes(::GetCurrentThread(), &creation.file_time, &exit.file_time, &kernel.file_time, &user.file_time);
				std::nanoseconds ns(100 * user.large_integer.QuadPart);
				return ns;
			}
		}
	#endif // defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT
}
}}}
