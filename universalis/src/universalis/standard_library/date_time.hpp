#pragma once
#include <ctime> // for std::time_t in std::utc_time

namespace std {
	/*
		The following types are also provided by the boost date-time library.
		There is also a prototype implementation at http://www.crystalclearsoftware.com/libraries/date_time/n2328_impl.tar.gz
		For now, we just define a minimal implementation for the timed_wait and sleep functions to work.
		For example:
			std::unique_lock<std::mutex> lk(mut);
			// Wait for 2 seconds on a condition variable
			std::utc_time time_out = std::hiresolution_clock::universal_time() + std::seconds(2);
			{
				bool timed_out = !cv.timed_wait(lk, time_out);
				if (timed_out)
					// deal with time out
			}
			
			std::this_thread::sleep(std::hiresolution_clock::universal_time() + std::nanoseconds(123456789));
			std::this_thread::sleep(std::hiresolution_clock::universal_time() + std::microseconds(123456));
			std::this_thread::sleep(std::hiresolution_clock::universal_time() + std::milliseconds(123));
	*/

	/*******************************************************************/
	// time duration types
		
	template<typename Final, typename Tick, Tick Ticks_Per_Seconds, Tick Seconds_Per_Tick>
	class basic_time_duration {
		public:
			typedef Tick tick_type;
			basic_time_duration(tick_type tick = 0) : tick_(tick) {}
			tick_type get_count() const { return tick_; }
			tick_type static ticks_per_second() { return Ticks_Per_Seconds; }
			tick_type static seconds_per_tick() { return Seconds_Per_Tick; }
			bool static is_subsecond() { return ticks_per_second() > seconds_per_tick(); }
			bool operator==(Final const & that) const { return this->tick_ == that.tick_; }
			bool operator!=(Final const & that) const { return this->tick_ != that.tick_; }
			bool operator<=(Final const & that) const { return this->tick_ <= that.tick_; }
			bool operator>=(Final const & that) const { return this->tick_ >= that.tick_; }
			bool operator< (Final const & that) const { return this->tick_ < that.tick_; }
			bool operator> (Final const & that) const { return this->tick_ > that.tick_; }
			Final operator-() const { Final f(-this->tick_); return f; }
			Final operator+(Final const & that) const { Final f(this->tick_ + that.tick_); return f; }
			Final operator-(Final const & that) const { Final f(this->tick_ - that.tick_); return f; }
			Final operator/(int d) const { Final f(tick_ / d); return f; }
			Final operator*(int m) const { Final f(tick_ * m); return f; }
			Final & operator+=(Final const & that) { this->tick_ += that.tick_; return static_cast<Final>(*this); }
			Final & operator-=(Final const & that) { this->tick_ -= that.tick_; return static_cast<Final>(*this); }
			Final & operator/=(int d) { tick_ /= d; return static_cast<Final>(*this); }
			Final & operator*=(int m) { tick_ *= m; return static_cast<Final>(*this); }
		protected:
			typedef basic_time_duration<Final, Tick, Ticks_Per_Seconds, Seconds_Per_Tick> basic_time_duration_type;
		private:
			tick_type tick_;
	};

	class nanoseconds : public basic_time_duration<nanoseconds, long long int, 1000000000, 0> {
		public:
			nanoseconds(tick_type ns = 0) : basic_time_duration_type(ns) {}
	};

	class microseconds : public basic_time_duration<microseconds, long long int, 1000000, 0> {
		public:
			microseconds(tick_type us = 0) : basic_time_duration_type(us) {}
			operator nanoseconds() const { nanoseconds ns(get_count() * nanoseconds::ticks_per_second() / ticks_per_second()); return ns; }
	};

	class milliseconds : public basic_time_duration<milliseconds, long long int, 1000, 0> {
		public:
			milliseconds(tick_type ms = 0) : basic_time_duration_type(ms) {}
			operator nanoseconds() const { nanoseconds ns(get_count() * nanoseconds::ticks_per_second() / ticks_per_second()); return ns; }
			operator microseconds() const { microseconds us(get_count() * microseconds::ticks_per_second() / ticks_per_second()); return us; }
	};

	class seconds : public basic_time_duration<seconds, long long int, 1, 1> {
		public:
			seconds(tick_type s = 0) : basic_time_duration_type(s) {}
			operator nanoseconds() const { nanoseconds ns(get_count() * nanoseconds::ticks_per_second() / ticks_per_second()); return ns; }
			operator microseconds() const { microseconds us(get_count() * microseconds::ticks_per_second() / ticks_per_second()); return us; }
			operator milliseconds() const { milliseconds ms(get_count() * milliseconds::ticks_per_second() / ticks_per_second()); return ms; }
	};

