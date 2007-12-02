// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2004-2007 psycle development team http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>

///\interface universalis::operating_system::threads::thread

/**************************** note ****************************
the C++ standardisation commitee is working on a threading api that will provide an equivalent interface:
#include <condition>
template<typename Lock> std::condition;

see reference:
	Document number: N2320=07-0180
	http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2007/n2320.html
***************************************************************/

#pragma once
#include "mutex.hpp"
namespace universalis { namespace operating_system { namespace threads {

	template <class Lock> class condition {
		public:
			typedef Lock lock_type;
			
			void notify_one();
			
			void notify_all();
			
			void wait(lock_type & lock);
			
			template<typename Predicate>
			void wait(lock_type & lock, Predicate predicate);
			
			bool timed_wait(lock_type & lock, utc_time const & absolute_time);
			
			template<typename Predicate>
			bool timed_wait(lock_type & lock, utc_time const & absolute_time, Predicate predicate);
			
	}
	typedef condition<mutex>              condition_mtx;
	typedef condition<unique_lock<mutex>> condition_ulm;

}}}
