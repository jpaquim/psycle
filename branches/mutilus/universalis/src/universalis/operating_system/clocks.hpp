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
	#include <sstream>
#endif
#define UNIVERSALIS__COMPILER__DYNAMIC_LINK UNIVERSALIS__OPERATING_SYSTEM__CLOCKS
#include <universalis/compiler/dynamic_link/begin.hpp>
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
					u.tv_sec += o.tv_sec;
					u.tv_nsec += o.tv_nsec;
					if(u.tv_nsec >= 1e9) { u.tv_nsec -= 1e9; ++u.tv_sec; }
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
					else { --u.tv_sec; u.tv_nsec += 1e9 - o.tv_nsec; }
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
						u = seconds * 1e7; // ::FILETIME resolution
					#else
						u.tv_sec =  seconds;
						u.tv_nsec = (seconds - u.tv_sec) * 1e9;
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

	/// counts the time elapsed since some unspecified origin.
	///
	/// The implementation reads, if available, the tick count register of some unspecified CPU.
	/// On most CPU architectures, the register is updated at a rate based on the frequency of the cycles, but often the count value and the tick events are unrelated,
	/// i.e. the value might not be incremented one by one. So the period corresponding to 1 count unit may be even smaller than the period of a CPU cycle, but should probably stay in the same order of magnitude.
	/// If the counter is increased by 4,000,000,000 over a second, and is 64-bit long, it is possible to count an uptime period in the order of a century without wrapping.
	/// The implementation for x86, doesn't work well at all on some of the CPUs whose frequency varies over time. This will eventually be fixed http://www.x86-secret.com/?option=newsd&nid=845.
	/// The implementation for mswindows is unpsecified on SMP systems.
	class UNIVERSALIS__COMPILER__DYNAMIC_LINK wall {
		public:
			opaque_time static current();
	};

	/// counts the time spent by the CPU(s) in the current process, kernel included.
	class UNIVERSALIS__COMPILER__DYNAMIC_LINK process {
		public:
			opaque_time static current();
	};

	/// counts the time spent by the CPU(s) in the current thread, kernel included.
	class UNIVERSALIS__COMPILER__DYNAMIC_LINK thread {
		public:
			opaque_time static current();
	};

	#if defined BOOST_AUTO_TEST_CASE
		BOOST_AUTO_TEST_CASE(clock_test)
		{
			typedef thread clock;
			real_time const sleep_seconds(0.25);
			opaque_time const start(clock::current());
			universalis::operating_system::threads::sleep(sleep_seconds);
			double const ratio((clock::current() - start).to_real_time() / sleep_seconds);
			std::ostringstream s;
			s << ratio;
			BOOST_MESSAGE(s.str());
			BOOST_CHECK(0.75 < ratio && ratio < 1.25);
		}
	#endif
}}}
#include <universalis/compiler/dynamic_link/end.hpp>
