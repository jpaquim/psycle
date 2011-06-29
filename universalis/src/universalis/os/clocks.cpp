// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2004-2011 members of the psycle project http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>

#include <universalis/detail/project.private.hpp>
#include "clocks.hpp"
#include "detail/clocks.hpp"
#include "exception.hpp"
#include <universalis/compiler/thread_local_storage.hpp>
#if defined DIVERSALIS__OS__POSIX
	#include <unistd.h> // for sysconf
	#include <sys/time.h>
#endif
#include <ctime>
#include <iostream>
#include <sstream>
namespace universalis { namespace os { namespace clocks {

using namespace stdlib::chrono;

namespace detail {
	#if defined DIVERSALIS__OS__POSIX
		namespace posix {
			bool clock_gettime_supported, clock_getres_supported, monotonic_clock_supported, process_cpu_time_supported, thread_cpu_time_supported;
			::clockid_t monotonic_clock_id, process_cpu_time_clock_id, thread_cpu_time_clock_id;

			namespace {
				void error(int const code = errno) {
					std::cerr << "error: " << code << ": " << ::strerror(code) << std::endl;
				}

				bool supported(int const option) {
					long int result(::sysconf(option));
					if(result < -1) error();
					return result > 0;
				}
			}

			void config() {
				bool static once = false;
				if(once) return;

				monotonic_clock_id = process_cpu_time_clock_id = thread_cpu_time_clock_id = CLOCK_REALTIME;

				#if defined DIVERSALIS__COMPILER__DOXYGEN
					/// define this macro to diagnose potential issues
					#define UNIVERSALIS__OS__CLOCKS__DIAGNOSE
				#endif

				/// TIMERS
				#if !_POSIX_TIMERS
					#if defined UNIVERSALIS__OS__CLOCKS__DIAGNOSE && defined DIVERSALIS__COMPILER__FEATURE__WARNING
						#warning will use posix sysconf at runtime to determine whether this OS supports timers: !_POSIX_TIMERS
					#endif
				#elif _POSIX_TIMERS == -1
					#if defined UNIVERSALIS__OS__CLOCKS__DIAGNOSE && defined DIVERSALIS__COMPILER__FEATURE__WARNING
						#warning this OS does not support posix timers: _POSIX_TIMERS == -1
					#endif
					clock_gettime_supported = clock_getres_supported = false;
				#elif _POSIX_TIMERS > 0
					clock_gettime_supported = clock_getres_supported = true;
				#endif
				#if !defined _SC_TIMERS
					#if defined UNIVERSALIS__OS__CLOCKS__DIAGNOSE && defined DIVERSALIS__COMPILER__FEATURE__WARNING
						#warning cannot use posix sysconf at runtime to determine whether this OS supports timers: !defined _SC_TIMERS
					#endif
					clock_gettime_supported = clock_getres_supported = false;
				#else
					clock_gettime_supported = clock_getres_supported = supported(_SC_TIMERS);
					// beware: cygwin has clock_gettime, but it doesn't have clock_getres.
					#if defined UNIVERSALIS__OS__CLOCKS__DIAGNOSE && defined DIVERSALIS__OS__CYGWIN && defined DIVERSALIS__COMPILER__FEATURE__WARNING
						#warning \
							Operating system is Cygwin. Cygwin has _POSIX_TIMERS > 0, \
							but only partially implements this posix option: it supports ::clock_gettime, but not ::clock_getres. \
							This might be the reason ::sysconf(_SC_TIMERS) returns 0, but this condradicts _POSIX_TIMERS > 0. \
							We assume that ::clock_gettime is supported.
							if(!clock_gettime_supported) clock_gettime_supported = true;
					#endif
				#endif

				// MONOTONIC_CLOCK
				#if !_POSIX_MONOTONIC_CLOCK
					#if defined UNIVERSALIS__OS__CLOCKS__DIAGNOSE && defined DIVERSALIS__COMPILER__FEATURE__WARNING
						#warning will use posix sysconf at runtime to determine whether this OS supports monotonic clock: !_POSIX_MONOTONIC_CLOCK
					#endif
				#elif _POSIX_MONOTONIC_CLOCK == -1
					#if defined UNIVERSALIS__OS__CLOCKS__DIAGNOSE && defined DIVERSALIS__COMPILER__FEATURE__WARNING
						#warning this OS does not support posix monotonic clock: _POSIX_MONOTONIC_CLOCK == -1
					#endif
					monotonic_clock_supported = false;
				#elif _POSIX_MONOTONIC_CLOCK > 0
					monotonic_clock_supported = true;
				#endif
				#if !defined _SC_MONOTONIC_CLOCK
					#if defined UNIVERSALIS__OS__CLOCKS__DIAGNOSE && defined DIVERSALIS__COMPILER__FEATURE__WARNING
						#warning cannot use posix sysconf at runtime to determine whether this OS supports monotonic clock: !defined _SC_MONOTONIC_CLOCK
					#endif
					monotonic_clock_supported = false;
				#else
					monotonic_clock_supported = supported(_SC_MONOTONIC_CLOCK);
					if(monotonic_clock_supported) monotonic_clock_id = CLOCK_MONOTONIC;
				#endif

				// PROCESS_CPUTIME
				#if !_POSIX_CPUTIME
					#if defined UNIVERSALIS__OS__CLOCKS__DIAGNOSE && defined DIVERSALIS__COMPILER__FEATURE__WARNING
						#warning will use posix sysconf at runtime to determine whether this OS supports process cpu time: !_POSIX_CPUTIME
					#endif
				#elif _POSIX_CPUTIME == -1
					#if defined UNIVERSALIS__OS__CLOCKS__DIAGNOSE && defined DIVERSALIS__COMPILER__FEATURE__WARNING
						#warning this OS does not support posix process cpu time: _POSIX_CPUTIME == -1
					#endif
					process_cputime_supported = false;
				#elif _POSIX_CPUTIME > 0
					process_cpu_time_supported = true;
				#endif
				#if !defined _SC_CPUTIME
					#if defined UNIVERSALIS__OS__CLOCKS__DIAGNOSE && defined DIVERSALIS__COMPILER__FEATURE__WARNING
						#warning cannot use posix sysconf at runtime to determine whether this OS supports process cpu time: !defined _SC_CPUTIME
					#endif
					process_cpu_time_supported = false;
				#else
					process_cpu_time_supported = supported(_SC_CPUTIME);
					if(process_cpu_time_supported) process_cpu_time_clock_id = CLOCK_PROCESS_CPUTIME_ID;
				#endif

				// THREAD_CPUTIME
				#if !_POSIX_THREAD_CPUTIME
					#if defined UNIVERSALIS__OS__CLOCKS__DIAGNOSE && defined DIVERSALIS__COMPILER__FEATURE__WARNING
						#warning will use posix sysconf at runtime to determine whether this OS supports thread cpu time: !_POSIX_THREAD_CPUTIME
					#endif
				#elif _POSIX_THREAD_CPUTIME == -1
					#if defined UNIVERSALIS__OS__CLOCKS__DIAGNOSE && defined DIVERSALIS__COMPILER__FEATURE__WARNING
						#warning this OS does not support posix thread cpu time: _POSIX_THREAD_CPUTIME == -1
					#endif
					thread_cpu_time_supported = false;
				#elif _POSIX_THREAD_CPUTIME > 0
					thread_cpu_time_supported = true;
				#endif
				#if !defined _SC_THREAD_CPUTIME
					#if defined UNIVERSALIS__OS__CLOCKS__DIAGNOSE && defined DIVERSALIS__COMPILER__FEATURE__WARNING
						#warning cannot use posix sysconf at runtime to determine whether this OS supports thread cpu time: !defined _SC_THREAD_CPUTIME
					#endif
					thread_cpu_time_supported = false;
				#else
					thread_cpu_time_supported = supported(_SC_THREAD_CPUTIME);
					if(thread_cpu_time_supported) thread_cpu_time_clock_id = CLOCK_THREAD_CPUTIME_ID;
				#endif

				// SMP
				#if _POSIX_CPUTIME > 0 || _SC_CPUTIME
					if(process_cpu_time_supported || thread_cpu_time_supported) {
						::clockid_t clock_id;
						if(clock_getcpuclockid(0, &clock_id) == ENOENT) {
							#if defined UNIVERSALIS__OS__CLOCKS__DIAGNOSE && defined DIVERSALIS__COMPILER__FEATURE__WARNING
								#warning this SMP system makes CLOCK_PROCESS_CPUTIME_ID and CLOCK_THREAD_CPUTIME_ID inconsistent
							#endif
							process_cpu_time_supported = thread_cpu_time_supported = false;
						}
					}
				#endif

				once = true;
			}
		}
	#endif // defined DIVERSALIS__OS__POSIX

