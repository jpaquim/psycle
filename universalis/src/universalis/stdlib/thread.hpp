// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2008 members of the psycle project http://psycle.pastnotecut.org ; johan boule <bohan@jabber.org>

///\file \brief thread standard header
/// This file implements the C++ standards proposal at http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2007/n2320.html

#ifndef UNIVERSALIS__STDLIB__THREAD__INCLUDED
#define UNIVERSALIS__STDLIB__THREAD__INCLUDED
#pragma once

#include <boost/version.hpp>
#if BOOST_VERSION < 103500
	#include <universalis/os/sched.hpp>
#endif
#if BOOST_VERSION >= 103500 && defined BOOST_DATE_TIME_HAS_NANOSECONDS
	#include <boost/thread/thread_time.hpp> // boost::get_system_time()
	#include <boost/date_time/posix_time/posix_time_duration.hpp>
#else
	#include "detail/boost_xtime.hpp" // boost_xtime_get_and_add
#endif
#include <boost/thread/thread.hpp>
#include <boost/thread/once.hpp>
#include <diversalis/os.hpp>
#if defined DIVERSALIS__OS__MICROSOFT
	#include <windows.h>
#endif
#if defined BOOST_AUTO_TEST_CASE
	#include <sstream>
#endif

namespace universalis { namespace stdlib {

class thread {
	private:
		typedef boost::thread impl_type;
		impl_type impl_;

	public:
		#if BOOST_VERSION >= 103500
			typedef impl_type::native_handle_type native_handle_type;
			native_handle_type native_handle() { return impl_.native_handle(); }
		#else
			typedef void* native_handle_type;
			native_handle_type native_handle() {
				assert(false);
				return 0;
			}
		#endif

		template<typename Callable>
		explicit thread(Callable callable) : impl_(callable) {}
		
		void join() { impl_.join(); }
		
		template<typename Elapsed_Time>
		bool timed_join(Elapsed_Time const & elapsed_time) {
			#if BOOST_VERSION >= 103500
				return impl_.timed_join(elapsed_time);
			#else
				impl_.join();
				return true;
			#endif
		}

		bool joinable() const {
			#if BOOST_VERSION >= 103500
				return impl_.joinable();
			#else
				return true;
			#endif
		}
		
		void detach() {
			#if BOOST_VERSION >= 103500
				impl_.detach();
			#endif
		}

		static unsigned int hardware_concurrency() {
			#if BOOST_VERSION >= 103500
				return impl_type::hardware_concurrency();
			#else
				return os::sched::process().affinity_mask().active_count();
			#endif
		}
};

typedef boost::once_flag once_flag;

/// The standard uses the new "constexpr" keyword to mark once_flag's constructor
/// so that once_flag objects are initialised as compiled-time constants.
/// Since we cannot yet use "constexpr",
/// we have to require explicit initialisation with a constant:
///\code
/// std::once_flag flag = BOOST_ONCE_INIT;
///\endcode
/// instead of just:
///\code
/// std::once_flag flag;
///\endcode
#define STD_ONCE_INIT BOOST_ONCE_INIT

template<typename Callable /*, typename Arguments...*/>
void inline call_once(once_flag & flag, Callable callable /*, Arguments... arguments*/) {
	#if BOOST_VERSION >= 103800
		boost::call_once(flag, callable);
	#else
		// older boost versions want a function pointer and arguments in reversed order
		boost::call_once(callable, flag);
	#endif
}

namespace this_thread {

	void inline yield() { boost::thread::yield(); }

	/// see the standard header date_time for duration types implementing the Elapsed_Time concept
	template<typename Elapsed_Time>
	void inline sleep(Elapsed_Time const & elapsed_time) {
		// boost::thread::sleep sleeps until the given absolute event date.
		// So, we compute the event date by getting the current date and adding the delta to it.
		// Note: boost::thread::sleep returns on interruptions (at least on posix)
		boost::thread::sleep(
			#if BOOST_VERSION >= 103500 && defined BOOST_DATE_TIME_HAS_NANOSECONDS
				boost::get_system_time() +
				boost::posix_time::seconds(elapsed_time.get_count())
			#else
				detail::boost_xtime_get_and_add(elapsed_time)
			#endif
		);
	}
}

#if defined BOOST_AUTO_TEST_CASE
	BOOST_AUTO_TEST_CASE(hardware_concurrency_test) {
		std::ostringstream s; s << "hardware concurrency: " << thread::hardware_concurrency();
		BOOST_MESSAGE(s.str());
	}
#endif

}}

/****************************************************************************/
// injection in std namespace
namespace std { using namespace universalis::stdlib; }

#endif
