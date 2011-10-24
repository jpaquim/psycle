// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2004-2011 members of the psycle project http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>

#pragma once
#include <universalis/stdlib/detail/chrono/duration_and_time_point.hpp>

#if defined DIVERSALIS__COMPILER__FEATURE__CXX0X && !defined DIVERSALIS__STDLIB__CXX0X__BROKEN__THREAD
	#include <chrono>
#endif

namespace universalis { namespace os { namespace clocks {

// recommended reading: http://icl.cs.utk.edu/papi/custom/index.html?lid=62&slid=96

namespace detail {
	template<typename Final, bool Is_Steady>
	struct basic_clock {
		typedef stdlib::chrono::nanoseconds duration;
		typedef duration::rep rep;
		typedef duration::period period;
		typedef stdlib::chrono::time_point<Final, duration> time_point;
		static const bool is_steady = Is_Steady;
	};
}

/// a clock that returns the official, wall time, with UTC timezone, with the origin (zero) being the unix epoch (1970-01-01T00:00:00UTC).
///
/// This clock is NOT monotonic:
/// Even if it's using the UTC timezone and hence not subject to daylight time saving,
/// the clock may still jump forward and backward to take things like leap seconds into account.
/// See http://www.ucolick.org/~sla/leapsecs/timescales.html .
///
/// This clock is NOT steady:
/// It may also temporarily adjust its speed (accelerate or decelerate) to resynchronize with an external reference clock,
/// for example using ntpdate or a hardware device.
///
/// This clock should only be used for absolute timeouts (functions named *_until).
struct utc_since_epoch : public detail::basic_clock<utc_since_epoch, false> {
	static UNIVERSALIS__DECL time_point now();
};

/// a clock that counts the real, physical time elapsed since some unspecified origin, and hence is not only monotonic but also steady.
///
/// For the definition, see the ISO C++2011 standard: http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2010/n3128.html#time.clock.steady
///
/// This clock is suited for relative timeouts (functions named *_for).
#if defined DIVERSALIS__COMPILER__FEATURE__CXX0X && !defined DIVERSALIS__STDLIB__CXX0X__BROKEN__THREAD
	typedef std::chrono::steady_clock steady;
#else
	struct steady : public detail::basic_clock<steady, true> {
		static UNIVERSALIS__DECL time_point now();
	};
#endif

/// a virtual clock that counts the time spent by the CPU(s) in the current process.
struct process : public detail::basic_clock<process, false> {
	static UNIVERSALIS__DECL time_point now();
};

/// a virtual clock that counts the time spent by the CPU(s) in the current thread.
struct thread : public detail::basic_clock<thread, false> {
	static UNIVERSALIS__DECL time_point now();
};

/// a virtual clock that counts the time spent by the CPU(s) in the current thread
/// or fallback to process or steady clock if the resolution is low.
struct hires_thread_or_fallback : public detail::basic_clock<hires_thread_or_fallback, false> {
	static UNIVERSALIS__DECL time_point now();
};

}}}

#if !defined DIVERSALIS__COMPILER__FEATURE__CXX0X || defined DIVERSALIS__STDLIB__CXX0X__BROKEN__THREAD
	// Due to a mutual dependency with <universalis/stdlib/chrono.hpp>, we define the typedefs here.
	namespace universalis { namespace stdlib { namespace chrono {
		typedef os::clocks::utc_since_epoch system_clock;
		typedef os::clocks::steady steady_clock;
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
			measure_clock_against_sleep<steady>();
			#if 0 // illogic for these clocks
				measure_clock_against_sleep<process>();
				measure_clock_against_sleep<thread>();
				measure_clock_against_sleep<hires_thread_or_fallback>();
			#endif
			measure_clock_resolution<utc_since_epoch>();
			measure_clock_resolution<steady>();
			measure_clock_resolution<process>(10);
			measure_clock_resolution<thread>(10);
			measure_clock_resolution<hires_thread_or_fallback>();
		}
	}}}}
#endif
