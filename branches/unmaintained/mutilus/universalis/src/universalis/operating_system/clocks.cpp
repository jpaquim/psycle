// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2007 johan boule <bohan@jabber.org>
// copyright 2004-2007 psycledelics http://psycle.pastnotecut.org

///\implementation universalis::operating_system::clocks
#include <packageneric/pre-compiled.private.hpp>
#include <packageneric/module.private.hpp>
#include <universalis/detail/project.private.hpp>
#include "clocks.hpp"
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

				#if !defined _POSIX_TIMERS && defined DIVERSALIS__COMPILER__GNU
					#warning !defined _POSIX_TIMERS
				#elif _POSIX_TIMERS == -1 && defined DIVERSALIS__COMPILER__GNU
					#warning _POSIX_TIMERS == -1
					#define UNIVERSALIS__OPERATING_SYSTEM__CLOCKS__DETAIL 0
				#elif _POSIX_TIMERS == 0 && defined DIVERSALIS__COMPILER__GNU
					#warning defined _POSIX_TIMERS && _POSIX_TIMERS == 0
				#elif _POSIX_TIMERS > 0
					// beware: cygwin defines this because it has clock_gettime, but it doesn't have clock_getres.
					#if defined DIVERSALIS__OPERATING_SYSTEM__CYGWIN && defined DIVERSALIS__COMPILER__GNU
						#warning _POSIX_TIMERS > 0, but this is cygwin. \
							Cygwin only partially implements this posix option: it has ::clock_gettime, but not ::clock_getres. \
							Moreover, ::sysconf(_SC_TIMERS) would return 0, which condradicts _POSIX_TIMERS > 0. \
							We will consider that it simply implements nothing.
					#else
						#define UNIVERSALIS__OPERATING_SYSTEM__CLOCKS__DETAIL 1
					#endif
				#endif
				#if !defined _SC_TIMERS && defined DIVERSALIS__COMPILER__GNU
					#warning !defined _SC_TIMERS
					#if !defined UNIVERSALIS__OPERATING_SYSTEM__CLOCKS__DETAIL
						#define UNIVERSALIS__OPERATING_SYSTEM__CLOCKS__DETAIL 0
						timers_supported = false;
					#endif
				#else
					timers_supported = supported(_SC_TIMERS);
				#endif
				
				// CPUTIME
				#if !defined _POSIX_CPUTIME && defined DIVERSALIS__COMPILER__GNU
					#warning !defined _POSIX_CPUTIME
				#elif _POSIX_CPUTIME == -1 && defined DIVERSALIS__COMPILER__GNU
					#warning _POSIX_CPUTIME == -1
					#undef  UNIVERSALIS__OPERATING_SYSTEM__CLOCKS__DETAIL
					#define UNIVERSALIS__OPERATING_SYSTEM__CLOCKS__DETAIL 0
				#elif _POSIX_CPUTIME == 0 && defined DIVERSALIS__COMPILER__GNU
					#warning defined _POSIX_CPUTIME && _POSIX_CPUTIME == 0
				#elif _POSIX_CPUTIME > 0
					#undef  UNIVERSALIS__OPERATING_SYSTEM__CLOCKS__DETAIL
					#define UNIVERSALIS__OPERATING_SYSTEM__CLOCKS__DETAIL 1
				#endif
				#if !defined _SC_CPUTIME && defined DIVERSALIS__COMPILER__GNU
					#warning !defined _SC_CPUTIME
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
				#if !defined _POSIX_MONOTONIC_CLOCK && defined DIVERSALIS__COMPILER__GNU
					#warning !defined _POSIX_MONOTONIC_CLOCK
				#elif _POSIX_MONOTONIC_CLOCK == -1 && defined DIVERSALIS__COMPILER__GNU
					#warning _POSIX_MONOTONIC_CLOCK == -1
					#undef  UNIVERSALIS__OPERATING_SYSTEM__CLOCKS__DETAIL
					#define UNIVERSALIS__OPERATING_SYSTEM__CLOCKS__DETAIL 0
				#elif _POSIX_MONOTONIC_CLOCK == 0 && defined DIVERSALIS__COMPILER__GNU
					#warning defined _POSIX_MONOTONIC_CLOCK && _POSIX_MONOTONIC_CLOCK == 0
				#elif _POSIX_MONOTONIC_CLOCK > 0
					#undef  UNIVERSALIS__OPERATING_SYSTEM__CLOCKS__DETAIL
					#define UNIVERSALIS__OPERATING_SYSTEM__CLOCKS__DETAIL 1
				#endif
				#if !defined _SC_MONOTONIC_CLOCK && defined DIVERSALIS__COMPILER__GNU
					#warning !defined _SC_MONOTONIC_CLOCK
					#if !defined UNIVERSALIS__OPERATING_SYSTEM__CLOCKS__DETAIL
						#define UNIVERSALIS__OPERATING_SYSTEM__CLOCKS__DETAIL 0
					#endif
				#else
				{
					bool const result(supported(_SC_MONOTONIC_CLOCK));
					if(result) {
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
					std::cout << "using ::clock_getres ...\n ";
					if(::clock_getres(clock, &result))
					{
						universalis::operating_system::error();
						return 1;
					}
				#else
					std::cout << "using CLOCKS_PER_SEC ...\n";
					result.tv_sec = 1 / CLOCKS_PER_SEC;
					result.tv_nsec = static_cast<long int>(1e9 / CLOCKS_PER_SEC);
				#endif
				std:: cout << "clock resolution: " << result.tv_sec << "s + " << result.tv_nsec * 1e-9 << "s" << std::endl;
				return 0;
			}
		#endif
	#endif // defined DIVERSALIS__OPERATING_SYSTEM__POSIX

	opaque_time wall::current() {
		opaque_time result;
		opaque_time::underlying_type & u(result);
		#if defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT
			::LARGE_INTEGER counter, frequency;
			if(::QueryPerformanceCounter(&counter) && ::QueryPerformanceFrequency(&frequency)) {
				u = counter.QuadPart * 1e7 / frequency.QuadPart; // to ::FILETIME
			} else { // The CPU has no tick count register.
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
				u = ::timeGetTime() * 1e4; // milliseconds to ::FILETIME
					// ::timeGetTime() equivalent to ::GetTickCount() but Microsoft very loosely tries to express the idea that it might be more precise, especially if calling ::timeBeginPeriod and ::timeEndPeriod.
					//
					// ::GetTickCount()
					// uptime (i.e., time elapsed since computer was booted), in milliseconds. Microsoft doesn't even specifies wether it's monotonic and as linear as possible, but we can probably assume so.
					// This function returns a value which is read from a context value which is updated only on context switches, and hence is very inaccurate: it can lag behind the real clock value as much as 15ms.
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
				::GetThreadTimes(&creation, &exit, &kernel, &user);
				union winapi_is_badly_designed // real unit 1e-7 seconds
				{
					::FILETIME file_time;
					::LARGE_INTEGER large_integer;
				} u1, u2;
				u1.file_time = user;
				u2.file_time = kernel;
				u = u1.large_integer.QuadPart + u2.large_integer.QuadPart;
			#else // Use the wall clock instead, which majorates all virtual subclocks.
				u = wall::current().underlying();
			#endif
		#else
			static bool once(false); if(!once) best();
			::clock_gettime(thread_clock_id, &u);
		#endif
		return result;
	}
}}}
