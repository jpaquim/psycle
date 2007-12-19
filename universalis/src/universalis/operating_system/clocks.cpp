// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2004-2007 psycledelics http://psycle.pastnotecut.org ; johan boule <bohan@jabber.org>

///\implementation universalis::operating_system::clocks
#include <packageneric/pre-compiled.private.hpp>
#include <packageneric/module.private.hpp>
#include <universalis/detail/project.private.hpp>
#include "clocks.hpp"
#include <universalis/operating_system/exceptions/code_description.hpp>
#if defined DIVERSALIS__OPERATING_SYSTEM__POSIX
	#include <unistd.h> // for sysconf
	#include <sys/time.h>
#endif
#include <ctime>
#include <iostream>
#include <sstream>
namespace universalis { namespace operating_system { namespace clocks {

// recommended: http://icl.cs.utk.edu/papi/custom/index.html?lid=62&slid=96

/******************************************************************************************/
#if defined DIVERSALIS__OPERATING_SYSTEM__POSIX
	namespace detail { namespace posix {
		bool clock_gettime_supported, clock_getres_supported, monotonic_clock_supported, cputime_supported;
		::clockid_t monotonic_clock_id, process_cputime_clock_id, thread_cputime_clock_id;

		namespace {
			void error(int const & code = errno) {
				std::cerr << "error: " << code << ": " << ::strerror(code) << std::endl;
			}

			bool supported(int const & option) {
				long int result(::sysconf(option));
				if(result < -1) error();
				return result > 0;
			}
		}

