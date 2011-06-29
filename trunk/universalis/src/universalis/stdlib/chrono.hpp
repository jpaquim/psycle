// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2011 members of the psycle project http://psycle.pastnotecut.org ; johan boule <bohan@jabber.org>

///\file \brief chrono standard header
/// C++0x standards proposal http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2008/n2615.html

#pragma once
#include "detail/chrono/duration_and_time_point.hpp"
#if defined DIVERSALIS__COMPILER__FEATURE__CXX0X && !defined DIVERSALIS__STDLIB__CXX0X__BROKEN__THREAD
	#include <chrono>
	namespace universalis { namespace stdlib { namespace chrono {
		using std::chrono::system_clock;
		using std::chrono::monotonic_clock;
		using std::chrono::high_resolution_clock;
	}}}
#else
	#include <universalis/os/clocks.hpp>
	namespace universalis { namespace stdlib { namespace chrono {
		typedef os::clocks::utc_since_epoch system_clock;
		typedef os::clocks::monotonic monotonic_clock;
		typedef system_clock high_resolution_clock; // as done in gcc 4.4
	}}}
#endif

/******************************************************************************************/
#ifdef BOOST_AUTO_TEST_CASE
	#include "detail/chrono/measure_clock.hpp"
	#include <typeinfo>
	namespace universalis { namespace stdlib { namespace chrono { namespace test {
		BOOST_AUTO_TEST_CASE(measure_clocks_stdlib_test) {
			measure_clock_against_sleep<system_clock>();
			measure_clock_resolution<system_clock>(100);
			if(typeid(system_clock) != typeid(monotonic_clock)) {
				measure_clock_against_sleep<monotonic_clock>();
				measure_clock_resolution<monotonic_clock>(100);
			}
			if(typeid(system_clock) != typeid(high_resolution_clock)) {
				measure_clock_against_sleep<high_resolution_clock>();
				measure_clock_resolution<high_resolution_clock>(100);
			}
		}

	}}}}
#endif
