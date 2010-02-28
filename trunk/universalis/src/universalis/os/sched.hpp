// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2008-2009 members of the psycle project http://psycle.sourceforge.net

///\interface universalis::os::sched

#ifndef UNIVERSALIS__OS__SCHED__INCLUDED
#define UNIVERSALIS__OS__SCHED__INCLUDED
#pragma once

#include "exception.hpp"
#if defined DIVERSALIS__OS__POSIX
	#include <pthread.h>
#elif defined DIVERSALIS__OS__MICROSOFT
	#include "include_windows_without_crap.hpp"
#else
	#error unsupported operating system
#endif
#if defined BOOST_AUTO_TEST_CASE
	#include <sstream>
#endif

namespace universalis { namespace os { namespace sched {

/// cpu affinity mask
class UNIVERSALIS__DECL affinity_mask {
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
				DWORD_PTR
			#else
				#error unsupported operating system
			#endif
			native_mask_type;
		native_mask_type native_mask_;
		friend class thread;
		friend class process;
};

/// process
class UNIVERSALIS__DECL process {
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
		///\name ctors
		///\{
			/// initialises an instance that represents the current process
			process();
			/// initialises an instance that represents the given process
			process(native_handle_type native_handle) : native_handle_(native_handle) {}
		///\}

		///\name cpu affinity
		///\{
			/// affinity mask type
			typedef class affinity_mask affinity_mask_type;
			/// gets the affinity mask against the set of cpu available to the process.
			affinity_mask_type affinity_mask() const throw(exception);
			/// sets the affinity mask against the set of cpu available to the process.
				#if defined DIVERSALIS__OS__MICROSOFT
					/// special case for windows, this must be done inline!
					/// The doc says:
					/// "Do not call SetProcessAffinityMask in a DLL that may be called by processes other than your own."
					inline
				#endif
			void affinity_mask(affinity_mask_type const &) throw(exception);
		///\}

		///\name priority
		///\{
			/// process priority type
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
			priority_type priority() throw(exception);
			/// sets the process priority level
			void priority(priority_type priority) throw(exception);

			/// process priority values
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
		///\}
};

/// thread
class UNIVERSALIS__DECL thread {
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
		///\name ctors
		///\{
			/// initialises an instance that represents the current thread
			thread();
			/// initialises an instance that represents the given thread
			thread(native_handle_type native_handle) : native_handle_(native_handle) {}
		///\}

		///\name cpu affinity
		///\{
			/// affinity mask type
			typedef class affinity_mask affinity_mask_type;
			/// gets the affinity mask against the set of cpu available to the process.
			class affinity_mask affinity_mask() const throw(exception);
			/// sets the affinity mask against the set of cpu available to the process.
			void affinity_mask(class affinity_mask const &) throw(exception);
		///\}

		///\name priority
		///\{
			/// thread priority type
			typedef int priority_type;

			/// gets the thread priority level
			priority_type priority() throw(exception);
			/// sets the thread priority level
			void priority(priority_type priority) throw(exception);

			/// thread priority values
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
					static priority_type const idle     = -300; // min
					static priority_type const lowest   = -200; // (max + min) * 1 / 6
					static priority_type const low      = -100; // (max + min) * 2 / 6
					static priority_type const normal   =    0; // (max + min) / 2
					static priority_type const high     = +100; // (max + min) * 4 / 6
					static priority_type const highest  = +200; // (max + min) * 5 / 6
					static priority_type const realtime = +300; // max
				#endif
			};
		///\}
};

/****************************************************/
// inline implementation

#if defined DIVERSALIS__OS__MICROSOFT
	// special case for windows, this must be done inline!
	// The doc says:
	// "Do not call SetProcessAffinityMask in a DLL that may be called by processes other than your own."
	void inline process::affinity_mask(class affinity_mask const & affinity_mask) throw(exception) {
		if(!SetProcessAffinityMask(native_handle_, affinity_mask.native_mask_))
			throw exception(UNIVERSALIS__COMPILER__LOCATION);
	}
#endif

/****************************************************/
// test cases

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
		try {
			typedef thread::priorities priorities;
			thread_priority_test_one(priorities::idle);
			thread_priority_test_one(priorities::lowest);
			thread_priority_test_one(priorities::low);
			thread_priority_test_one(priorities::normal);
			thread_priority_test_one(priorities::high);
			thread_priority_test_one(priorities::highest);
			thread_priority_test_one(priorities::realtime);
		} catch(exceptions::operation_not_permitted e) {
			std::ostringstream s;
			s << "could not set thread priority: " << e.what();
			BOOST_MESSAGE(s.str());
		}
	}
#endif

}}}

#endif
