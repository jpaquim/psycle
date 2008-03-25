// -*- mode:c++; indent-tabs-mode:t -*-
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
	#include <vector>
	#include <boost/bind.hpp>
	#include <sstream>
	namespace universalis { namespace standard_library { namespace detail { namespace test {
		using namespace std;

		class condition_test_class {
			public:
				void test() {
					i = 0;
					thread t(boost::bind(&condition_test_class::thread_function, this));
					{ scoped_lock_type l(m);
						while(i != 1) c.wait(l);
						i = 2;
					}
					c.notify_one();
					t.join();
				}
			
			private:
				typedef scoped_lock<mutex> scoped_lock_type;
				mutex m;
				condition<scoped_lock_type> c;
				int i;

			void thread_function() {
				{ scoped_lock_type l(m);
					i = 1;
				}
				c.notify_one();
				{ scoped_lock<mutex> l(m);
					while(i != 2) c.wait(l); 
				}
				#if 0 ///\todo strange bug...
				{ // timeout in the future
					scoped_lock_type l(m);
					utc_time const timeout(hiresolution_clock<utc_time>::universal_time() + seconds(1));
					while(c.timed_wait(l, timeout)) {
						utc_time const now(hiresolution_clock<utc_time>::universal_time() - milliseconds(500));
						//BOOST_MESSAGE(timeout.nanoseconds_since_epoch().get_count());
						//BOOST_MESSAGE(now.nanoseconds_since_epoch().get_count());
						BOOST_CHECK(now < timeout);
						if(now >= timeout) break;
					}
				}
				{ // timeout in the past
					scoped_lock_type l(m);
					utc_time const timeout(hiresolution_clock<utc_time>::universal_time() - days(1));
					while(c.timed_wait(l, timeout)) {
						utc_time const now(hiresolution_clock<utc_time>::universal_time() - milliseconds(500));
						//BOOST_MESSAGE(timeout.nanoseconds_since_epoch().get_count());
						//BOOST_MESSAGE(now.nanoseconds_since_epoch().get_count());
						BOOST_CHECK(now < timeout);
						if(now >= timeout) break;
					}
				}
				#endif
			}
		};

		BOOST_AUTO_TEST_CASE(std_condition_test) {
			condition_test_class test;
			test.test();
		}
		
		class condition_speed_test_class {
			private:
				nanoseconds static cpu_time_clock() {
					#if 0
						return hiresolution_clock<utc_time>::universal_time().nanoseconds_since_epoch();
					#elif 0
						return universalis::operating_system::clocks::thread_cpu_time::current();
					#elif 0
						return universalis::operating_system::clocks::process_cpu_time::current();
					#else
						return universalis::operating_system::clocks::monotonic::current();
					#endif
				}
				
				class tls {
					public:
						tls(int i) : t(), count(), i(i) {}
						nanoseconds t;
						unsigned int count;
						int i;
				};
				
				void thread_function(tls * const tls_pointer) {
					tls & tls(*tls_pointer);
					{ scoped_lock_type l(m);
						--shared_start;
						while(shared_start) c.wait(l);
					}
					c.notify_all();
					while(tls.count < end) {
						nanoseconds const t0(cpu_time_clock());
						for(unsigned int i(0); i < inner_loop; ++i) {
							{ scoped_lock_type l(m);
								if(shared == -1) return;
								shared = tls.i;
							}
							c.notify_all();
							{ scoped_lock_type l(m);
								while(shared == tls.i) c.wait(l);
							}
						}
						std::nanoseconds const t1(cpu_time_clock());
						if(++tls.count > start) tls.t += t1 - t0;
					}
					{ scoped_lock_type l(m);
						shared = -1;
					}
					c.notify_all();
				}
				
				typedef scoped_lock<mutex> scoped_lock_type;
				mutex m;
				condition<scoped_lock_type> c;

				unsigned int inner_loop, start, end;
				
				int shared, shared_start;

			public:
				void test(unsigned int threads) {
					inner_loop = 1000;
					start = 100;
					end = start + 100;
					shared_start = threads;
					shared = 0;

					vector<tls*> tls_(threads);
					for(unsigned int i(0); i < threads; ++i) tls_[i] = new tls(i);
					
					vector<thread*> threads_(threads);
					for(unsigned int i(0); i < threads; ++i)
						threads_[i] = new thread(boost::bind(&condition_speed_test_class::thread_function, this, tls_[i]));
					
					for(unsigned int i(0); i < threads; ++i) threads_[i]->join();
					
					for(unsigned int i(0); i < threads; ++i) {
						delete threads_[i];
						{
							std::ostringstream s;
							s << threads << " threads: " << i << ": " << tls_[i]->t.get_count() * 1e-9 / (tls_[i]->count - start) / inner_loop << 's';
							BOOST_MESSAGE(s.str());
						}
						delete tls_[i];
					}
				}
		};

		BOOST_AUTO_TEST_CASE(std_condition_speed_test) {
			condition_speed_test_class test;
			test.test(2);
			test.test(8);
		}
	}}}}
#endif