	/******************************************************************************************/

	/// iso std time.
	/// returns the time since the Epoch (00:00:00 UTC, January 1, 1970), measured in seconds.
	/// test result: clock: std::time, min: 1s, avg: 1s, max: 1s
	iso_std_time::time_point iso_std_time::now() {
		std::time_t const t(std::time(0));
		if(t < 0) throw exceptions::iso_or_posix_std(UNIVERSALIS__COMPILER__LOCATION__NO_CLASS);
		return time_point(seconds(t));
	}

	/// iso std clock.
	/// returns an approximation of processor time used by the program
	/// test result on colinux AMD64: clock: std::clock, min: 0.01s, avg: 0.01511s, max: 0.02s
	iso_std_clock::time_point iso_std_clock::now() {
		std::clock_t const t(std::clock());
		if(t < 0) throw exceptions::iso_or_posix_std(UNIVERSALIS__COMPILER__LOCATION__NO_CLASS);
		return time_point(nanoseconds(1000 * 1000 * 1000LL * t / CLOCKS_PER_SEC));
	}

	#if defined DIVERSALIS__OS__POSIX
		namespace posix {
			namespace {
				nanoseconds get(::clockid_t clock) {
					#if defined _POSIX_TIMERS > 0 || defined _SC_TIMERS
						::timespec t;
						if(::clock_gettime(clock, &t)) throw exception(UNIVERSALIS__COMPILER__LOCATION__NO_CLASS);
						nanoseconds ns(nanoseconds(t.tv_nsec) + seconds(t.tv_sec));
						return ns;
					#else
						return iso_std_clock().time_since_epoch(); // note: this is the process cpu time!
					#endif
				}
			}

