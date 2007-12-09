#pragma once
namespace std {
	/*
		The following types are also provided by the boost date-time library.
		There is also a prototype implementation at http://www.crystalclearsoftware.com/libraries/date_time/n2328_impl.tar.gz
		For now, we just define a minimal implementation for the sleep function to work.
		For example:
			std::this_thread::sleep(std::nanoseconds(123456789));
			std::this_thread::sleep(std::microseconds(123456));
			std::this_thread::sleep(std::milliseconds(123));
	*/
	
	class nanoseconds;
	class microseconds;
	class milliseconds;
	class seconds;
	class minutes;
	class hours;
	//class day; // difficult to implement and not useful for specifying pause durations
	//class month; // difficult to implement and not useful for specifying pause durations
	//class year; // difficult to implement and not useful for specifying pause durations
	
	class nanoseconds {
		public:
			typedef long long int tick_type;
			nanoseconds(tick_type ns = 0) : ns_(ns) {}
			inline nanoseconds(microseconds const &);
			inline nanoseconds(milliseconds const &);
			inline nanoseconds(seconds const &);
			inline nanoseconds(minutes const &);
			inline nanoseconds(hours const & h);
			tick_type get_count() const { return ns_; }
		private:
			tick_type ns_;
	};

	class microseconds {
		public:
			typedef long long int tick_type;
			microseconds(tick_type us = 0) : us_(us) {}
			inline microseconds(milliseconds const &);
			inline microseconds(seconds const &);
			inline microseconds(minutes const &);
			inline microseconds(hours const & h);
			tick_type get_count() const { return us_; }
			operator nanoseconds() const { nanoseconds ns(us_ * 1000); return ns; }
		private:
			tick_type us_;
	};

	class milliseconds {
		public:
			typedef long long int tick_type;
			milliseconds(tick_type ms = 0) : ms_(ms) {}
			inline milliseconds(seconds const &);
			inline milliseconds(minutes const &);
			inline milliseconds(hours const & h);
			tick_type get_count() const { return ms_; }
			operator nanoseconds() const { nanoseconds ns(ms_ * 1000000); return ns; }
			operator microseconds() const { microseconds us(ms_ * 1000); return us; }
		private:
			tick_type ms_;
	};

	class seconds {
		public:
			typedef long long int tick_type;
			seconds(tick_type s = 0) : s_(s) {}
			inline seconds(minutes const &);
			inline seconds(hours const & h);
			tick_type get_count() const { return s_; }
			operator nanoseconds() const { nanoseconds ns(s_ * 1000000000); return ns; }
			operator microseconds() const { microseconds us(s_ * 1000000); return us; }
			operator milliseconds() const { milliseconds ms(s_ * 1000); return ms; }
		private:
			tick_type s_;
	};

	class minutes {
		public:
			typedef long long int tick_type;
			minutes(tick_type m = 0) : m_(m) {}
			inline minutes(hours const & h);
			tick_type get_count() const { return m_; }
			operator nanoseconds() const { nanoseconds ns(m_ * 60000000000LL); return ns; }
			operator microseconds() const { microseconds us(m_ * 60000000); return us; }
			operator milliseconds() const { milliseconds ms(m_ * 60000); return ms; }
			operator seconds() const { seconds s(m_ * 60); return s; }
		private:
			tick_type m_;
	};

	class hours {
		public:
			typedef long long int tick_type;
			hours(tick_type h = 0) : h_(h) {}
			tick_type get_count() const { return h_; }
			operator nanoseconds() const { nanoseconds ns(h_ * 3600000000000LL); return ns; }
			operator microseconds() const { microseconds us(h_ * 3600000000LL); return us; }
			operator milliseconds() const { milliseconds ms(h_ * 3600000); return ms; }
			operator seconds() const { seconds s(h_ * 3600); return s; }
			operator minutes() const { minutes m(h_ * 60); return m; }
		private:
			tick_type h_;
	};

	/***********************************************************************/
	// inline implementation
	
	nanoseconds::nanoseconds(microseconds const & us) : ns_(us.get_count() * 1000) {}
	nanoseconds::nanoseconds(milliseconds const & ms) : ns_(ms.get_count() * 1000000) {}
	nanoseconds::nanoseconds(seconds const & s) : ns_(s.get_count() * 1000000000) {}
	nanoseconds::nanoseconds(minutes const & m) : ns_(m.get_count() * 60000000000LL) {}
	nanoseconds::nanoseconds(hours const & h) : ns_(h.get_count() * 3600000000000LL) {}

	microseconds::microseconds(milliseconds const & ms) : us_(ms.get_count() * 1000) {}
	microseconds::microseconds(seconds const & s) : us_(s.get_count() * 1000000) {}
	microseconds::microseconds(minutes const & m) : us_(m.get_count() * 60000000LL) {}
	microseconds::microseconds(hours const & h) : us_(h.get_count() * 3600000000LL) {}
	
	milliseconds::milliseconds(seconds const & s) : ms_(s.get_count() * 1000) {}
	milliseconds::milliseconds(minutes const & m) : ms_(m.get_count() * 60000LL) {}
	milliseconds::milliseconds(hours const & h) : ms_(h.get_count() * 3600000LL) {}

	seconds::seconds(minutes const & m) : s_(m.get_count() * 60LL) {}
	seconds::seconds(hours const & h) : s_(h.get_count() * 3600LL) {}

	minutes::minutes(hours const & h) : m_(h.get_count() * 60LL) {}

	#if defined BOOST_AUTO_TEST_CASE
		BOOST_AUTO_TEST_CASE(std_date_time_test)
		{
			seconds const s(1);
			BOOST_CHECK(s.get_count() == 1);
			
			milliseconds const ms(s);
			BOOST_CHECK(ms.get_count() == 1000);
			
			microseconds const us(ms);
			BOOST_CHECK(us.get_count() == 1000000);
			
			nanoseconds const ns(ms);
			BOOST_CHECK(ns.get_count() == 1000000000LL);
		}
	#endif
}