		void config() {
			monotonic_clock_id = process_cputime_clock_id = thread_cputime_clock_id = CLOCK_REALTIME;

			/// TIMERS
			#if !_POSIX_TIMERS && defined DIVERSALIS__COMPILER__FEATURE__WARNING
				#warning will use posix sysconf at runtime to determine whether this OS supports timers: !_POSIX_TIMERS
			#elif _POSIX_TIMERS == -1 && defined DIVERSALIS__COMPILER__FEATURE__WARNING
				#warning this OS does not support posix timers: _POSIX_TIMERS == -1
				clock_gettime_supported = clock_getres_supported = false;
			#elif _POSIX_TIMERS > 0
				clock_gettime_supported = clock_getres_supported = true;
			#endif
			#if !defined _SC_TIMERS && defined DIVERSALIS__COMPILER__FEATURE__WARNING
				#warning cannot use posix sysconf at runtime to determine whether this OS supports timers: !defined _SC_TIMERS
				clock_gettime_supported = clock_getres_supported = false;
			#else
				clock_gettime_supported = clock_getres_supported = supported(_SC_TIMERS);
				// beware: cygwin has clock_gettime, but it doesn't have clock_getres.
				#if defined DIVERSALIS__OPERATING_SYSTEM__CYGWIN && defined DIVERSALIS__COMPILER__FEATURE__WARNING
					#warning \
						Operating system is Cygwin. Cygwin has _POSIX_TIMERS > 0, \
						but only partially implements this posix option: it supports ::clock_gettime, but not ::clock_getres. \
						This might be the reason ::sysconf(_SC_TIMERS) returns 0, but this condradicts _POSIX_TIMERS > 0. \
						We assume that ::clock_gettime is supported.
						if(!clock_gettime_supported) clock_gettime_supported = true;
				#endif
			#endif
			
			// MONOTONIC_CLOCK
			#if !_POSIX_MONOTONIC_CLOCK && defined DIVERSALIS__COMPILER__FEATURE__WARNING
				#warning will use posix sysconf at runtime to determine whether this OS supports monotonic clock: !_POSIX_MONOTONIC_CLOCK
			#elif _POSIX_MONOTONIC_CLOCK == -1 && defined DIVERSALIS__COMPILER__GNU
				#warning this OS does not support posix monotonic clock: _POSIX_MONOTONIC_CLOCK == -1
				monotonic_clock_supported = false;
			#elif _POSIX_MONOTONIC_CLOCK > 0
				monotonic_clock_supported  = true;
			#endif
			#if !defined _SC_MONOTONIC_CLOCK && defined DIVERSALIS__COMPILER__FEATURE__WARNING
				#warning cannot use posix sysconf at runtime to determine whether this OS supports monotonic clock: !defined _SC_MONOTONIC_CLOCK
				monotonic_clock_supported = false;
			#else
				monotonic_clock_supported = supported(_SC_MONOTONIC_CLOCK);
				if(monotonic_clock_supported) monotonic_clock_id = CLOCK_MONOTONIC;
			#endif

			// CPUTIME
			#if !_POSIX_CPUTIME && defined DIVERSALIS__COMPILER__FEATURE__WARNING
				#warning will use posix sysconf at runtime to determine whether this OS supports cpu time: !_POSIX_CPUTIME
			#elif _POSIX_CPUTIME == -1 && defined DIVERSALIS__COMPILER__FEATURE__WARNING
				#warning this OS does not support posix cpu time: _POSIX_CPUTIME == -1
				cputime_supported = false;
			#elif _POSIX_CPUTIME > 0
				cputime_supported = true;
			#endif
			#if !defined _SC_CPUTIME && defined DIVERSALIS__COMPILER__FEATURE__WARNING
				#warning cannot use posix sysconf at runtime to determine whether this OS supports cpu time: !defined _SC_CPUTIME
				cputime_supported = false;
			#else
				cputime_supported = supported(_SC_CPUTIME);
				if(cputime_supported) {
					::clockid_t clock_id;
					if(clock_getcpuclockid(0, &clock_id) == ENOENT) {
						// this SMP system makes CLOCK_PROCESS_CPUTIME_ID and CLOCK_THREAD_CPUTIME_ID inconsistent
						cputime_supported = false;
					} else {
						///\todo do we do something with the clock_id?
						//process_cputime_clock_id = thread_cputime_clock_id = clock_id;
						process_cputime_clock_id = CLOCK_PROCESS_CPUTIME_ID;
						thread_cputime_clock_id = CLOCK_THREAD_CPUTIME_ID;
					}
				}
			#endif
		}
	}}
#endif // defined DIVERSALIS__OPERATING_SYSTEM__POSIX

/******************************************************************************************/
typedef std::nanoseconds (*clock_function) ();
namespace detail {
	/// iso std time.
	/// returns the time since the Epoch (00:00:00 UTC, January 1, 1970), measured in seconds.
	/// test result: clock: std::time, min: 1s, avg: 1s, max: 1s
	std::nanoseconds iso_std_time() throw(std::runtime_error) {
		std::time_t const t(std::time(0));
		if(t < 0) {
			std::ostringstream s; s << exceptions::code_description();
			throw std::runtime_error(s.str().c_str());
		}
		std::nanoseconds ns = std::seconds(t);
		return ns;
	}

	/// iso std clock.
	/// returns an approximation of processor time used by the program
	/// test result on colinux AMD64: clock: std::clock, min: 0.01s, avg: 0.01511s, max: 0.02s
	std::nanoseconds iso_std_clock() throw(std::runtime_error) {
		std::clock_t const t(std::clock());
		if(t < 0) {
			std::ostringstream s; s << exceptions::code_description();
			throw std::runtime_error(s.str().c_str());
		}
		std::nanoseconds ns = std::nanoseconds(1000 * 1000 * 1000LL * t / CLOCKS_PER_SEC);
		return ns;
	}

	#if defined DIVERSALIS__OPERATING_SYSTEM__POSIX
		namespace posix {
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
			/// test result on colinux AMD64: clock: CLOCK_REALTIME, min: 1.3e-05s, avg: 1.6006e-05s, max: 0.001359s
			std::nanoseconds realtime() throw(std::runtime_error) { return get(CLOCK_REALTIME); }

