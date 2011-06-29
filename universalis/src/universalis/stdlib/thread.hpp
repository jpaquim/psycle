// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2011 members of the psycle project http://psycle.pastnotecut.org ; johan boule <bohan@jabber.org>

///\file \brief thread standard header
/// This file implements the C++ standards proposal at http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2007/n2320.html

/// note: std::thread::hardware_concurrency() is not impacted by the process' scheduler affinity mask;
/// you can use universalis::os::sched::process().affinity_mask().active_count() for that.

#pragma once
#include <universalis/detail/project.hpp>
#if defined DIVERSALIS__COMPILER__FEATURE__CXX0X && !defined DIVERSALIS__STDLIB__CXX0X__BROKEN__THREAD
	#include <thread>
	namespace universalis { namespace stdlib {
		using std::thread;
		namespace this_thread = std::this_thread;
	}}
#else
	#include "detail/chrono/duration_and_time_point.hpp"
	#include <boost/thread/thread.hpp>
	namespace universalis { namespace stdlib {
		using boost::thread;
		//namespace this_thread = boost::this_thread;
		namespace this_thread {
			using namespace boost::this_thread;
			
			template<typename Duration>
			void inline sleep_for(Duration const & d) {
				chrono::nanoseconds::rep const ns = chrono::nanoseconds(d).count();
				boost::this_thread::sleep(
					boost::posix_time::
						#ifdef BOOST_DATE_TIME_HAS_NANOSECONDS
							nanoseconds(ns)
						#else
							microseconds(ns / 1000)
						#endif
				);
			}

			template<typename Clock, typename Duration>
			void inline sleep_until(chrono::time_point<Clock, Duration> const & t) { sleep_for(t - Clock::now()); }
		}
	}}
#endif

/******************************************************************************************/
#ifdef BOOST_AUTO_TEST_CASE
#include <sstream>
	namespace universalis { namespace stdlib { namespace test {
		BOOST_AUTO_TEST_CASE(thread_hardware_concurrency_test) {
			unsigned int const count = thread::hardware_concurrency();
			std::ostringstream s; s << "hardware concurrency: " << count;
			BOOST_MESSAGE(s.str());
			BOOST_CHECK(count >= 1);
		}
	}}}
#endif
