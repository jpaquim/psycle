// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// Copyright (C) 1999-2005 Psycledelics http://psycle.pastnotecut.org : Johan Boule

///\file
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
				type current();
			}
		}
	}
}
// arch-tag: 7be60033-5668-447d-bef8-8377a327fde6
