// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2007 psycle development team http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>

#pragma once
#include "duration.hpp"
#include <ctime> // for std::time_t in std::utc_time
#include <boost/operators.hpp>
namespace std {

	template<typename Time_Point>
	class hiresolution_clock;

	class utc_time
	: private
		boost::equality_comparable<utc_time,
		boost::less_than_comparable<utc_time
		> >
	{
		public:
			/// storage type for number of nanoseconds since epoch
			typedef nanoseconds::tick_type tick_type;

			/// number of ticks per second
			tick_type static ticks_per_second() { return nanoseconds::ticks_per_second(); }

			/// number of seconds per tick
			tick_type static seconds_per_tick() { return nanoseconds::seconds_per_tick(); }

			/// true if resolution is less than 1 second
			bool static is_subsecond() { return nanoseconds::is_subsecond(); }

			/// zero time (i.e. epoch)
			utc_time() : ticks_() {}

			/// constructs a time point from the given separate seconds and nanoseconds since epoch
			utc_time(time_t s, nanoseconds ns) : ticks_(s * ticks_per_second() + ns.get_count()) {}

			/// number of seconds since epoch
			time_t seconds_since_epoch() const { return ticks_ / ticks_per_second(); }

			/// number of nanoseconds since epoch
			nanoseconds nanoseconds_since_epoch() const { return ticks_; }

			/// returns true if this == that
			bool operator==(utc_time const & that) const { return this->ticks_ == that.ticks_; }

			/// returns true if this < that
			bool operator <(utc_time const & that) const { return this->ticks_ <  that.ticks_; }

			/// returns the duration between this and that
			nanoseconds operator-(utc_time const & that) const { nanoseconds ns(this->ticks_ - that.ticks_); return ns; }

			/// returns this time point shifted by the given duration
			template<typename Duration>
			utc_time operator+(Duration const & d) const { nanoseconds ns(d); utc_time t(ticks_ + ns.get_count()); return t; }

			/// returns this time point shifted by the given duration
			template<typename Duration>
			utc_time operator-(Duration const & d) const { nanoseconds ns(d); utc_time t(ticks_ - ns.get_count()); return t; }

			/// returns shifts this time point by the given duration
			template<typename Duration>
			utc_time operator+=(Duration const & d) { nanoseconds ns(d); this->ticks_ += ns.get_count(); return *this; }

			/// returns shifts this time point by the given duration
			template<typename Duration>
			utc_time operator-=(Duration const & d) { nanoseconds ns(d); this->ticks_ -= ns.get_count(); return *this; }

		private:
			tick_type ticks_;
			utc_time(tick_type ticks) : ticks_(ticks) {} friend class hiresolution_clock<utc_time>;
	};
}

/******************************************************************************************/
#if defined BOOST_AUTO_TEST_CASE
	namespace universalis { namespace standard_library { namespace detail { namespace test {
		BOOST_AUTO_TEST_CASE(std_utc_time_test) {
			using namespace std;
			std::days const d(1);
			utc_time const t0;
			utc_time t1(t0 + d);
			BOOST_CHECK(t1 == t0 + d);
			BOOST_CHECK(t1 - t0 == d);
			t1 -= d;
			BOOST_CHECK(t1 == t0);
		}
	}}}}
#endif