			/// posix CLOCK_REALTIME.
			/// System-wide realtime clock.
			/// test result on colinux AMD64: clock: CLOCK_REALTIME, min: 1.3e-05s, avg: 1.6006e-05s, max: 0.001359s
			real_time::time_point real_time::now() { return time_point(get(CLOCK_REALTIME)); }

			/// posix CLOCK_MONOTONIC.
			/// Clock that cannot be set and represents monotonic time since some unspecified starting point.
			/// test result on colinux AMD64: clock: CLOCK_MONOTONIC, min: 1.3e-05s, avg: 1.606e-05s, max: 0.001745s
			monotonic::time_point monotonic::now() {
				return time_point(get(
					#if _POSIX_MONOTONIC_CLOCK > 0 || defined _SC_MONOTONIC_CLOCK
						CLOCK_MONOTONIC
					#else
						CLOCK_REALTIME
					#endif
				));
			}

			/// posix CLOCK_PROCESS_CPUTIME_ID.
			/// High-resolution per-process timer from the CPU.
			/// Realized on many platforms using timers from the CPUs (TSC on i386, AR.ITC on Itanium).
			/// test result on colinux AMD64: clock: CLOCK_PROCESS_CPUTIME_ID, min: 0.01s, avg: 0.01s, max: 0.01s
			process_cpu_time::time_point process_cpu_time::now() {
				return time_point(get(
					#if _POSIX_CPUTIME > 0 || defined _SC_CPUTIME
						CLOCK_PROCESS_CPUTIME_ID
					#else
						CLOCK_REALTIME
					#endif
				));
			}

