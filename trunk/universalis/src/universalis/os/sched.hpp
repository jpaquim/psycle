// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2008-2009 members of the psycle project http://psycle.sourceforge.net

///\interface universalis::os::sched

#ifndef UNIVERSALIS__OS__SCHED__INCLUDED
#define UNIVERSALIS__OS__SCHED__INCLUDED
#pragma once

#include <diversalis/os.hpp>
#if defined DIVERSALIS__OS__POSIX
	#include <sched.h>
	#include <pthread.h>
#elif defined DIVERSALIS__OS__MICROSOFT
	#include <windows.h>
#else
	#error unsupported operating system
#endif
#if defined BOOST_AUTO_TEST_CASE
	#include <sstream>
#endif
#include <stdexcept>

#define UNIVERSALIS__COMPILER__DYNAMIC_LINK UNIVERSALIS__SOURCE
#include <universalis/compiler/dynamic_link/begin.hpp>

namespace universalis { namespace os { namespace sched {

class UNIVERSALIS__COMPILER__DYNAMIC_LINK affinity_mask {
	public:
		/// initialises a zeroed-out mask
		affinity_mask();

		/// returns the number active cpus in the mask
		///\seealso std::thread::hardware_concurrency (which is supposed to be process-independent)
		unsigned int active_count() const;

		/// returns the max cpu index
		unsigned int size() const;

		/// returns whether cpu at index is active; the index range is [0, size()[.
		bool operator()(unsigned int cpu_index) const;
		/// sets whether cpu at index is active; the index range is [0, size()[.
		void operator()(unsigned int cpu_index, bool);

	private:
		typedef
			#if defined DIVERSALIS__OS__POSIX
				cpu_set_t
			#elif defined DIVERSALIS__OS__MICROSOFT
				DWORD
			#else
				#error unsupported operating system
			#endif
			native_mask_type;
		native_mask_type native_mask_;
		friend class thread;
		friend class process;
};

class UNIVERSALIS__COMPILER__DYNAMIC_LINK process {
	public:
		/// native handle type
		typedef
			#if defined DIVERSALIS__OS__POSIX
				pid_t
			#elif defined DIVERSALIS__OS__MICROSOFT
				HANDLE
			#else
				#error unsupported operating system
			#endif
			native_handle_type;
	private:
		native_handle_type native_handle_;
		
	public:
		/// initialises an instance that represents the current process
		process();
		/// initialises an instance that represents the given process
		process(native_handle_type native_handle) : native_handle_(native_handle) {}

		/// gets the affinity mask against the set of cpu available to the process.
		class affinity_mask affinity_mask() const throw(std::runtime_error);
		/// sets the affinity mask against the set of cpu available to the process.
		void affinity_mask(class affinity_mask const &) throw(std::runtime_error);

		typedef
			#if defined DIVERSALIS__OS__POSIX
				int
			#elif defined DIVERSALIS__OS__MICROSOFT
				DWORD
			#else
				#error unsupported operating system
			#endif
			priority_type;

		/// gets the process priority level
		priority_type priority() throw(std::runtime_error);
		/// sets the process priority level
		void priority(priority_type priority) throw(std::runtime_error);

		struct priorities {
			#if defined DIVERSALIS__OS__MICROSOFT
				static priority_type const idle     = IDLE_PRIORITY_CLASS;
				static priority_type const lowest   = IDLE_PRIORITY_CLASS;
				static priority_type const low      = BELOW_NORMAL_PRIORITY_CLASS;
				static priority_type const normal   = NORMAL_PRIORITY_CLASS;
				static priority_type const high     = ABOVE_NORMAL_PRIORITY_CLASS;
				static priority_type const highest  = HIGH_PRIORITY_CLASS;
				static priority_type const realtime = REALTIME_PRIORITY_CLASS;
			#else
				// Note: These are nice values. Some systems may accept +20.
				static priority_type const idle     = -20;
				static priority_type const lowest   = -10;
				static priority_type const low      = -5;
				static priority_type const normal   =  0;
				static priority_type const high     = +5;
				static priority_type const highest  = +10;
				static priority_type const realtime = +19;
			#endif
		};
};

class UNIVERSALIS__COMPILER__DYNAMIC_LINK thread {
	public:
		/// native handle type
		typedef
			#if defined DIVERSALIS__OS__POSIX
				::pthread_t
			#elif defined DIVERSALIS__OS__MICROSOFT
				HANDLE
			#else
				#error unsupported operating system
			#endif
			native_handle_type;
	private:
		native_handle_type native_handle_;