	class minutes : public basic_time_duration<minutes, long long int, 0, 60> {
		public:
			minutes(tick_type m = 0) : basic_time_duration_type(m) {}
			operator nanoseconds() const { nanoseconds ns(get_count() * nanoseconds::ticks_per_second() / ticks_per_second()); return ns; }
			operator microseconds() const { microseconds us(get_count() * microseconds::ticks_per_second() / ticks_per_second()); return us; }
			operator milliseconds() const { milliseconds ms(get_count() * milliseconds::ticks_per_second() / ticks_per_second()); return ms; }
			operator seconds() const { seconds s(get_count() * seconds_per_tick()); return s; }
	};

	class hours : public basic_time_duration<hours, long long int, 0, 60 * 60> {
		public:
			hours(tick_type h = 0) : basic_time_duration_type(h) {}
			operator nanoseconds() const { nanoseconds ns(get_count() * nanoseconds::ticks_per_second() / ticks_per_second()); return ns; }
			operator microseconds() const { microseconds us(get_count() * microseconds::ticks_per_second() / ticks_per_second()); return us; }
			operator milliseconds() const { milliseconds ms(get_count() * milliseconds::ticks_per_second() / ticks_per_second()); return ms; }
			operator seconds() const { seconds s(get_count() * seconds_per_tick()); return s; }
			operator minutes() const { minutes m(get_count() * seconds_per_tick() / minutes::seconds_per_tick()); return m; }
	};
	
	class days : public basic_time_duration<days, long long int, 0, 60 * 60 * 24> {
		public:
			days(tick_type d = 0) : basic_time_duration_type(d) {}
			operator nanoseconds() const { nanoseconds ns(get_count() * nanoseconds::ticks_per_second() / ticks_per_second()); return ns; }
			operator microseconds() const { microseconds us(get_count() * microseconds::ticks_per_second() / ticks_per_second()); return us; }
			operator milliseconds() const { milliseconds ms(get_count() * milliseconds::ticks_per_second() / ticks_per_second()); return ms; }
			operator seconds() const { seconds s(get_count() * seconds_per_tick()); return s; }
			operator minutes() const { minutes m(get_count() * seconds_per_tick() / minutes::seconds_per_tick()); return m; }
			operator hours() const { hours h(get_count() * 24); return h; }
	};

	//class months; // difficult to implement and not useful for specifying pause durations
	//class years; // difficult to implement and not useful for specifying pause durations

	/*******************************************************************/
	// time clock
	
	template<typename time_type>
	class hiresolution_clock {
		public:
			static time_type universal_time() {
				time_type t;
				///\ŧodo
				return t;
			}
	};

	/*******************************************************************/
	// time point
	
	class utc_time {
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
			utc_time() : ns_() {}

			#if 0 ///\todo
			/// constructs a time point from the given separate seconds and nanoseconds since epoch
			utc_time(time_t t, nanoseconds ns);
			#endif

			/// number of seconds since epoch
			std::time_t seconds_since_epoch() const { return ns_ / ticks_per_second(); }

			/// number of nanoseconds since epoch
			nanoseconds nanoseconds_since_epoch() const { return ns_; }

			/// returns true if this == that
			bool operator==(utc_time const & that) const { return this->ns_ == that.ns_; }

			/// returns true if this != that
			bool operator!=(utc_time const & that) const { return this->ns_ != that.ns_; }

			/// returns true if this < that
			bool operator<(utc_time const & that) const { return this->ns_ < that.ns_; }

			/// returns true if this > that
			bool operator>(utc_time const & that) const { return this->ns_ > that.ns_; }

			/// returns true if this <= that
			bool operator<=(utc_time const & that) const { return this->ns_ <= that.ns_; }

			/// returns true if this >= that
			bool operator>=(utc_time const & that) const { return this->ns_ >= that.ns_; }

			/// returns duration between this and that
			nanoseconds operator-(utc_time const & that) const { return this->ns_ - that.ns_; }

			/// returns this time point shifted by the given duration
			template<typename Duration>
			utc_time operator+(Duration const & d) const { nanoseconds ns(d); return utc_time(ns_ + d); }

			/// returns this time point shifted by the given duration
			template<typename Duration>
			utc_time operator-(Duration const & d) const { nanoseconds ns(d); return utc_time(ns_ - d); }

			/// returns shifts this time point by the given duration
			template<typename Duration>
			utc_time operator+=(Duration const & d) const { nanoseconds ns(d); this->ns_ += ns.get_count(); return *this; }

			/// returns shifts this time point by the given duration
			template<typename Duration>
			utc_time operator-=(Duration const & d) const { nanoseconds ns(d); this->ns_ -= ns.get_count(); return *this; }

		private:
			tick_type ns_;
			utc_time(tick_type t) : ns_(t) {} friend class hiresolution_clock<tick_type>;
	};
	
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