			/// posix CLOCK_THREAD_CPUTIME_ID.
			/// Thread-specific CPU-time clock.
			/// Realized on many platforms using timers from the CPUs (TSC on i386, AR.ITC on Itanium).
			/// test result on colinux AMD64: clock: CLOCK_THREAD_CPUTIME_ID, min: 0.01s, avg: 0.01s, max: 0.01s
			thread_cpu_time::time_point thread_cpu_time::now() {
				return time_point(get(
					#if _POSIX_THREAD_CPUTIME > 0 || defined _SC_THREAD_CPUTIME
						CLOCK_THREAD_CPUTIME_ID
					#else
						CLOCK_REALTIME
					#endif
				));
			}

			/// posix gettimeofday.
			/// test result on colinux AMD64: clock: gettimeofday, min: 1.3e-05s, avg: 1.5878e-05s, max: 0.001719s
			time_of_day::time_point time_of_day::now() {
				::timeval t;
				if(::gettimeofday(&t, 0)) // second argument passed is 0 for no timezone
					throw exception(UNIVERSALIS__COMPILER__LOCATION__NO_CLASS);
				nanoseconds ns(microseconds(t.tv_usec) + seconds(t.tv_sec));
				return time_point(ns);
			}
		}
	#elif defined DIVERSALIS__OS__MICROSOFT
		namespace microsoft {
			/// wall clock.
			/// ::QueryPerformanceCounter() is realised using timers from the CPUs (TSC on i386, AR.ITC on Itanium).
			/// test result on AMD64: clock resolution: QueryPerformancefrequency: 3579545Hz (3.6MHz)
			/// test result on AMD64: clock: QueryPerformanceCounter, min: 3.073e-006s, avg: 3.524e-006s, max: 0.000375746s
			performance_counter::time_point performance_counter::now() {
				// http://en.wikipedia.org/wiki/Time_Stamp_Counter
				// http://msdn.microsoft.com/en-us/library/ee417693.aspx
				// http://stackoverflow.com/questions/644510/cpu-clock-frequency-and-thus-queryperformancecounter-wrong

				static UNIVERSALIS__COMPILER__THREAD_LOCAL_STORAGE nanoseconds::rep last_time = 0;
				static UNIVERSALIS__COMPILER__THREAD_LOCAL_STORAGE nanoseconds::rep last_frequency_time = 0;
				static UNIVERSALIS__COMPILER__THREAD_LOCAL_STORAGE ::LONGLONG last_frequency(0);
				static UNIVERSALIS__COMPILER__THREAD_LOCAL_STORAGE ::LONGLONG last_counter(0);

				if(!last_frequency_time || nanoseconds(last_time - last_frequency_time) > seconds(1)) {
					last_frequency_time = last_time;
					::LARGE_INTEGER frequency;
					if(!::QueryPerformanceFrequency(&frequency)) throw exception(UNIVERSALIS__COMPILER__LOCATION__NO_CLASS);
					last_frequency = frequency.QuadPart;
				}
				
				::LARGE_INTEGER counter;
				if(!::QueryPerformanceCounter(&counter)) throw exception(UNIVERSALIS__COMPILER__LOCATION__NO_CLASS);
				
				last_time += nanoseconds::ticks_per_second() * (counter.QuadPart - last_counter) / last_frequency;
				last_counter = counter.QuadPart;
				return time_point(nanoseconds(last_time));
			}