			/// posix CLOCK_MONOTONIC.
			/// Clock that cannot be set and represents monotonic time since some unspecified starting point.
			/// test result on colinux AMD64: clock: CLOCK_MONOTONIC, min: 1.3e-05s, avg: 1.606e-05s, max: 0.001745s
			std::nanoseconds monotonic() throw(std::runtime_error) { return get(CLOCK_MONOTONIC); }

			/// posix CLOCK_PROCESS_CPUTIME_ID.
			/// High-resolution per-process timer from the CPU.
			/// Realized on many platforms using timers from the CPUs (TSC on i386,  AR.ITC on Itanium).
			/// test result on colinux AMD64: clock: CLOCK_PROCESS_CPUTIME_ID, min: 0.01s, avg: 0.01s, max: 0.01s
			std::nanoseconds process_cpu_time() throw(std::runtime_error) { return get(CLOCK_PROCESS_CPUTIME_ID); }

			/// posix CLOCK_THREAD_CPUTIME_ID.
			/// Thread-specific CPU-time clock.
			/// Realized on many platforms using timers from the CPUs (TSC on i386,  AR.ITC on Itanium).
			/// test result on colinux AMD64: clock: CLOCK_THREAD_CPUTIME_ID, min: 0.01s, avg: 0.01s, max: 0.01s
			std::nanoseconds thread_cpu_time() throw(std::runtime_error) { return get(CLOCK_THREAD_CPUTIME_ID); }

			/// posix gettimeofday.
			/// test result on colinux AMD64: clock: gettimeofday, min: 1.3e-05s, avg: 1.5878e-05s, max: 0.001719s
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
		namespace microsoft {
			/// wall clock.
			/// ::QueryPerformanceCounter() is realised using timers from the CPUs (TSC on i386,  AR.ITC on Itanium).
			/// test result on AMD64: clock res: QueryPerformancefrequency: 3579545Hz (3.6MHz)
			/// test result on AMD64: clock: QueryPerformanceCounter, min: 3.073e-006s, avg: 3.524e-006s, max: 0.000375746s
			std::nanoseconds performance_counter() throw(std::runtime_error) {
				::LARGE_INTEGER counter, frequency;
				if(!::QueryPerformanceCounter(&counter) || !::QueryPerformanceFrequency(&frequency)) {
					std::ostringstream s; s << exceptions::code_description();
					throw std::runtime_error(s.str().c_str());
				}
				std::nanoseconds ns(counter.QuadPart * 1000 * 1000 * 1000 / frequency.QuadPart);
				///\todo check possibility of overflow
				//std::nanoseconds ns(counter.QuadPart * (1000 * 1000 * 1000 / frequency.QuadPart));
				return ns;
			}

