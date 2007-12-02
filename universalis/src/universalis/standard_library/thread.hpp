#pragma once
#include <universalis/operating_system/threads/thread.hpp>
namespace std {

	typedef universalis::operating_system::threads::thread thread;

	namespace this_thread {

		thread::id get_id() {
			return universalis::operating_system::threads::id::current();
		}

		#if 0 ///\todo		
			void yield() {
				universalis::operating_system::threads::yield();
			}
		#endif

		template<typename Elapsed_Time>
		void sleep(Elapsed_Time const & elapsed_time) {
			nanoseconds const ns(elapsed_time);
			universalis::operating_system::threads::sleep(ns.get_count() * 1e9);
		}
	}

	/*
		The following types are also provided by the boost date-time library.
		There is also a prototype implementation at http://www.crystalclearsoftware.com/libraries/date_time/n2328_impl.tar.gz
		For now, we just define a minimal implementation for the sleep function to work.
		For example:
			std::this_thread::sleep(std::nanoseconds(123456789));
			std::this_thread::sleep(std::microseconds(123456));
			std::this_thread::sleep(std::milliseconds(123));
	*/
	
	class nanoseconds {
		public:
			typedef long long int tick_type;
			nanoseconds(tick_type ns = 0) : ns_(ns);
			tick_type tick_type get_count() const { return ns_; }
		private:
			tick_type ns_;
	};

	class microseconds {
		public:
			typedef long long int tick_type;
			microseconds(tick_type us = 0) : us_(us);
			tick_type tick_type get_count() const { return us_; }
			operator nanoseconds() const { nanoseconds ns(us_ * 1000); return ns; }
		private:
			tick_type us_;
	};

	class milliseconds {
		public:
			typedef long long int tick_type;
			milliseconds(tick_type ms = 0) : ms_(ms);
			tick_type tick_type get_count() const { return ms_; }
			operator nanoseconds() const { nanoseconds ns(ms_ * 1000000); return ns; }
			operator microseconds() const { microseconds us(ms_ * 1000); return us; }
		private:
			tick_type ms_;
	};

	class seconds {
		public:
			typedef long long int tick_type;
			seconds(tick_type s = 0) : s_(s);
			tick_type tick_type get_count() const { return s_; }
			operator nanoseconds() const { nanoseconds ns(s_ * 1000000000); return ns; }
			operator microseconds() const { microseconds us(s_ * 1000000); return us; }
			operator milliseconds() const { milliseconds ms(s_ * 1000); return ms; }
		private:
			tick_type s_;
	};

	class minutes {
		public:
			typedef long long int tick_type;
			minutes(tick_type m = 0) : m_(m);
			tick_type tick_type get_count() const { return m_; }
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
			hours(tick_type h = 0) : h_(h);
			tick_type tick_type get_count() const { return h_; }
			operator nanoseconds() const { nanoseconds ns(h_ * 3600000000000LL); return ns; }
			operator microseconds() const { microseconds us(h_ * 3600000000LL); return us; }
			operator milliseconds() const { milliseconds ms(h_ * 3600000); return ms; }
			operator seconds() const { seconds s(h_ * 3600); return s; }
			operator minutes() const { minutes m(h_ * 60); return m; }
		private:
			tick_type h_;
	};
}
