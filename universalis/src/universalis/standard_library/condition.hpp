// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2007 psycle development team http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>

///\file \brief condition standard header
/// This file implements the C++ standards proposal at http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2007/n2320.html
#pragma once
#include "detail/boost_xtime.hpp"
#include <boost/thread/condition.hpp>
#include <date_time>
namespace std {

	typedef boost::lock_error lock_error;
	// system_error
	// thread_canceled

	template<typename Lock>
	class condition : private boost::noncopyable {
		public:
			///\name notification
			///\{
				void notify_one() { implementation_.notify_one(); }
				void notify_all() { implementation_.notify_all(); }
			///\}

			typedef Lock lock_type;

			///\name waiting
			///\{
				void wait(lock_type & lock) throw(lock_error) {
					implementation_.wait(lock.implementation_lock());
				}

				template<typename Predicate>
				void wait(lock_type & lock, Predicate predicate) throw(lock_error) {
					implementation_.wait(lock.implementation_lock(), predicate);
				}

				bool timed_wait(lock_type & lock, utc_time const & timeout) throw(lock_error) {
					return implementation_.timed_wait(lock.implementation_lock(),
						universalis::standard_library::detail::make_boost_xtime(timeout)
					);
				}

				template<typename Predicate>
				bool timed_wait(lock_type & lock, Predicate predicate, utc_time const & timeout) throw(lock_error) {
					return implementation_.timed_wait(lock.implementation_lock(), predicate,
						universalis::standard_library::detail::make_boost_xtime(timeout)
					);
				}
			///\}

		private:
			boost::condition implementation_;
	};
}

#include "mutex.hpp"
namespace std {
	typedef condition<mutex> condition_mtx;
	typedef condition<unique_lock<mutex> > condition_ulm;
}

/******************************************************************************************/
#if defined BOOST_AUTO_TEST_CASE
	#include <boost/bind.hpp>
	namespace universalis { namespace standard_library { namespace detail { namespace test {
		using namespace std;
		class shared_data {
			public:
				mutex m;
				condition<scoped_lock<mutex> > c;
				int i;
				shared_data() : i() {}
		};

		void thread_function(shared_data * data_pointer) {
			shared_data & data(*data_pointer);
			scoped_lock<mutex> l(data.m);
			data.i = 1; data.c.notify_one();
			do data.c.wait(l); while(data.i != 2);
			data.i = 3; data.c.notify_one();
			{
				bool const timed_out(!data.c.timed_wait(l, hiresolution_clock<utc_time>::universal_time() - days(1)));
				BOOST_MESSAGE(timed_out);
				BOOST_MESSAGE(data.i);
				// spurious wakeups? BOOST_CHECK(timed_out || data.i == 4);
			}
			{
				bool const timed_out(!data.c.timed_wait(l, hiresolution_clock<utc_time>::universal_time() + seconds(1)));
				BOOST_MESSAGE(timed_out);
				BOOST_MESSAGE(data.i);
				// spurious wakeups? BOOST_CHECK(timed_out || data.i == 4);
			}
		}

		BOOST_AUTO_TEST_CASE(std_condition_mutex_test) {
			shared_data data;
			thread t(boost::bind(thread_function, &data));
			{
				scoped_lock<mutex> l(data.m);
				while(data.i != 1) data.c.wait(l);
				data.i = 2; data.c.notify_one();
				while(data.i != 3) data.c.wait(l);
				data.i = 4; data.c.notify_one();
			}
			t.join();
		}
	}}}}
#endif
