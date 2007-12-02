// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2004-2007 psycle development team http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>

///\interface universalis::operating_system::threads::thread

/**************************** note ****************************
the C++ standardisation commitee is working on a threading api that will provide an equivalent interface:
#include <thread>
class std::thread;

see reference:
	Document number: N2320=07-0180
	http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2007/n2320.html
***************************************************************/

#pragma once
#include <universalis/detail/project.hpp>
#if defined UNIVERSALIS__QUAQUAVERSALIS && defined DIVERSALIS__OPERATING_SYSTEM__POSIX
	#include <pthread.h>
#elif defined UNIVERSALIS__QUAQUAVERSALIS && defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT
	#include <windows.h>
#else
	namespace Glib {
		class Thread;
	}
#endif

#include <universalis/operating_system/threads/id.hpp>
#include <universalis/operating_system/threads/sleep.hpp>

#define UNIVERSALIS__COMPILER__DYNAMIC_LINK  UNIVERSALIS__OPERATING_SYSTEM__THREADS__THREAD
#include <universalis/compiler/dynamic_link/begin.hpp>
namespace universalis { namespace operating_system { namespace threads {

class thread {
	public:
		typedef universalis::operating_system::threads::id::type id;

		thread()
		:
			#if defined UNIVERSALIS__QUAQUAVERSALIS && defined DIVERSALIS__OPERATING_SYSTEM__POSIX
				///\todo
			#elif defined UNIVERSALIS__QUAQUAVERSALIS && defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT
				///\todo
			#else
				///\todo
			#endif
		{
		}

		void join() {
			#if defined UNIVERSALIS__QUAQUAVERSALIS && defined DIVERSALIS__OPERATING_SYSTEM__POSIX
				::pthread_join(implementation_);
			#elif defined UNIVERSALIS__QUAQUAVERSALIS && defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT
				///\todo
			#else
				implementation_.join();
			#endif
		}

	private:
		typedef
			#if defined UNIVERSALIS__QUAQUAVERSALIS && defined DIVERSALIS__OPERATING_SYSTEM__POSIX
				::pthread_t
			#elif defined UNIVERSALIS__QUAQUAVERSALIS && defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT
				::HANDLE
			#else
				Glib::Thread
			#endif
			implementation_type;
		implementation_type implementation_;
};

}}}
#include <universalis/compiler/dynamic_link/end.hpp>
