// -*- mode:c++; indent-tabs-mode:t -*-
// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2004-2007 psycle development team http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>

#pragma once
#include <universalis/exception.hpp>
#include <boost/thread/xtime.hpp>
#include "utc_time.hpp" // cannot #include <date_time> due to dependency of std::hiresolution_clock on universalis::operating_system::clocks, which itself uses std::this_thread::sleep for unit testing, which itself uses this file
namespace universalis { namespace standard_library { namespace detail {

	/// see the standard header date_time for duration types implementing the Elapsed_Time concept
	template<typename Elapsed_Time>
	boost::xtime make_boost_xtime(Elapsed_Time const & elapsed_time) {
		std::nanoseconds const ns(elapsed_time);
		std::seconds const s(ns.get_count() / (1000 * 1000 * 1000));
		boost::xtime xtime;
		xtime.sec  = static_cast<boost::xtime::xtime_sec_t>(s.get_count());
		xtime.nsec = static_cast<boost::xtime::xtime_nsec_t>((ns - s).get_count());
		return xtime;
	}
	
	boost::xtime inline make_boost_xtime(std::utc_time const & t) {
		boost::xtime xtime(make_boost_xtime(t.nanoseconds_since_epoch()));
		return xtime;
	}
	
	/// see the standard header date_time for duration types implementing the Elapsed_Time concept
	template<typename Elapsed_Time>
	boost::xtime inline boost_xtime_get_and_add(Elapsed_Time const & elapsed_time) {
		// We compute the event date by getting the current date and adding the delta to it.
		boost::xtime xtime;
		// get the current date.
		boost::xtime_clock_types const clock(boost::TIME_UTC);
		if(!boost::xtime_get(&xtime, clock)) throw universalis::exceptions::runtime_error("failed to get current time", UNIVERSALIS__COMPILER__LOCATION__NO_CLASS);
		// convert the elapsed_time to boost::xtime
		boost::xtime xtime2(make_boost_xtime(elapsed_time));
		// add the delta
		xtime.sec  += xtime2.sec;
		xtime.nsec += xtime2.nsec;
		if(xtime.nsec > 1000 * 1000 * 1000) {
			xtime.nsec -= 1000 * 1000 * 1000;
			++xtime.sec;
		}
		return xtime;
	}
}}}
