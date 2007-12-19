// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2007 psycledelics http://psycle.pastnotecut.org ; johan boule <bohan@jabber.org>

///\file \brief thread standard header
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
