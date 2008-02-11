/* -*- mode:c++, indent-tabs-mode:t -*- */
// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2007 psycledelics http://psycle.pastnotecut.org ; johan boule <bohan@jabber.org>

#pragma once
#include <boost/operators.hpp>
namespace std {

	/*******************************************************************/
	// time duration types
	
	template<typename Final, typename Tick, Tick Ticks_Per_Seconds, Tick Seconds_Per_Tick>
	class basic_time_duration
	: private
		boost::equality_comparable<Final,
		boost::less_than_comparable<Final,
		boost::additive<Final,
		boost::multiplicative<Final, int
		> > > >
	{
		public:
			typedef Tick tick_type;
			basic_time_duration(tick_type ticks = 0) : ticks_(ticks) {}
			tick_type get_count() const { return ticks_; }
			tick_type static ticks_per_second() { return Ticks_Per_Seconds; }
			tick_type static seconds_per_tick() { return Seconds_Per_Tick; }
			bool static is_subsecond() { return ticks_per_second() > seconds_per_tick(); }
			bool operator==(Final const & that) const { return this->ticks_ == that.ticks_; }
			bool operator< (Final const & that) const { return this->ticks_ <  that.ticks_; }
			Final & operator+=(Final const & that) { this->ticks_ += that.ticks_; return static_cast<Final&>(*this); }
			Final & operator-=(Final const & that) { this->ticks_ -= that.ticks_; return static_cast<Final&>(*this); }
			Final operator-() const { Final f(-this->ticks_); return f; }
			Final & operator/=(int d) { ticks_ /= d; return static_cast<Final&>(*this); }
			Final & operator*=(int m) { ticks_ *= m; return static_cast<Final&>(*this); }
		protected:
			typedef basic_time_duration<Final, Tick, Ticks_Per_Seconds, Seconds_Per_Tick> basic_time_duration_type;
		private:
			tick_type ticks_;
	};

	class nanoseconds : public basic_time_duration<nanoseconds, long long int, 1000 * 1000 * 1000, 0> {
		public:
			nanoseconds(tick_type ns = 0) : basic_time_duration_type(ns) {}
	};

	class microseconds : public basic_time_duration<microseconds, long long int, 1000 * 1000, 0> {
		public:
			microseconds(tick_type us = 0) : basic_time_duration_type(us) {}
			operator nanoseconds() const { nanoseconds ns(get_count() * (nanoseconds::ticks_per_second() / ticks_per_second())); return ns; }
	};

	class milliseconds : public basic_time_duration<milliseconds, long long int, 1000, 0> {
		public:
			milliseconds(tick_type ms = 0) : basic_time_duration_type(ms) {}
			operator nanoseconds() const { nanoseconds ns(get_count() * (nanoseconds::ticks_per_second() / ticks_per_second())); return ns; }
			operator microseconds() const { microseconds us(get_count() * (microseconds::ticks_per_second() / ticks_per_second())); return us; }
	};

	class seconds : public basic_time_duration<seconds, long long int, 1, 1> {
		public:
			seconds(tick_type s = 0) : basic_time_duration_type(s) {}
			operator nanoseconds() const { nanoseconds ns(get_count() * nanoseconds::ticks_per_second()); return ns; }
			operator microseconds() const { microseconds us(get_count() * microseconds::ticks_per_second()); return us; }
			operator milliseconds() const { milliseconds ms(get_count() * milliseconds::ticks_per_second()); return ms; }
	};

	class minutes : public basic_time_duration<minutes, long long int, 0, 60> {
		public:
			minutes(tick_type m = 0) : basic_time_duration_type(m) {}
			operator nanoseconds() const { nanoseconds ns(get_count() * nanoseconds::ticks_per_second() * seconds_per_tick()); return ns; }
			operator microseconds() const { microseconds us(get_count() * microseconds::ticks_per_second() * seconds_per_tick()); return us; }
			operator milliseconds() const { milliseconds ms(get_count() * milliseconds::ticks_per_second() * seconds_per_tick()); return ms; }
			operator seconds() const { seconds s(get_count() * seconds_per_tick()); return s; }
	};