	public:
		/// initialises an instance that represents the current thread
		thread();
		/// initialises an instance that represents the given thread
		thread(native_handle_type native_handle) : native_handle_(native_handle) {}

		/// gets the affinity mask against the set of cpu available to the process.
		class affinity_mask affinity_mask() const throw(std::runtime_error);
		/// sets the affinity mask against the set of cpu available to the process.
		void affinity_mask(class affinity_mask const &) throw(std::runtime_error);

		typedef int priority_type;

		/// gets the thread priority level
		priority_type priority() throw(std::runtime_error);
		/// sets the thread priority level
		void priority(priority_type priority) throw(std::runtime_error);

		struct priorities {
			#if defined DIVERSALIS__OS__MICROSOFT
				static priority_type const idle     = THREAD_PRIORITY_IDLE;
				static priority_type const lowest   = THREAD_PRIORITY_LOWEST;
				static priority_type const low      = THREAD_PRIORITY_BELOW_NORMAL;
				static priority_type const normal   = THREAD_PRIORITY_NORMAL;
				static priority_type const high     = THREAD_PRIORITY_ABOVE_NORMAL;
				static priority_type const highest  = THREAD_PRIORITY_HIGHEST;
				static priority_type const realtime = THREAD_PRIORITY_TIME_CRITICAL;
				// Note: If the thread (actually, the process) has the REALTIME_PRIORITY_CLASS base class,
				//       then this can also be -7, -6, -5, -4, -3, 3, 4, 5, or 6.
			#else
				// Note: These are not the actual native values but an arbitrary scale.
				//       The native values depends on the scheduling policy,
				//       and the sched_get_priority_min(policy) and sched_get_priority_max(policy) functions
				//       are used to rescale to native values.
				//       The policy is chosen this way:
				//           - value >  normal: if min == max, the policy is set to SCHED_RR,
				//           - value == normal: SCHED_OTHER (the default policy of the os),
				//           - value <  normal: SCHED_BATCH if defined, otherwise SCHED_OTHER.
				static priority_type const idle     = -300;
				static priority_type const lowest   = -200;
				static priority_type const low      = -100;
				static priority_type const normal   =    0;
				static priority_type const high     = +100;
				static priority_type const highest  = +200;
				static priority_type const realtime = +300;
			#endif
		};
};

#if defined BOOST_AUTO_TEST_CASE
	BOOST_AUTO_TEST_CASE(affinity_test) {
		{
			process p;
			std::ostringstream s; s << "process affinity mask active count: " << p.affinity_mask().active_count();
			BOOST_MESSAGE(s.str());
		}
		{
			thread t;
			std::ostringstream s; s << "thread affinity mask active count: " << t.affinity_mask().active_count();
			BOOST_MESSAGE(s.str());
		}
	}
	
	void thread_priority_test_one(int p) {
		thread t;
		int p0 = t.priority(); // save previous
		{ std::ostringstream s; s << "setting thread priority to: " << p;
			BOOST_MESSAGE(s.str());
		}
		t.priority(p);
		p = t.priority();
		{ std::ostringstream s; s << "thread priority is: " << p;
			BOOST_MESSAGE(s.str());
		}
		t.priority(p0); // reset to previous
	}
	
	BOOST_AUTO_TEST_CASE(thread_priority_test) {
		typedef thread::priorities priorities;
		thread_priority_test_one(priorities::idle);
		thread_priority_test_one(priorities::lowest);
		thread_priority_test_one(priorities::low);
		thread_priority_test_one(priorities::normal);
		thread_priority_test_one(priorities::high);
		thread_priority_test_one(priorities::highest);
		thread_priority_test_one(priorities::realtime);
	}
#endif

}}}

#include <universalis/compiler/dynamic_link/end.hpp>

#endif
