// -*- mode:c++; indent-tabs-mode:t -*-
// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2008-2008 psycle development team http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>

///\interface universalis::operating_system::cpu_affinity
#pragma once
#include <diversalis/operating_system.hpp>
#if defined DIVERSALIS__OPERATING_SYSTEM__POSIX
	//#define _GNU_SOURCE // for sched_getaffinity
	#include <sched.h> // for sched_getaffinity
	#include <pthread.h> // for pthread_getaffinity_np
	#include <cerrno>
#elif defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT
	#include <windows.h>
#else
	#error unsupported operating system
#endif
#include <stdexcept>
#include <universalis/operating_system/exceptions/code_description.hpp>
#if defined BOOST_AUTO_TEST_CASE
	#include <sstream>
#endif
#define UNIVERSALIS__COMPILER__DYNAMIC_LINK UNIVERSALIS__OPERATING_SYSTEM__CPU_AFFINITY
#include <universalis/compiler/dynamic_link/begin.hpp>
namespace universalis { namespace operating_system { namespace cpu_affinity {

	/// returns the number of cpus available for the current process
	unsigned int cpu_count() throw(std::runtime_error) {
		unsigned int result(0);
		#if defined DIVERSALIS__OPERATING_SYSTEM__POSIX
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
		#elif defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT
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
#include <universalis/compiler/dynamic_link/end.hpp>
