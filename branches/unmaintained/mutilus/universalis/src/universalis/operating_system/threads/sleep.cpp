// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2004-2007 johan boule <bohan@jabber.org>
// copyright 2004-2007 psycle development team http://psycle.sourceforge.net

///\implementation universalis::operating_system::threads::sleep
#include <packageneric/pre-compiled.private.hpp>
#include <packageneric/module.private.hpp>
#include <universalis/detail/project.private.hpp>
#include "sleep.hpp"
#if /* boost::thread::sleep returns on interruptions */ /*defined UNIVERSALIS__QUAQUAVERSALIS &&*/ defined DIVERSALIS__OPERATING_SYSTEM__POSIX
	#include <universalis/operating_system/exceptions/code_description.hpp>
	#include <ctime> // posix 1003.1b ::nanosleep, ::timespec
	#include <cerrno>
	#include <sstream>
#elif defined UNIVERSALIS__QUAQUAVERSALIS && defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT
	#include <windows.h>
#else
	#include <boost/thread/thread.hpp>
	#include <boost/thread/xtime.hpp>
	#include <sstream>
#endif
namespace universalis
{
	namespace operating_system
	{
		namespace threads
		{
			void sleep(compiler::numeric<>::floating_point const & seconds) throw(exception)
			{
				#if /* boost::thread::sleep returns on interruptions */ /*defined UNIVERSALIS__QUAQUAVERSALIS &&*/ defined DIVERSALIS__OPERATING_SYSTEM__POSIX
					::timespec requested, remains;
					requested.tv_sec = static_cast< ::time_t >(seconds);
					compiler::numeric<>::floating_point const fractional(seconds - requested.tv_sec);
					requested.tv_nsec = static_cast</*un*/signed long int>(fractional * 1e9);
					while(::nanosleep(&requested, &remains)) ///\todo use ::clock_nanosleep instead
					{
						if(errno != EINTR)
						{
							std::ostringstream s;
							s << "failed to sleep: " << exceptions::code_description();
							throw exceptions::runtime_error(s.str(), UNIVERSALIS__COMPILER__LOCATION);
						}
						// We have been interrupted before the period has completed.
						requested = remains;
					}
				#elif defined UNIVERSALIS__QUAQUAVERSALIS && defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT
					// ::Sleep is only in miliseconds.
					::Sleep(static_cast< ::DWORD >(seconds * 1e3));
				#else
					// boost::thread::sleep sleeps until the given absolute event date.
					// So, we compute the event date by getting the current date and adding the delta to it.
					boost::xtime xtime;
					// get the current date.
					boost::xtime_clock_types const clock(boost::TIME_UTC);
					if(!boost::xtime_get(&xtime, clock)) throw exceptions::runtime_error("failed to get current time", UNIVERSALIS__COMPILER__LOCATION__NO_CLASS);
					boost::xtime::xtime_sec_t const integral(static_cast<boost::xtime::xtime_sec_t>(seconds));
					boost::xtime::xtime_nsec_t const fractional(static_cast<boost::xtime::xtime_nsec_t>((seconds - integral) * 1e9));
					// add the delta
					xtime.sec += integral;
					xtime.nsec += fractional;
					// sleep until absolute date
					boost::thread::sleep(xtime); //\todo returns on interruptions
				#endif
			}
		}
	}
}
