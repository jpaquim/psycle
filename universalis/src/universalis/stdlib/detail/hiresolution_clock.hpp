// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2007 psycledelics http://psycle.pastnotecut.org ; johan boule <bohan@jabber.org>

#ifndef UNIVERSALIS__STDLIB__DETAIL__HIRESOLUTION_CLOCK__INCLUDED
#define UNIVERSALIS__STDLIB__DETAIL__HIRESOLUTION_CLOCK__INCLUDED
#pragma once

#include "utc_time.hpp"
#include <universalis/os/clocks.hpp>

namespace universalis { namespace stdlib {

template<typename Time_Point>
class hiresolution_clock {
	public:
		Time_Point static universal_time() {
			std::nanoseconds const ns(universalis::os::clocks::utc_since_epoch::current());
			Time_Point t(ns);
			return t;
		}
};
template<>
class hiresolution_clock<std::utc_time> {
	public:
		std::utc_time static universal_time() {
			std::nanoseconds const ns(universalis::os::clocks::utc_since_epoch::current());
			std::utc_time t(ns.get_count());
			return t;
		}
};

}}

/****************************************************************************/
// injection in std namespace
namespace std { using namespace universalis::stdlib; }

/******************************************************************************************/
#if defined BOOST_AUTO_TEST_CASE
	#include <universalis/stdlib/thread.hpp>
	#include <sstream>
	namespace universalis { namespace stdlib {
		BOOST_AUTO_TEST_CASE(hiresolution_clock_and_sleep_test) {
			nanoseconds const sleep_nanoseconds(milliseconds(250));
			utc_time const t0(hiresolution_clock<utc_time>::universal_time());
			this_thread::sleep(sleep_nanoseconds);
			double const ratio(
				double((hiresolution_clock<utc_time>::universal_time() - t0).get_count()) /
				sleep_nanoseconds.get_count()
			);
			{
				std::ostringstream s; s << ratio;
				BOOST_MESSAGE(s.str());
			}
			BOOST_CHECK(0.66 < ratio && ratio < 1.33);
		}
	}}
#endif

#endif
