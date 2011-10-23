// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2004-2011 members of the psycle project http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>

#pragma once
#include <universalis/stdlib/detail/chrono/duration_and_time_point.hpp>
namespace universalis { namespace os { namespace clocks {

namespace detail {
	template<typename Final, bool Is_Monotonic>
	struct basic_clock {
		typedef stdlib::chrono::nanoseconds duration;
		typedef duration::rep rep;
		typedef duration::period period;
		typedef stdlib::chrono::time_point<Final, duration> time_point;
		static const bool is_monotonic = Is_Monotonic;
	};
}

// recommended: http://icl.cs.utk.edu/papi/custom/index.html?lid=62&slid=96

/// a real time clock that counts the UTC time elasped since the unix epoch (1970-01-01T00:00:00UTC).
///
/// This is the UTC time, and hence not monotonic since UTC has leap seconds to readjust with TAI time.
/// http://www.ucolick.org/~sla/leapsecs/timescales.html
struct utc_since_epoch : public detail::basic_clock<utc_since_epoch, false> {
	static UNIVERSALIS__DECL time_point now();
};

/// a clock that counts the real time elapsed since some unspecified origin.
///
/// The implementation reads, if available, the tick count register of some unspecified CPU.
/// On most CPU architectures, the register is updated at a rate based on the frequency of the cycles, but often the count value and the tick events are unrelated,
/// i.e. the value might not be incremented one by one. So the period corresponding to 1 count unit may be even smaller than the period of a CPU cycle, but should probably stay in the same order of magnitude.
/// If the counter is increased by 4,000,000,000 over a second, and is 64-bit long, it is possible to count an uptime period in the order of a century without wrapping.
/// The implementation for x86, doesn't work well at all on some of the CPUs whose frequency varies over time. This will eventually be fixed http://www.x86-secret.com/?option=newsd&nid=845.
/// The implementation for mswindows is unpsecified on SMP systems.
struct monotonic : public detail::basic_clock<monotonic, true> { // note: new name in c++0x is "steady"
	static UNIVERSALIS__DECL time_point now();
};

/// a virtual clock that counts the time spent by the CPU(s) in the current process.
struct process : public detail::basic_clock<process, true> {
	static UNIVERSALIS__DECL time_point now();
};

/// a virtual clock that counts the time spent by the CPU(s) in the current thread.
struct thread : public detail::basic_clock<thread, true> {
	static UNIVERSALIS__DECL time_point now();
};

/// a virtual clock that counts the time spent by the CPU(s) in the current thread
/// or fallback to process or monotonic clock if the resolution is low.
struct hires_thread_or_fallback : public detail::basic_clock<hires_thread_or_fallback, true> {
	static UNIVERSALIS__DECL time_point now();
};

}}}

#if !defined DIVERSALIS__COMPILER__FEATURE__CXX0X || defined DIVERSALIS__STDLIB__CXX0X__BROKEN__THREAD
	// Due to a mutual dependency with <universalis/stdlib/chrono.hpp>, we define the typedefs here.
	namespace universalis { namespace stdlib { namespace chrono {
		typedef os::clocks::utc_since_epoch system_clock;
		typedef os::clocks::monotonic monotonic_clock;
		typedef system_clock high_resolution_clock; // as done in gcc 4.4
	}}}
#endif

/*****************************************************************************/
#ifdef BOOST_AUTO_TEST_CASE
	#include <universalis/stdlib/detail/chrono/measure_clock.hpp>
	namespace universalis { namespace os { namespace clocks { namespace test {
		BOOST_AUTO_TEST_CASE(measure_clocks_os_test) {
			using stdlib::chrono::test::measure_clock_against_sleep;
			using stdlib::chrono::test::measure_clock_resolution;
			measure_clock_against_sleep<utc_since_epoch>();
			measure_clock_against_sleep<monotonic>();
			#if 0 // illogic for these clocks
				measure_clock_against_sleep<process>();
				measure_clock_against_sleep<thread>();
				measure_clock_against_sleep<hires_thread_or_fallback>();
			#endif
			measure_clock_resolution<utc_since_epoch>();
			measure_clock_resolution<monotonic>();
			measure_clock_resolution<process>(10);
			measure_clock_resolution<thread>(10);
			measure_clock_resolution<hires_thread_or_fallback>();
		}
	}}}}
#endif
