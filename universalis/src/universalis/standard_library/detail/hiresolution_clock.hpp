// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2007 psycledelics http://psycle.pastnotecut.org ; johan boule <bohan@jabber.org>

#ifndef UNIVERSALIS__STANDARD_LIBRARY__DETAIL__HIRESOLUTION_CLOCK__INCLUDED
#define UNIVERSALIS__STANDARD_LIBRARY__DETAIL__HIRESOLUTION_CLOCK__INCLUDED
#pragma once

#include "utc_time.hpp"
#include <universalis/operating_system/clocks.hpp>

namespace std {
	template<typename Time_Point>
	class hiresolution_clock {
		public:
			Time_Point static universal_time() {
				std::nanoseconds const ns(universalis::operating_system::clocks::utc_since_epoch::current());
				Time_Point t(ns);
				return t;
			}
	};
	template<>
	class hiresolution_clock<std::utc_time> {
		public:
			std::utc_time static universal_time() {
				std::nanoseconds const ns(universalis::operating_system::clocks::utc_since_epoch::current());
				std::utc_time t(ns.get_count());
				return t;
			}
	};
}

/******************************************************************************************/
#if defined BOOST_AUTO_TEST_CASE
	#include <thread>
	#include <sstream>
	namespace universalis { namespace standard_library { namespace detail { namespace test {
		BOOST_AUTO_TEST_CASE(std_hiresolution_clock_and_sleep_test) {
			std::nanoseconds const sleep_nanoseconds(std::milliseconds(250));
			std::utc_time const t0(std::hiresolution_clock<std::utc_time>::universal_time());
			std::this_thread::sleep(sleep_nanoseconds);
			double const ratio(
				double((std::hiresolution_clock<std::utc_time>::universal_time() - t0).get_count()) /
				sleep_nanoseconds.get_count()
			);
			{
				std::ostringstream s; s << ratio;
				BOOST_MESSAGE(s.str());
			}
			BOOST_CHECK(0.66 < ratio && ratio < 1.33);
		}
	}}}}
#endif

#endif
