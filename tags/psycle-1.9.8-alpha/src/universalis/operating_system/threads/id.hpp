// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// Copyright (C) 1999-2006 Johan Boule <bohan@jabber.org>
// Copyright (C) 2004-2006 Psycledelics http://psycle.pastnotecut.org

///\interface universalis::operating_system::threads::id
#pragma once
#include <universalis/detail/project.hpp>
#if defined UNIVERSALIS__QUAQUAVERSALIS && defined DIVERSALIS__OPERATING_SYSTEM__POSIX
	#include <pthread.h>
#elif defined UNIVERSALIS__QUAQUAVERSALIS && defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT
	#include <windows.h>
#else
	namespace Glib
	{
		class Thread;
	}
#endif
#define UNIVERSALIS__COMPILER__DYNAMIC_LINK UNIVERSALIS__OPERATING_SYSTEM__THREADS__ID
#include <universalis/compiler/dynamic_link/begin.hpp>
namespace universalis
{
	namespace operating_system
	{
		namespace threads
		{
			namespace id
			{
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
				type UNIVERSALIS__COMPILER__DYNAMIC_LINK current();
			}
		}
	}
}
#include <universalis/compiler/dynamic_link/end.hpp>
