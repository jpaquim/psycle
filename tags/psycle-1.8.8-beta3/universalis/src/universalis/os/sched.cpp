// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2008-2009 members of the psycle project http://psycle.sourceforge.net

///\implementation universalis::os::sched

#include <universalis/detail/project.private.hpp>
#include "sched.hpp"

#if defined DIVERSALIS__OS__POSIX
	#include <sys/time.h>
	#include <sys/resource.h>
	#include <cerrno>
#endif
#include <cassert>

namespace universalis { namespace os { namespace sched {

/**********************************************************************/
// process

process::process()
: native_handle_(
	#if defined DIVERSALIS__OS__MICROSOFT
		GetCurrentProcess()
	#endif
)
{}

affinity_mask process::affinity_mask() const throw(exception) {
	#if defined DIVERSALIS__OS__POSIX
		///\todo also try using os.sysconf('SC_NPROCESSORS_ONLN') // SC_NPROCESSORS_CONF
		#if defined DIVERSALIS__OS__CYGWIN
			///\todo sysconf
			class affinity_mask result;
			result(0, true);
			return result;
		#else
			class affinity_mask result;
			if(sched_getaffinity(native_handle_, sizeof result.native_mask_, &result.native_mask_) == -1)
				throw exception(UNIVERSALIS__COMPILER__LOCATION);
			return result;
		#endif
	#elif defined DIVERSALIS__OS__MICROSOFT
		class affinity_mask process, system;
		if(!GetProcessAffinityMask(native_handle_, &process.native_mask_, &system.native_mask_))
			throw exception(UNIVERSALIS__COMPILER__LOCATION);
		return process;
	#else
		#error unsupported operating system
	#endif
}

#if defined DIVERSALIS__OS__MICROSOFT
	/// special case for windows, this must be done inline!
	/// The doc says:
	/// "Do not call SetProcessAffinityMask in a DLL that may be called by processes other than your own."
#else
	void process::affinity_mask(class affinity_mask const & affinity_mask) throw(exception) {
		#if defined DIVERSALIS__OS__POSIX
			#if defined DIVERSALIS__OS__CYGWIN
				///\todo sysconf
			#else
				if(sched_setaffinity(native_handle_, sizeof affinity_mask.native_mask_, &affinity_mask.native_mask_) == -1)
					throw exception(UNIVERSALIS__COMPILER__LOCATION);
			#endif
		#elif defined DIVERSALIS__OS__MICROSOFT
			if(!SetProcessAffinityMask(native_handle_, affinity_mask.native_mask_))
				throw exception(UNIVERSALIS__COMPILER__LOCATION);
		#else
			#error unsupported operating system
		#endif
	}
#endif

process::priority_type process::priority() throw(exception) {
	#if defined DIVERSALIS__OS__POSIX
		errno = 0;
		priority_type result(getpriority(PRIO_PROCESS, native_handle_));
		if(result == -1 && errno) throw exception(UNIVERSALIS__COMPILER__LOCATION);
		return result;
	#elif defined DIVERSALIS__OS__MICROSOFT
		priority_type result(GetPriorityClass(native_handle_));
		if(!result) throw exception(UNIVERSALIS__COMPILER__LOCATION);
		return result;
	#else
		#error unsupported operating system
	#endif
}

void process::priority(process::priority_type priority) throw(exception) {
	#if defined DIVERSALIS__OS__POSIX
		if(setpriority(PRIO_PROCESS, native_handle_, priority))
			throw exception(UNIVERSALIS__COMPILER__LOCATION);
	#elif defined DIVERSALIS__OS__MICROSOFT
		if(!SetPriorityClass(native_handle_, priority))
			throw exception(UNIVERSALIS__COMPILER__LOCATION);
	#else
		#error unsupported operating system
	#endif
}

/**********************************************************************/
// thread

thread::thread() : native_handle_(
	#if defined DIVERSALIS__OS__POSIX
		::pthread_self()
	#elif defined DIVERSALIS__OS__MICROSOFT
		::GetCurrentThread()
	#else
		#error unsupported operating system
	#endif
) {}

affinity_mask thread::affinity_mask() const throw(exception) {
	#if defined DIVERSALIS__OS__POSIX
		#if defined DIVERSALIS__OS__CYGWIN
			///\todo sysconf
			class affinity_mask result;
			result[0] = true;
			return result;
		#else
			class affinity_mask result;
			if(int error = pthread_getaffinity_np(native_handle_, sizeof result.native_mask_, &result.native_mask_))
				throw exception(error, UNIVERSALIS__COMPILER__LOCATION);
			return result;
		#endif
	#elif defined DIVERSALIS__OS__MICROSOFT
		// *** Note ***
		// There is no GetThreadAffinityMask function in the winapi!
		// So we have no choice but to return the process' affinity mask.
		process p;
		class affinity_mask result(p.affinity_mask());
		return result;
	#else
		#error unsupported operating system
	#endif
}

void thread::affinity_mask(class affinity_mask const & affinity_mask) throw(exception) {
	#if defined DIVERSALIS__OS__POSIX
		#if defined DIVERSALIS__OS__CYGWIN
			///\todo sysconf
		#else
			if(int error = pthread_setaffinity_np(native_handle_, sizeof affinity_mask.native_mask_, &affinity_mask.native_mask_))
				throw exception(error, UNIVERSALIS__COMPILER__LOCATION);
		#endif
	#elif defined DIVERSALIS__OS__MICROSOFT
		if(!SetThreadAffinityMask(native_handle_, affinity_mask.native_mask_))
			throw exception(UNIVERSALIS__COMPILER__LOCATION);
	#else
		#error unsupported operating system
	#endif
}

#if defined DIVERSALIS__OS__POSIX
namespace {
	// returns the min and max priorities for a policy
	void priority_min_max(int policy, int & min, int & max) {
		if((min = sched_get_priority_min(policy)) == -1)
			throw exception(UNIVERSALIS__COMPILER__LOCATION__NO_CLASS);
		if((max = sched_get_priority_max(policy)) == -1)
			throw exception(UNIVERSALIS__COMPILER__LOCATION__NO_CLASS);
	}
}
#endif

thread::priority_type thread::priority() throw(exception) {
	#if defined DIVERSALIS__OS__POSIX
		int policy;
		sched_param param;
		if(int error = pthread_getschedparam(native_handle_, &policy, &param))
			throw exception(error, UNIVERSALIS__COMPILER__LOCATION);
		int min_native_priority, max_native_priority;
		priority_min_max(policy, min_native_priority, max_native_priority);
		if(min_native_priority == max_native_priority) {
			switch(policy) {
				#if defined SCHED_BATCH
					case SCHED_BATCH: return priorities::low;
				#endif
				case SCHED_RR: return priorities::high;
				case SCHED_OTHER:
				default: return priorities::normal;
			}
		} else return
			#if 0 ///\todo boggus formula
				idle + (realtime - idle) * (param.sched_priority - min_native_priority) / (max_native_priority - min_native_priority);
			#else
				param.sched_priority;
			#endif
	#elif defined DIVERSALIS__OS__MICROSOFT
		int priority;
		if((priority = ::GetThreadPriority(native_handle_)) == THREAD_PRIORITY_ERROR_RETURN)
			throw exception(UNIVERSALIS__COMPILER__LOCATION);
		return priority;
	#else
		return priorities::normal;
	#endif
}

void thread::priority(thread::priority_type priority) throw(exception) {
	#if defined DIVERSALIS__OS__POSIX
		int const policy =
			priority > priorities::normal ? SCHED_RR :
			#if defined SCHED_BATCH
				priority < priorities::normal ? SCHED_BATCH :
			#endif
			SCHED_OTHER;
		int min_native_priority, max_native_priority;
		priority_min_max(policy, min_native_priority, max_native_priority);
		sched_param param;
		#if 0 ///\todo boggus formula
			param.sched_priority = min_native_priority + (max_native_priority - min_native_priority) * priority / (realtime - idle);
		#else
			param.sched_priority =
				priority <  priorities::lowest  ? min_native_priority :
				priority <  priorities::low     ? std::max(0, min_native_priority) :
				priority <  priorities::normal  ? std::max(0, min_native_priority) :
				priority == priorities::normal  ? 0 :
				priority <= priorities::high    ? std::min(1, max_native_priority) :
				priority <= priorities::highest ? std::min(2, max_native_priority) : max_native_priority;
		#endif
		if(int error = pthread_setschedparam(native_handle_, policy, &param))
			switch(error) {
				case EPERM: throw exceptions::operation_not_permitted(UNIVERSALIS__COMPILER__LOCATION);
				default: throw exception(error, UNIVERSALIS__COMPILER__LOCATION);
			}
	#elif defined DIVERSALIS__OS__MICROSOFT
		// We can't choose values between the predefined constants,
		// so we have to "snap" the value to one of those constants.
		int const native_priority =
			priority <  priorities::lowest  ? priorities::idle    :
			priority <  priorities::low     ? priorities::lowest  :
			priority <  priorities::normal  ? priorities::low     :
			priority == priorities::normal  ? priorities::normal  :
			priority <= priorities::high    ? priorities::high    :
			priority <= priorities::highest ? priorities::highest : priorities::realtime;
		if(!::SetThreadPriority(native_handle_, native_priority))
			throw exception(UNIVERSALIS__COMPILER__LOCATION);
	#endif
}

/**********************************************************************/
// affinity_mask

affinity_mask::affinity_mask()
	#if defined DIVERSALIS__OS__MICROSOFT
		: native_mask_()
	#endif
{
	#if defined DIVERSALIS__OS__POSIX
		CPU_ZERO(&native_mask_);
	#endif
}

unsigned int affinity_mask::active_count() const {
	unsigned int result(0);
	for(unsigned int i = 0; i < size(); ++i) if((*this)(i)) ++result;
	return result;
}

unsigned int affinity_mask::size() const {
	return
		#if defined DIVERSALIS__OS__POSIX
			CPU_SETSIZE;
		#elif defined DIVERSALIS__OS__MICROSOFT
			sizeof native_mask_ << 3;
		#else
			#error unsupported operating system
		#endif
}

bool affinity_mask::operator()(unsigned int cpu_index) const {
	assert(cpu_index < size());
	return
		#if defined DIVERSALIS__OS__POSIX
			CPU_ISSET(cpu_index, &native_mask_);
		#elif defined DIVERSALIS__OS__MICROSOFT
			native_mask_ & native_mask_type(1) << cpu_index;
		#else
			#error unsupported operating system
		#endif
}

void affinity_mask::operator()(unsigned int cpu_index, bool active) {
	assert(cpu_index < size());
	#if defined DIVERSALIS__OS__POSIX
		if(active)
			CPU_SET(cpu_index, &native_mask_);
		else
			CPU_CLR(cpu_index, &native_mask_);
	#elif defined DIVERSALIS__OS__MICROSOFT
		native_mask_ |= native_mask_type(1) << cpu_index;
	#else
		#error unsupported operating system
	#endif
	assert((*this)(cpu_index) == active);
}

}}}
