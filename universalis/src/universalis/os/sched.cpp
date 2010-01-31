// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2008-2009 members of the psycle project http://psycle.sourceforge.net

///\implementation universalis::os::sched

#include <universalis/detail/project.private.hpp>
#include "sched.hpp"

#include <diversalis/os.hpp>
#if defined DIVERSALIS__OS__POSIX
	//#define _GNU_SOURCE // sched_getaffinity
	#include <sched.h> // sched_getaffinity
	//#include <pthread.h> // pthread_getaffinity_np
#endif
#include "exceptions/code_description.hpp"
#include "loggers.hpp"

namespace universalis { namespace os { namespace sched {

namespace thread { namespace priority {
	
int get(native_handle_type native_handle) {
	#if defined DIVERSALIS__OS__POSIX
		int policy;
		sched_param param;
		if(int error = pthread_getschedparam(native_handle, &policy, &param)) {
			std::ostringstream s;
			s << "could not get thread policy and priority: " << os::exceptions::code_description(error);
			loggers::warning()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
			return normal;
		}
		int min_native_priority;
		if((min_native_priority = sched_get_priority_min(policy)) == -1) {
			std::ostringstream s;
			s << "could not get min priority for policy: " << policy << ": " << os::exceptions::code_description();
			loggers::warning()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
			return normal;
		}
		int max_native_priority;
		if((max_native_priority = sched_get_priority_max(policy)) == -1) {
			std::ostringstream s;
			s << "could not get max priority for policy: " << policy << ": " << os::exceptions::code_description();
			loggers::warning()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
			return normal;
		}
		if(min_native_priority == max_native_priority) {
			switch(policy) {
				#if defined DIVERSALIS__OS__LINUX
					case SCHED_BATCH: return low;
				#endif
				case SCHED_RR: return high;
				case SCHED_OTHER:
				default: return normal;
			}
		} else return
			#if 0 ///\todo bug un formula
				idle + (realtime - idle) * (param.sched_priority - min_native_priority) / (max_native_priority - min_native_priority);
			#else
				param.sched_priority;
			#endif
	#elif defined DIVERSALIS__OS__MICROSOFT
		int const priority = ::GetThreadPriority(native_handle);
		switch(priority) {
			case THREAD_PRIORITY_ERROR_RETURN: {
				std::ostringstream s;
				s << "could not get thread priority: " << os::exceptions::code_description();
				loggers::warning()(s.str(), UNIVERSALIS__COMPILER__LOCATION__NO_CLASS);
				return normal;
			}
			default: return priority;
		}
	#else
		return normal;
	#endif
}

void set(native_handle_type native_handle, int priority) {
	#if defined DIVERSALIS__OS__POSIX
		int const policy =
			priority > normal ? SCHED_RR :
			#if defined DIVERSALIS__OS__LINUX
				priority < normal ? SCHED_BATCH :
			#endif
			SCHED_OTHER;
		int min_native_priority;
		if((min_native_priority = sched_get_priority_min(policy)) == -1) {
			std::ostringstream s;
			s << "could not get min priority for policy: " << policy << ": " << os::exceptions::code_description();
			loggers::warning()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
			return;
		}
		int max_native_priority;
		if((max_native_priority = sched_get_priority_max(policy)) == -1) {
			std::ostringstream s;
			s << "could not get max priority for policy: " << policy << ": " << os::exceptions::code_description();
			loggers::warning()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
			return;
		}
		sched_param param;
		#if 0
			param.sched_priority = min_native_priority + (max_native_priority - min_native_priority) * priority / (realtime - idle);
		#else
			param.sched_priority =
				priority <  lowest  ? min_native_priority :
				priority <  low     ? std::max(0, min_native_priority) :
				priority <  normal  ? std::max(0, min_native_priority) :
				priority == normal  ? 0 :
				priority <= high    ? std::min(1, max_native_priority) :
				priority <= highest ? std::min(2, max_native_priority) : max_native_priority;
		#endif
		if(int error = pthread_setschedparam(native_handle, policy, &param)) {
			std::ostringstream s;
			s << "could not set thread policy to: " << policy << ", and priority to: " << param.sched_priority << ": " << os::exceptions::code_description(error);
			loggers::warning()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
		}
	#elif defined DIVERSALIS__OS__MICROSOFT
		// We can't choose values between the predefined constants,
		// so we have to "snap" the value to one of those constants.
		int const native_priority =
			priority <  lowest  ? idle    :
			priority <  low     ? lowest  :
			priority <  normal  ? low     :
			priority == normal  ? normal  :
			priority <= high    ? high    :
			priority <= highest ? highest : realtime;
		if(!::SetThreadPriority(native_handle, native_priority)) {
			std::ostringstream s;
			s << "could not set thread priority to: " << native_priority << ": " << os::exceptions::code_description();
			loggers::warning()(s.str(), UNIVERSALIS__COMPILER__LOCATION__NO_CLASS);
		}
	#endif
}

}}

/// returns the number of cpus available to the current process
unsigned int hardware_concurrency() throw(std::runtime_error) {
	#if BOOST_VERSION >= 103500
		return boost::thread::hardware_concurrency();
	#else
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
	#endif
}

}}}