			/// wall clock.
			/// ::timeGetTime() is equivalent to ::GetTickCount() but Microsoft very loosely tries to express the idea that
			/// it might be more precise, especially if calling ::timeBeginPeriod and ::timeEndPeriod.
			/// Possibly realised using the PIT/PIC PC hardware.
			/// test result: clock: mmsystem timeGetTime, min: 0.001s, avg: 0.0010413s, max: 0.084s.
			mme_system_time::time_point mme_system_time::now() {
				class set_timer_resolution {
					private: ::UINT milliseconds;
					public:
						set_timer_resolution() {
							// tries to get the best possible resolution, starting with 1ms (which is the minimum supported anyway)
							milliseconds = 1;
							retry:
							if(::timeBeginPeriod(milliseconds) == TIMERR_NOCANDO /* looks like microsoft invented LOLCode! */) {
								if(++milliseconds < 50) goto retry;
								else milliseconds = 0; // give up :-(
							}
						}
						~set_timer_resolution() throw() {
							if(!milliseconds) return; // was not set
							if(::timeEndPeriod(milliseconds) == TIMERR_NOCANDO /* looks like microsoft invented LOLCode! */)
								return; // cannot throw in a destructor
						}
				} static once;
				nanoseconds ns(1000LL * 1000 * ::timeGetTime());
				return time_point(ns);
			}

			/// wall clock.
			/// ::GetTickCount() returns the uptime (i.e., time elapsed since computer was booted), in milliseconds.
			/// Microsoft doesn't even specifies wether it's monotonic and as linear as possible, but we can probably assume so.
			/// This function returns a value which is read from a context value which is updated only on context switches,
			/// and hence is very inaccurate: it can lag behind the real clock value as much as 15ms, and sometimes more.
			/// Possibly realised using the PIT/PIC PC hardware.
			/// test result: clock: GetTickCount, min: 0.015s, avg: 0.015719s, max: 0.063s.
			tick_count::time_point tick_count::now() {
				nanoseconds ns(1000LL * 1000 * ::GetTickCount());
				return time_point(ns);
			}

			/// wall clock.
			/// test result: clock: GetSystemTimeAsFileTime, min: 0.015625s, avg: 0.0161875s, max: 0.09375s.
			system_time_as_file_time_since_epoch::time_point system_time_as_file_time_since_epoch::now() {
				union winapi_is_badly_designed {
					::FILETIME file_time;
					::LARGE_INTEGER large_integer;
				} u;
				::GetSystemTimeAsFileTime(&u.file_time);
				u.large_integer.QuadPart -= 116444736000000000LL; // microsoft disregards the unix/posix epoch time, so we need to apply an offset
				nanoseconds ns(100 * u.large_integer.QuadPart);
				return time_point(ns);
			}

			/// virtual clock. kernel time not included.
			/// test result: clock: GetProcessTimes, min: 0.015625s, avg: 0.015625s, max: 0.015625s.
			process_time::time_point process_time::now() {
				union winapi_is_badly_designed {
					::FILETIME file_time;
					::LARGE_INTEGER large_integer;
				} creation, exit, kernel, user;
				::GetProcessTimes(::GetCurrentProcess(), &creation.file_time, &exit.file_time, &kernel.file_time, &user.file_time);
				nanoseconds ns(100 * user.large_integer.QuadPart);
				return time_point(ns);
			}

