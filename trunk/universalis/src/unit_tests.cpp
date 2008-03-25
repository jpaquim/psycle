// -*- mode:c++; indent-tabs-mode:t -*-
#include <packageneric/pre-compiled.private.hpp>

#define BOOST_LIB_NAME boost_unit_test_framework
#include <boost/version.hpp>
#if !defined _MSC_VER || BOOST_VERSION >= 103400 // looks like for boost version 1.33 there's no dll for the unit test framework on msvc
	#define BOOST_DYN_LINK
#endif
#include <boost/config/auto_link.hpp>

#include <boost/version.hpp>
#if !defined _MSC_VER || BOOST_VERSION >= 103400 // looks like for boost version 1.33 there's no dll for the unit test framework on msvc
	#define BOOST_TEST_DYN_LINK
#endif
#define BOOST_AUTO_TEST_MAIN
#include <boost/test/auto_unit_test.hpp>

#include <date_time>
#include <thread>
#include <mutex>
#include <condition>
#include <universalis/operating_system/cpu_affinity.hpp>

using namespace std;
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
			while(tls.count < end) {
				nanoseconds const t0(cpu_time_clock());
				for(unsigned int i(0); i < inner_loop; ++i) {
					{ scoped_lock_type l(m);
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
				shared = tls.i;
			}
			c.notify_all();
		}
		
		typedef scoped_lock<mutex> scoped_lock_type;
		mutex m;
		condition<scoped_lock_type> c;

		unsigned int inner_loop, start, end;
		
		int shared;

	public:
		void test(unsigned int threads) {
			inner_loop = 1000;
			start = 10;
			end = start + 10;
			shared = -1;

			tls ** const tls_(new tls*[threads]);
			thread ** const threads_(new thread*[threads]);
			for(unsigned int i(0); i < threads; ++i) {
				tls_[i] = new tls(i);
				threads_[i] = new thread(boost::bind(&condition_speed_test_class::thread_function, this, tls_[i]));
			}
			
			for(unsigned int i(0); i < threads; ++i) threads_[i]->join();
			
			for(unsigned int i(0); i < threads; ++i) {
				delete threads_[i];
				{
					std::ostringstream s;
					s << i << ": " << tls_[i]->t.get_count() * 1e-9 / (tls_[i]->count - start) << 's';
					BOOST_MESSAGE(s.str());
				}
				delete tls_[i];
			}
		}
};

BOOST_AUTO_TEST_CASE(std_condition_speed_test) {
	condition_speed_test_class test;
	test.test(2);
	//test.test(8);
	//test.test(50);
}
