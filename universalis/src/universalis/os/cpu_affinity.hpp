// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2008-2008 members of the psycle project http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>

///\interface universalis::os::cpu_affinity

#ifndef UNIVERSALIS__OS__CPU_AFFINITY__INCLUDED
#define UNIVERSALIS__OS__CPU_AFFINITY__INCLUDED
#pragma once

#include <diversalis/os.hpp>
#if defined DIVERSALIS__OS__POSIX
	//#define _GNU_SOURCE // for sched_getaffinity
	#include <sched.h> // for sched_getaffinity
	#include <pthread.h> // for pthread_getaffinity_np
	#include <cerrno>
#elif defined DIVERSALIS__OS__MICROSOFT
	#include <windows.h>
#else
	#error unsupported operating system
#endif
#include <stdexcept>
#include <universalis/os/exceptions/code_description.hpp>
#if defined BOOST_AUTO_TEST_CASE
	#include <sstream>
#endif

namespace universalis { namespace os { namespace cpu_affinity {

/// returns the number of cpus available for the current process
unsigned int inline cpu_count() throw(std::runtime_error) {
	unsigned int result(0);
	#if defined DIVERSALIS__OS__POSIX
		///\todo also try using os.sysconf('SC_NPROCESSORS_ONLN') // SC_NPROCESSORS_CONF
		#if defined DIVERSALIS__OS__CYGWIN
			return 1; ///\todo sysconf
		#else
			cpu_set_t set;
			CPU_ZERO(&set);
			if(
				#if 0 // not available on all systems
					pthread_getaffinity_np(pthread_self()
				#else
					sched_getaffinity(0 // current process
				#endif
				, sizeof set /* warning: do not use CPU_SETSIZE here */, &set)
			) {
				//throw exception(UNIVERSALIS__COMPILER__LOCATION);
				std::ostringstream s; s << exceptions::code_description();
				throw std::runtime_error(s.str().c_str());
			}
			for(unsigned int i(0); i < CPU_SETSIZE; ++i) if(CPU_ISSET(i, &set)) ++result;
		#endif
	#elif defined DIVERSALIS__OS__MICROSOFT
		///\todo also try using int(os.environ.get('NUMBER_OF_PROCESSORS', 1))

		DWORD process, system;
		if(!GetProcessAffinityMask(GetCurrentProcess(), &process, &system)) {
			//throw exception(UNIVERSALIS__COMPILER__LOCATION);
			std::ostringstream s; s << exceptions::code_description();
			throw std::runtime_error(s.str().c_str());
		}
		for(DWORD mask(1); mask != 0; mask <<= 1) if(process & mask) ++result;
	#else
		#error unsupported operating system
	#endif
	return result;
}

#if defined BOOST_AUTO_TEST_CASE
	BOOST_AUTO_TEST_CASE(cpu_count_test) {
		std::ostringstream s; s << "cpu count: " << cpu_count();
		BOOST_MESSAGE(s.str());
	}
#endif

}}}

#endif
