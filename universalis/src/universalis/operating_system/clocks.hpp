// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2007 johan boule <bohan@jabber.org>
// copyright 2004-2007 psycledelics http://psycle.pastnotecut.org

///\implementation universalis::operating_system::clocks
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
		#pragma comment(lib,"kernel32") // win64?
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
#if defined BOOST_AUTO_TEST_CASE
	#include <universalis/operating_system/threads/sleep.hpp>
#endif
namespace universalis { namespace operating_system { namespace clocks {

	// recommended: http://icl.cs.utk.edu/papi/custom/index.html?lid=62&slid=96

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
					u. sec += o. sec;
					u.nsec += o.nsec;
					if(u.nsec >= 1e9) { u.nsec -= 1e9; ++u.sec; }
				#else
					u += o;
				#endif
				return *this;
			}

			opaque_time & operator-=(opaque_time const & other) {
				underlying_type & u(*this);
				underlying_type const & o(other);
				#if defined DIVERSALIS__OPERATING_SYSTEM__POSIX
					u.sec -= o.sec;
					if(u.nsec > o.nsec) u.nsec -= o.nsec;
					else { --u.sec; u.nsec += 1e9 - o.nsec; }
				#else
					u -= o;
				#endif
				return *this;
			}

			bool operator<(opaque_time const & other) const {
				underlying_type const & u(*this);
				underlying_type const & o(other);
				#if defined DIVERSALIS__OPERATING_SYSTEM__POSIX
					if(u. sec < o.sec) return true;
					if(u. sec > o.sec) return false;
					return u.nsec < o.nsec;
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
						u = seconds * 1e7; // ::FILETIME resolution
					#else
						u. sec =  seconds;
						u.nsec = (seconds - u.sec) * 1e9;
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
						result = u.sec + u.nsec * 1e-9;
					#endif
					return result;
				}
			///\}
	};

	/// counts the time elapsed since some unspecified origin.
	///
	/// The implementation reads, if available, the tick count register of some unspecified CPU.
	/// On most CPU architectures, the register is updated at a rate based on the frequency of the cycles, but often the count value and the tick events are unrelated,
	/// i.e. the value might not be incremented one by one. So the period corresponding to 1 count unit may be even smaller than the period of a CPU cycle, but should probably stay in the same order of magnitude.
	/// If the counter is increased by 4,000,000,000 over a second, and is 64-bit long, it is possible to count an uptime period in the order of a century without wrapping.
	/// The implementation for x86, doesn't work well at all on some of the CPUs whose frequency varies over time. This will eventually be fixed http://www.x86-secret.com/?option=newsd&nid=845.
	/// The implementation for mswindows is unpsecified on SMP systems.
	class wall {
		public:
			opaque_time static current() {
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
					static bool once(false);
					if(!once) detail::best();
					::clock_gettime(details::wall_time_clock_id, &u);
				#endif
				return result;
			}
	};

	/// counts the time spent by the CPU(s) in the current process, kernel included.
	class process {
		public:
			opaque_time static current() {
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
					static bool once(false);
					if(!once) detail::best();
					::clock_gettime(details::process_time_clock_id, &u);
				#endif
				return result;
			}
	};

	/// counts the time spent by the CPU(s) in the current thread, kernel included.
	class thread {
		public:
			opaque_time static current() {
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
					static bool once(false);
					if(!once) detail::best();
					return ::clock_gettime(details::thread_time_clock_id, &u);
				#endif
				return result;
			}
	};

	#if defined DIVERSALIS__OPERATING_SYSTEM__POSIX

		///\internal
		namespace detail {

			bool supported(int const & option) {
				long int result(::sysconf(option));
				if(result < -1) error();
				return result > 0;
			}

			void error(int const & code = errno) {
				std::cerr << "error: " << code << ": " << ::strerror(code) << std::endl;
			}

			::clockid_t wall_clock_id, process_clock_id, thread_clock_id;

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
						if(clock_getcpuclockid(0) == ENOENT) {
							// this SMP system makes CLOCK_PROCESS_CPUTIME_ID and CLOCK_THREAD_CPUTIME_ID inconsistent
							cpu_time_supported = false;
						} else {
							wall_clock_id = process_clock_id = thread_clock_id = CLOCK_PROCESS_CPUTIME_ID;
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

	#if defined BOOST_AUTO_TEST_CASE
		BOOST_AUTO_TEST_CASE(clock_test)
		{
			typedef thread clock;
			real_time const sleep_seconds(0.25);
			opaque_time const start(clock::current());
			universalis::operating_system::threads::sleep(sleep_seconds);
			double const ratio((clock::current() - start).to_real_time() / sleep_seconds);
			BOOST_CHECK(0.75 < ratio && ratio < 1.25);
		}
	#endif
}}}
