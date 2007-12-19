// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2004-2007 psycledelics http://psycle.pastnotecut.org ; johan boule <bohan@jabber.org>

///\file \brief thread standard header
/// This file implements the C++ standards proposal at http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2007/n2320.html
#pragma once
#include <diversalis/operating_system.hpp>
#include <boost/thread.hpp>
#include <boost/operators.hpp>
#include <functional>
namespace std {

	namespace this_thread {}

	class thread {
		public:
			typedef boost::thread native_handle_type;
		private:
			native_handle_type native_handle_;
	
		public:
			template<typename Functor>
			explicit thread(Functor functor) : native_handle_(functor) {}

			bool joinable() const { return native_handle_.joinable(); }
			void join() { native_handle_.join(); }

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
						// unlike glibmm's gthread, boost's thread gives no way to retrieve the current thread
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
					id(implementation_ implementation) : implementation_(implementation) {} friend id this_thread::get_id();
			};
			id get_id() const { return id_; }
		private:
			id id_;
	};

	namespace this_thread {

		thread::id get_id() {
			thread::id id(
				// unlike glibmm's gthread, boost's thread gives no way to retrieve the current thread
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

		void yield() { boost::thread::yield(); }

		/// see the standard header date_time for duration types implementing the Elapsed_Time concept
		template<typename Elapsed_Time>
		void sleep(Elapsed_Time const & elapsed_time) {
			std::nanoseconds const ns(elapsed_time);

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
		}
	}
}
