// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2007 johan boule <bohan@jabber.org>
// copyright 2004-2007 psycledelics http://psycle.pastnotecut.org

///\implementation universalis::operating_system::threads::id
#include <packageneric/pre-compiled.private.hpp>
#include <packageneric/module.private.hpp>
#include <universalis/detail/project.private.hpp>
#include "id.hpp"
#include <cassert>
#if defined UNIVERSALIS__QUAQUAVERSALIS && defined DIVERSALIS__OPERATING_SYSTEM__POSIX
	//
#elif defined UNIVERSALIS__QUAQUAVERSALIS && defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT
	//
#else
	#include <glibmm/thread.h>
	#if defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT
		#if defined DIVERSALIS__COMPILER__FEATURE__AUTO_LINK
			#if defined NDEBUG
				#pragma comment(lib,"glibmm-2.4.lib")
			#else
				#pragma comment(lib,"glibmm-2.4d.lib")
			#endif
			#pragma comment(lib,"glib-2.0.lib")
			#pragma comment(lib,"gthread-2.0.lib")
		#endif
	#endif
#endif
namespace universalis
{
	namespace operating_system
	{
		namespace threads
		{
			namespace id
			{
				type current()
				{
					#if defined UNIVERSALIS__QUAQUAVERSALIS && defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT
						return ::GetCurrentThreadId();
					#elif defined UNIVERSALIS__QUAQUAVERSALIS && defined DIVERSALIS__OPERATING_SYSTEM__POSIX
						return ::pthread_self();
					#else
						class once
						{
							public:
								once()
								{
									if(!Glib::thread_supported()) Glib::thread_init();
								}
						};
						once static once;
						assert(Glib::thread_supported());
						return Glib::Thread::self();
					#endif
				}
			}
		}
	}
}
