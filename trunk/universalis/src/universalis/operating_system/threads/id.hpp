// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2004-2007 psycle development team http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>

///\interface universalis::operating_system::threads::id

/**************************** note ****************************
the C++ standardisation commitee is working on a threading api that will provide an equivalent interface:
#include <thread>
std::thread::id std::this_thread::get_id();
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
#define UNIVERSALIS__COMPILER__DYNAMIC_LINK UNIVERSALIS__OPERATING_SYSTEM__THREADS__ID
#include <universalis/compiler/dynamic_link/begin.hpp>
namespace universalis { namespace operating_system { namespace threads { namespace id {

typedef
	#if defined UNIVERSALIS__QUAQUAVERSALIS && defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT
		::DWORD
	#elif defined UNIVERSALIS__QUAQUAVERSALIS && defined DIVERSALIS__OPERATING_SYSTEM__POSIX
		::pthread_t
	#else
		Glib::Thread *
	#endif
	type;

/// id of current thread.
UNIVERSALIS__COMPILER__DYNAMIC_LINK type current();

}}}}
#include <universalis/compiler/dynamic_link/end.hpp>
