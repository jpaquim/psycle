// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2007 psycledelics http://psycle.pastnotecut.org ; johan boule <bohan@jabber.org>

///\file \brief condition standard header
/// This file implements the C++ standards proposal at http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2007/n2320.html
#pragma once
#include "utc_time.hpp"
#include <boost/thread/condition.hpp>
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

				#if 0 ///\todo
				bool timed_wait(lock_type & lock, utc_time const & timeout) throw(lock_error) {
					return implementation_.timed_wait(lock.implementation_timed_lock(),
						universalis::standard_library::detail::make_boost_xtime(timeout)
					);
				}

				template<typename Predicate>
				bool timed_wait(lock_type & lock, Predicate predicate, utc_time const & timeout) throw(lock_error) {
					return implementation_.timed_wait(lock.implementation_timed_lock(), predicate,
						universalis::standard_library::detail::make_boost_xtime(timeout)
					);
				}
				#endif
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
		#if 0
				class shared_data {
					public:
						std::mutex m;
						std::condition<std::scoped_lock<std::mutex> > c;
						int i;
						shared_data() : i() {}
				};

				void thread_function(shared_data * data_pointer) {
					shared_data & data(*data_pointer);
					std::scoped_lock<std::mutex> l(data.m);
					data.i = 1; data.c.notify_one();
					do data.c.wait(l); while(data.i != 2);
				}

				BOOST_AUTO_TEST_CASE(std_condition_mutex_test) {
					shared_data data;
					std::thread t(boost::bind(thread_function, &data));
					{
						std::scoped_lock<std::mutex> l(data.m);
						while(data.i != 1) data.c.wait(l);
						data.i = 2; data.c.notify_one();
					}
					t.join();
				}
		#else // test using boost directly
				class shared_data {
					public:
						boost::mutex m;
						boost::condition c;
						int i;
						shared_data() : i() {}
				};

				void thread_function(shared_data * data_pointer) {
					shared_data & data(*data_pointer);
					boost::mutex::scoped_lock l(data.m);
					data.i = 1; data.c.notify_one();
					do data.c.wait(l); while(data.i != 2);
				}

				BOOST_AUTO_TEST_CASE(boost_condition_test) {
					shared_data data;
					boost::thread t(boost::bind(thread_function, &data));
					{
						boost::mutex::scoped_lock l(data.m);
						while(data.i != 1) data.c.wait(l);
						data.i = 2; data.c.notify_one();
					}
					t.join();
				}
		#endif
	}}}}
#endif
