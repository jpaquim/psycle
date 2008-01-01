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
				/// wakes up one of the threads waiting on the condition.
				/// Note: consider releasing the locks associated with the condition
				/// that the threads are waiting for before notifying.
				void notify_one() { implementation_.notify_one(); }

				/// broadcasts a wake up notification to all the threads waiting on the condition.
				/// Note: consider releasing the locks associated with the condition
				/// that the threads are waiting for before notifying.
				void notify_all() { implementation_.notify_all(); }
			///\}

			typedef Lock lock_type;

			///\name waiting
			///\{
				/// unlocks and sleeps until woken up.
				/// Beware: due to possible "spurious wake ups", this version should always be used within a loop
				/// checking that the predicate state logically associated with the condition has become true.
				/// The templated overload version encapsulates this loop idiom internally
				/// and is generally the preferred method.
				void wait(lock_type & lock) throw(lock_error) {
					implementation_.wait(lock.implementation_lock());
				}

				/// unlocks and sleeps until woken up and predicate has become true.
				/// Effect is as if the following was done:
				///\code
				/// while(!predicate()) wait(lock);
				///\endcode
				template<typename Predicate>
				void wait(lock_type & lock, Predicate predicate) throw(lock_error) {
					implementation_.wait(lock.implementation_lock(), predicate);
				}

				/// unlocks and sleep until woken up or until timeout is reached.
				/// Beware: due to possible "spurious wake ups", this version should always be used within a loop
				/// checking that the predicate state logically associated with the condition has become true.
				/// The templated overload version encapsulates this loop idiom internally
				/// and is generally the preferred method.
				bool timed_wait(lock_type & lock, utc_time const & timeout) throw(lock_error) {
					return implementation_.timed_wait(lock.implementation_lock(),
						universalis::standard_library::detail::make_boost_xtime(timeout)
					);
				}

				/// unlocks and sleep until woken up and predicate has become true or until timeout is reached.
				/// Effect is as if the following was done:
				///\code
				/// while(!predicate()) if(!timed_wait(lock, timeout) return false; return true;
				///\endcode
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
			#if 0 ///\todo strange bug...
			{ // timeout in the future
				utc_time const timeout(hiresolution_clock<utc_time>::universal_time() + seconds(1));
				while(data.c.timed_wait(l, timeout)) {
					utc_time const now(hiresolution_clock<utc_time>::universal_time() - milliseconds(500));
					//BOOST_MESSAGE(timeout.nanoseconds_since_epoch().get_count());
					//BOOST_MESSAGE(now.nanoseconds_since_epoch().get_count());
					BOOST_CHECK(now < timeout);
					if(now >= timeout) break;
				}
			}
			{ // timeout in the past
				utc_time const timeout(hiresolution_clock<utc_time>::universal_time() - days(1));
				while(data.c.timed_wait(l, timeout)) {
					utc_time const now(hiresolution_clock<utc_time>::universal_time() - milliseconds(500));
					//BOOST_MESSAGE(timeout.nanoseconds_since_epoch().get_count());
					//BOOST_MESSAGE(now.nanoseconds_since_epoch().get_count());
					BOOST_CHECK(now < timeout);
					if(now >= timeout) break;
				}
			}
			#endif
		}

		BOOST_AUTO_TEST_CASE(std_condition_mutex_test) {
			shared_data data;
			thread t(boost::bind(thread_function, &data));
			{
				scoped_lock<mutex> l(data.m);
				while(data.i != 1) data.c.wait(l);
				data.i = 2; data.c.notify_one();
			}
			t.join();
		}
	}}}}
#endif
