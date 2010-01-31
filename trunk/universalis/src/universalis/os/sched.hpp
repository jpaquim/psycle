// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2008-2009 members of the psycle project http://psycle.sourceforge.net

///\interface universalis::os::sched

#ifndef UNIVERSALIS__OS__SCHED__INCLUDED
#define UNIVERSALIS__OS__SCHED__INCLUDED
#pragma once

#include <diversalis/os.hpp>
#if defined DIVERSALIS__OS__POSIX
	#include <pthread.h>
#elif defined DIVERSALIS__OS__MICROSOFT
	#include <windows.h>
#else
	#error unsupported operating system
#endif
#include <boost/version.hpp>
#if BOOST_VERSION >= 103500
	#include <boost/thread/thread.hpp>
#endif
#if defined BOOST_AUTO_TEST_CASE
	#include <sstream>
#endif
#include <stdexcept>

namespace universalis { namespace os { namespace sched {

namespace thread {

	typedef 
		#if BOOST_VERSION >= 103500
			boost::thread::native_handle_type
		#elif defined DIVERSALIS__OS__POSIX
			::pthread_t 
		#elif defined DIVERSALIS__OS__MICROSOFT
			HANDLE
		#else
			#error unsupported operating system
		#endif
		native_handle_type;
	
	native_handle_type inline native_handle() {
		return
			#if defined DIVERSALIS__OS__POSIX
				::pthread_self();
			#elif defined DIVERSALIS__OS__MICROSOFT
				::GetCurrentThread();
			#else
				#error unsupported operating system
			#endif
	}

	namespace priority {

		int get(native_handle_type native_handle);
		int inline get() { return get(native_handle()); }

		void set(native_handle_type native_handle, int priority);
		void inline set(int priority) { set(native_handle(), priority); }

		#if defined DIVERSALIS__OS__MICROSOFT
			int const idle         = THREAD_PRIORITY_IDLE;
			int const lowest       = THREAD_PRIORITY_LOWEST;
			int const low          = THREAD_PRIORITY_BELOW_NORMAL;
			int const normal       = THREAD_PRIORITY_NORMAL;
			int const high         = THREAD_PRIORITY_ABOVE_NORMAL;
			int const highest      = THREAD_PRIORITY_HIGHEST;
			int const realtime     = THREAD_PRIORITY_TIME_CRITICAL;
			// Note: If the thread (actually, the process) has the REALTIME_PRIORITY_CLASS base class,
			//       then this can also be -7, -6, -5, -4, -3, 3, 4, 5, or 6.
		#else
			// Note: These are not the actual native values but an arbitrary scale.
			//       The native values depends on the scheduling policy,
			//       and the sched_get_priority_min(policy) and sched_get_priority_max(policy) functions
			//       are used to rescale to native values.
			//       The policy is chosen this way:
			//           - value >  normal: if min == max, the policy is set to SCHED_RR,
			//           - value == normal: SCHED_OTHER (the default policy of the os),
			//           - value <  normal: SCHED_BATCH on linux, SCHED_OTHER on other posix systems.
			int const idle         = -300;
			int const lowest       = -200;
			int const low          = -100;
			int const normal       =    0;
			int const high         = +100;
			int const highest      = +200;
			int const realtime     = +300;
		#endif
	}
}

/// returns the number of cpus available to the current process
unsigned int hardware_concurrency() throw(std::runtime_error);

#if defined BOOST_AUTO_TEST_CASE
	BOOST_AUTO_TEST_CASE(hardware_concurrency_test) {
		std::ostringstream s; s << "hardware concurrency: " << hardware_concurrency();
		BOOST_MESSAGE(s.str());
	}
	
	void thread_priority_test_one(int p) {
		using namespace thread;
		int p0 = priority::get(native_handle());
		{
			std::ostringstream s; s << "setting thread priority to: " << p;
			BOOST_MESSAGE(s.str());
		}
		priority::set(native_handle(), p);
		p = priority::get(native_handle());
		{
			std::ostringstream s; s << "thread priority is: " << p;
			BOOST_MESSAGE(s.str());
		}
		// reset to previous
		priority::set(native_handle(), p0);
	}
	
	BOOST_AUTO_TEST_CASE(thread_priority_test) {
		using namespace thread::priority;
		thread_priority_test_one(idle);
		thread_priority_test_one(lowest);
		thread_priority_test_one(low);
		thread_priority_test_one(normal);
		thread_priority_test_one(high);
		thread_priority_test_one(highest);
		thread_priority_test_one(realtime);
	}
#endif

}}}

#endif