			/// wall clock.
			/// ::timeGetTime() is equivalent to ::GetTickCount() but Microsoft very loosely tries to express the idea that
			/// it might be more precise, especially if calling ::timeBeginPeriod and ::timeEndPeriod.
			/// Possibly realised using the PIT/PIC PC hardware.
			/// test result: clock: mmsystem timeGetTime, min: 0.001s, avg: 0.0010413s, max: 0.084s.
			std::nanoseconds mme_system_time() {
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

			/// wall clock.
			/// ::GetTickCount() returns the uptime (i.e., time elapsed since computer was booted), in milliseconds.
			/// Microsoft doesn't even specifies wether it's monotonic and as linear as possible, but we can probably assume so.
			/// This function returns a value which is read from a context value which is updated only on context switches,
			/// and hence is very inaccurate: it can lag behind the real clock value as much as 15ms, and sometimes more.
			/// Possibly realised using the PIT/PIC PC hardware.
			/// test result: clock: GetTickCount, min: 0.015s, avg: 0.015719s, max: 0.063s.
			std::nanoseconds tick_count() {
				std::nanoseconds ns(1000LL * 1000 * ::GetTickCount());
				return ns;
			}

			/// wall clock.
			/// test result: clock: GetSystemTimeAsFileTime, min: 0.015625s, avg: 0.0161875s, max: 0.09375s.
			std::nanoseconds system_time_as_file_time_since_epoch() {
				union winapi_is_badly_designed {
					::FILETIME file_time;
					::LARGE_INTEGER large_integer;
				} u;
				::GetSystemTimeAsFileTime(&u.file_time);
				u.large_integer.QuadPart -= 116444736000000000LL; // microsoft disregards the unix/posix epoch time, so we need to apply an offset
				std::nanoseconds ns(100 * u.large_integer.QuadPart);
				return ns;
			}

			/// virtual clock. kernel time not included.
			/// test result: clock: GetProcessTimes, min: 0.015625s, avg: 0.015625s, max: 0.015625s.
			std::nanoseconds process_time() {
				union winapi_is_badly_designed {
					::FILETIME file_time;
					::LARGE_INTEGER large_integer;
				} creation, exit, kernel, user;
				::GetProcessTimes(::GetCurrentProcess(), &creation.file_time, &exit.file_time, &kernel.file_time, &user.file_time);
				std::nanoseconds ns(100 * user.large_integer.QuadPart);
				return ns;
			}

			/// virtual clock. kernel time not included.
			/// The implementation of mswindows' ::GetThreadTimes() is completly broken: http://blog.kalmbachnet.de/?postid=28
			/// test result: clock: GetThreadTimes, min: 0.015625s, avg: 0.015625s, max: 0.015625s.
			std::nanoseconds thread_time() {
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
} // namespace detail

/******************************************************************************************/
std::nanoseconds utc_since_epoch::current() {
	#if defined DIVERSALIS__OPERATING_SYSTEM__POSIX
		return detail::posix::realtime();
	#elif defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT
		// On microsoft's platform, only the performance_counter has a high resolution ;
		// the system_time_as_file_time_since_epoch lags 15ms at the minimum, and randomly has even much longer lags.
		// So, we need to compute an offset once between the system_time_as_file_time_since_epoch clock
		// and the performance_counter one, that we can apply to what the performance_counter clock
		// returns in every call to this function.
		std::nanoseconds const static performance_counter_to_utc_since_epoch_offset(
			detail::microsoft::system_time_as_file_time_since_epoch() -
			detail::microsoft::performance_counter()
		);
		std::nanoseconds ns(
			detail::microsoft::performance_counter() +
			performance_counter_to_utc_since_epoch_offset
		);
		return ns;
	#else
		return detail::iso_std_time();
	#endif
}

std::nanoseconds monotonic::current() {
	#if defined DIVERSALIS__OPERATING_SYSTEM__POSIX
		bool static once = false; if(!once) detail::posix::config();
		if(detail::posix::monotonic_clock_supported) return detail::posix::monotonic();
		else return detail::posix::realtime();
	#elif defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT
		return detail::microsoft::performance_counter();
	#else
		return detail::iso_std_time();
	#endif
}

std::nanoseconds process::current() {
	#if defined DIVERSALIS__OPERATING_SYSTEM__POSIX
		bool static once = false; if(!once) detail::posix::config();
		if(detail::posix::cputime_supported) return detail::posix::process_cpu_time();
		else return detail::iso_std_clock();
	#elif defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT
		return detail::microsoft::process_time();
	#else
		return detail::iso_std_clock();
	#endif
}

std::nanoseconds thread::current() {
	#if defined DIVERSALIS__OPERATING_SYSTEM__POSIX
		bool static once = false; if(!once) detail::posix::config();
		if(detail::posix::cputime_supported) return detail::posix::thread_cpu_time();
		else return detail::iso_std_clock();
	#elif defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT
		return detail::microsoft::thread_time();
	#else
		return detail::iso_std_clock();
	#endif
}

}}}