			/// virtual clock. kernel time not included.
			/// The implementation of mswindows' ::GetThreadTimes() is completly broken: http://blog.kalmbachnet.de/?postid=28
			/// test result: clock: GetThreadTimes, min: 0.015625s, avg: 0.015625s, max: 0.015625s.
			thread_time::time_point thread_time::now() {
				union winapi_is_badly_designed {
					::FILETIME file_time;
					::LARGE_INTEGER large_integer;
				} creation, exit, kernel, user;
				::GetThreadTimes(::GetCurrentThread(), &creation.file_time, &exit.file_time, &kernel.file_time, &user.file_time);
				nanoseconds ns(100 * user.large_integer.QuadPart);
				return time_point(ns);
			}
		}
	#endif // defined DIVERSALIS__OS__MICROSOFT
} // namespace detail

/******************************************************************************************/

utc_since_epoch::time_point utc_since_epoch::now() {
	#if defined DIVERSALIS__OS__POSIX
		return time_point(detail::posix::real_time::now().time_since_epoch());
	#elif defined DIVERSALIS__OS__MICROSOFT
		// On microsoft's platform, only the performance_counter has a high resolution ;
		// the system_time_as_file_time_since_epoch lags 15ms at the minimum, and randomly has even much longer lags.
		// So, we need to compute an offset once between the system_time_as_file_time_since_epoch clock
		// and the performance_counter one, that we can apply to what the performance_counter clock
		// returns in every call to this function.
		nanoseconds const static performance_counter_to_utc_since_epoch_offset =
			detail::microsoft::system_time_as_file_time_since_epoch::now().time_since_epoch() -
			detail::microsoft::performance_counter::now().time_since_epoch();
		detail::microsoft::performance_counter::time_point const t =
			detail::microsoft::performance_counter::now() + performance_counter_to_utc_since_epoch_offset;
		return time_point(t.time_since_epoch());
	#else
		return time_point(detail::iso_std_time::now().time_since_epoch());
	#endif
}

monotonic::time_point monotonic::now() {
	#if defined DIVERSALIS__OS__POSIX
		detail::posix::config();
		if(detail::posix::monotonic_clock_supported) return time_point(detail::posix::monotonic::now().time_since_epoch());
		else return time_point(detail::posix::real_time::now().time_since_epoch());
	#elif defined DIVERSALIS__OS__MICROSOFT
		return time_point(detail::microsoft::performance_counter::now().time_since_epoch());
	#else
		return time_point(detail::iso_std_time::now().time_since_epoch());
	#endif
}

process::time_point process::now() {
	#if defined DIVERSALIS__OS__POSIX
		detail::posix::config();
		if(detail::posix::process_cpu_time_supported) return time_point(detail::posix::process_cpu_time::now().time_since_epoch());
		else return time_point(monotonic::now().time_since_epoch());
	#elif defined DIVERSALIS__OS__MICROSOFT
		return time_point(detail::microsoft::process_time::now().time_since_epoch());
	#else
		return time_point(detail::iso_std_clock::now().time_since_epoch());
	#endif
}

thread::time_point thread::now() {
	#if defined DIVERSALIS__OS__POSIX
		detail::posix::config();
		if(detail::posix::thread_cpu_time_supported) return time_point(detail::posix::thread_cpu_time::now().time_since_epoch());
		else return time_point(process::now().time_since_epoch());
	#elif defined DIVERSALIS__OS__MICROSOFT
		return time_point(detail::microsoft::thread_time::now().time_since_epoch());
	#else
		return time_point(detail::iso_std_clock::now().time_since_epoch());
	#endif
}

hires_thread_or_fallback::time_point hires_thread_or_fallback::now() {
	#if defined DIVERSALIS__OS__POSIX
		detail::posix::config();
		if(detail::posix::thread_cpu_time_supported) return time_point(detail::posix::thread_cpu_time::now().time_since_epoch());
		else return time_point(process::now().time_since_epoch());
	#elif defined DIVERSALIS__OS__MICROSOFT
		// The implementation of mswindows' ::GetThreadTimes() is completly broken: http://blog.kalmbachnet.de/?postid=28
		// It's also a very low resolution: min: 0.015625s, avg: 0.015625s, max: 0.015625s.
		// So we use the performance counter instead.
		return time_point(detail::microsoft::performance_counter::now().time_since_epoch());
	#else
		return time_point(detail::iso_std_clock::now().time_since_epoch());
	#endif
}

}}}
