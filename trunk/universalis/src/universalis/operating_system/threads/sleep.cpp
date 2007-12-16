// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2004-2007 psycle development team http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>

///\implementation universalis::operating_system::threads::sleep
#include <packageneric/pre-compiled.private.hpp>
#include <packageneric/module.private.hpp>
#include <universalis/detail/project.private.hpp>
#include "sleep.hpp"
#if /* boost::thread::sleep returns on interruptions */ \
	/*defined UNIVERSALIS__QUAQUAVERSALIS &&*/ \
	defined DIVERSALIS__OPERATING_SYSTEM__POSIX
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
namespace universalis { namespace operating_system { namespace threads {

void sleep(std::nanoseconds const & ns) throw(exception) {
	#if /* boost::thread::sleep returns on interruptions */ /*defined UNIVERSALIS__QUAQUAVERSALIS &&*/ defined DIVERSALIS__OPERATING_SYSTEM__POSIX
		::timespec requested, remains;
		requested.tv_sec = static_cast<std::time_t>(ns.get_count() / (1000 * 1000 * 1000));
		requested.tv_nsec = static_cast</*un*/signed long int>(ns.get_count() - requested.tv_sec);
		while(::nanosleep(&requested, &remains)) { ///\todo use ::clock_nanosleep instead
			if(errno != EINTR) {
				std::ostringstream s;
				s << "failed to sleep: " << exceptions::code_description();
				throw exceptions::runtime_error(s.str(), UNIVERSALIS__COMPILER__LOCATION);
			}
			// We have been interrupted before the period has completed.
			requested = remains;
		}
	#elif defined UNIVERSALIS__QUAQUAVERSALIS && defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT
		// ::Sleep is only in miliseconds.
		::Sleep(static_cast< ::DWORD >(ns.get_count() / (1000 * 1000)));
	#else
		// boost::thread::sleep sleeps until the given absolute event date.
		// So, we compute the event date by getting the current date and adding the delta to it.
		boost::xtime xtime;
		// get the current date.
		boost::xtime_clock_types const clock(boost::TIME_UTC);
		if(!boost::xtime_get(&xtime, clock)) throw exceptions::runtime_error("failed to get current time", UNIVERSALIS__COMPILER__LOCATION__NO_CLASS);
		// convert our std::nanoseconds to boost::xtime
		boost::xtime xtime2;
		xtime2.sec = static_cast<boost::xtime::xtime_sec_t>(ns.get_count() / (1000 * 1000 * 1000));
		xtime2.nsec  = static_cast<boost::xtime::xtime_nsec_t>(ns.get_count() - xtime2.sec);
		// add the delta
		xtime.sec += xtime2.sec;
		xtime.nsec += xtime2.nsec;
		if(xtime.nsec > 1000 * 1000 * 1000) {
			xtime.nsec -= 1000 * 1000 * 1000;
			++xtime.sec;
		}
		// sleep until absolute date
		boost::thread::sleep(xtime); //\todo returns on interruptions
	#endif
}
}}}
