// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2004-2007 psycle development team http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>

///\file \brief thread standard header
/// This file implements the C++ standards proposal at http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2007/n2320.html
#pragma once
#include <diversalis/operating_system.hpp>
#include "detail/boost_xtime.hpp"
#include <boost/thread.hpp>
#include <boost/operators.hpp>
namespace std {

	class thread {
		public:
			typedef boost::thread native_handle_type;
		private:
			native_handle_type native_handle_;
	
		public:
			template<typename Functor>
			explicit thread(Functor functor) : native_handle_(functor) {}
			void join() { native_handle_.join(); }
			bool joinable() const { return true; /**\todo return native_handle_.joinable(); */ }

		///\name id
		///\todo
		///\{
		#if 0
			public:
				class id
				: private
					boost::equality_comparable< id,
					boost::less_than_comparable< id
					> >
				{
					public:
						id() : implementation_() {}
						bool operator==(id const & that) const { return this->implementation_ == that.implementation_; }
						bool operator< (id const & that) const { return this->implementation_ <  that.implementation_; }
					private:
						typedef
							// unlike glibmm's gthread, boost's thread gives no way to retrieve the current thread (actually, yes, boost::thread's default constructor represents the current thread)
							// so we need to know the OS type
							#if defined DIVERSALIS__OPERATING_SYSTEM__POSIX
								::pthread_t *
							#elif defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT
								::DWORD
							#else
								#error unsupported operating system
							#endif
							implementation_type;

						implementation_type implementation_;

						friend id this_thread::id();
						id(implementation_type implementation) : implementation_(implementation) {}
				};
				id get_id() const { return id_; }
			private:
				id id_; ///\todo set it
		#endif
		///\}
	};

	namespace this_thread {

		#if 0 ///\todo
		thread::id inline id() {
			thread::id id(
				// unlike glibmm's gthread, boost's thread gives no way to retrieve the current thread (actually, yes, boost::thread's default constructor represents the current thread)
				// so we need to know the OS type
				#if defined DIVERSALIS__OPERATING_SYSTEM__POSIX
					::pthread_self()
				#elif defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT
					::GetCurrentThreadId()
				#else
					#error unsupported operating system
				#endif
			);
			return id;
		}
		#else
			// unlike glibmm's gthread, boost's thread gives no way to retrieve the current thread (actually, yes, boost::thread's default constructor represents the current thread)
			// so we need to know the OS type
			#if defined DIVERSALIS__OPERATING_SYSTEM__POSIX
				::pthread_t * inline id() { return ::pthread_self(); }
			#elif defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT
				::DWORD       inline id() { return ::GetCurrentThreadId(); }
			#else
				#error unsupported operating system
			#endif
		#endif

		void inline yield() { boost::thread::yield(); }

		/// see the standard header date_time for duration types implementing the Elapsed_Time concept
		template<typename Elapsed_Time>
		void sleep(Elapsed_Time const & elapsed_time) {
			// boost::thread::sleep sleeps until the given absolute event date.
			// So, we compute the event date by getting the current date and adding the delta to it.
			// Note: boost::thread::sleep returns on interruptions (at least on posix)
			boost::thread::sleep(universalis::standard_library::detail::boost_xtime_get_and_add(elapsed_time));
		}
	}
}
