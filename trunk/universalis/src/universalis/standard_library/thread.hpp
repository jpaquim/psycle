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

		/// see the standard header date_time for types implementing the Elapsed_Time concept
		template<typename Elapsed_Time>
		void sleep(Elapsed_Time const & elapsed_time) {
			nanoseconds const ns(elapsed_time);
			universalis::operating_system::threads::sleep(ns.get_count() * 1e9);
		}
	}
}