	class hours : public basic_time_duration<hours, long long int, 0, 60 * 60> {
		public:
			hours(tick_type h = 0) : basic_time_duration_type(h) {}
			operator nanoseconds() const { nanoseconds ns(get_count() * nanoseconds::ticks_per_second() * seconds_per_tick()); return ns; }
			operator microseconds() const { microseconds us(get_count() * microseconds::ticks_per_second() * seconds_per_tick()); return us; }
			operator milliseconds() const { milliseconds ms(get_count() * milliseconds::ticks_per_second() * seconds_per_tick()); return ms; }
			operator seconds() const { seconds s(get_count() * seconds_per_tick()); return s; }
			operator minutes() const { minutes m(get_count() * (seconds_per_tick() / minutes::seconds_per_tick())); return m; }
	};
	
	class days : public basic_time_duration<days, long long int, 0, 60 * 60 * 24> {
		public:
			days(tick_type d = 0) : basic_time_duration_type(d) {}
			operator nanoseconds() const { nanoseconds ns(get_count() * nanoseconds::ticks_per_second() * seconds_per_tick()); return ns; }
			operator microseconds() const { microseconds us(get_count() * microseconds::ticks_per_second() * seconds_per_tick()); return us; }
			operator milliseconds() const { milliseconds ms(get_count() * milliseconds::ticks_per_second() * seconds_per_tick()); return ms; }
			operator seconds() const { seconds s(get_count() * seconds_per_tick()); return s; }
			operator minutes() const { minutes m(get_count() * (seconds_per_tick() / minutes::seconds_per_tick())); return m; }
			operator hours() const { hours h(get_count() * (seconds_per_tick() / hours::seconds_per_tick())); return h; }
	};

	//class months; // difficult to implement and not useful for specifying pause durations
	//class years; // difficult to implement and not useful for specifying pause durations
}

/******************************************************************************************/
#if defined BOOST_AUTO_TEST_CASE
	namespace universalis { namespace standard_library { namespace detail { namespace test {
		BOOST_AUTO_TEST_CASE(std_date_time_duration_test) {
			using namespace std;
			{
				days const d(1);
				BOOST_CHECK(d.get_count() == 1);

				hours const h(d);
				BOOST_CHECK(h == d);
				BOOST_CHECK(h.get_count() == 24);

				minutes const m(h);
				BOOST_CHECK(m == d);
				BOOST_CHECK(m == h);
				BOOST_CHECK(m.get_count() == 24 * 60);

				seconds const s(m);
				BOOST_CHECK(s == d);
				BOOST_CHECK(s == h);
				BOOST_CHECK(s == m);
				BOOST_CHECK(s.get_count() == 24 * 60 * 60);

				milliseconds const ms(s);
				BOOST_CHECK(ms == d);
				BOOST_CHECK(ms == h);
				BOOST_CHECK(ms == m);
				BOOST_CHECK(ms == s);
				BOOST_CHECK(ms.get_count() == 24 * 60 * 60 * 1000);

				microseconds const us(ms);
				BOOST_CHECK(us == d);
				BOOST_CHECK(us == h);
				BOOST_CHECK(us == m);
				BOOST_CHECK(us == s);
				BOOST_CHECK(us == ms);
				BOOST_CHECK(us.get_count() == 24 * 60 * 60 * 1000 * 1000LL);

				nanoseconds const ns(us);
				BOOST_CHECK(ns == d);
				BOOST_CHECK(ns == h);
				BOOST_CHECK(ns == m);
				BOOST_CHECK(ns == s);
				BOOST_CHECK(ns == ms);
				BOOST_CHECK(ns == us);
				BOOST_CHECK(ns.get_count() == 24 * 60 * 60 * 1000 * 1000LL * 1000);
			}
			{
				hours const h(1);
				minutes m(1);
				m += h;
				BOOST_CHECK(m == minutes(1) + hours(1));
				BOOST_CHECK((m - h).get_count() == 1);
				BOOST_CHECK(m.get_count() == 61);
			}
			{
				seconds const s1(1), s2(2);
				BOOST_CHECK(s1 <  s2);
				BOOST_CHECK(s1 <= s2);
				BOOST_CHECK(s1 <= s1);
				BOOST_CHECK(s1 == s1);
				BOOST_CHECK(s1 != s2);
				BOOST_CHECK(s1 >= s1);
				BOOST_CHECK(s2 >= s1);
				BOOST_CHECK(s2 >  s1);
				BOOST_CHECK(s1 + s1 == s2);
				BOOST_CHECK(s1 * 2 == s2);
			}

		}
	}}}}
#endif
