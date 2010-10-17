// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2009 members of the psycle project http://psycle.pastnotecut.org ; johan boule <bohan@jabber.org>

///\file \brief mutex standard header
/// This file implements the C++ standards proposal at http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2007/n2320.html

#ifndef UNIVERSALIS__STDLIB__MUTEX__INCLUDED
#define UNIVERSALIS__STDLIB__MUTEX__INCLUDED
#pragma once

#include "detail/boost_xtime.hpp"
#include <boost/version.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/recursive_mutex.hpp>

namespace universalis { namespace stdlib {

class mutex;
class recursive_mutex;
class timed_mutex;
class recursive_timed_mutex;

template<typename Mutex>
class scoped_lock;

template<typename Mutex>
class unique_lock;

template<typename Lock>
class condition;

typedef boost::lock_error lock_error;

///\internal
namespace detail {

	#if defined DIVERSALIS__COMPILER__MICROSOFT && DIVERSALIS__COMPILER__VERSION < 1500
		template<typename, typename> class boost_timed_mutex_wrapper;
	#endif

	template<typename Boost_Mutex, typename Boost_Lock>
	class boost_mutex_wrapper : private boost::noncopyable {
		public:
			boost_mutex_wrapper()
			:
				implementation_lock_(implementation_,
					#if BOOST_VERSION >= 103500
						boost::defer_lock
					#else
						false
					#endif
				)
			{}

			void lock() throw(lock_error) { implementation_lock_.lock(); }
			bool try_lock() throw(lock_error) { return implementation_lock_.try_lock(); }
			void unlock() throw(lock_error) { implementation_lock_.unlock(); }
		private:
			Boost_Mutex implementation_;
			Boost_Lock  implementation_lock_;
			typedef Boost_Lock implementation_lock_type;
			#if defined DIVERSALIS__COMPILER__MICROSOFT && DIVERSALIS__COMPILER__VERSION < 1500
				friend class boost_timed_mutex_wrapper<Boost_Mutex, Boost_Lock>;
				friend class scoped_lock<mutex>;
				friend class scoped_lock<recursive_mutex>;
				friend class scoped_lock<timed_mutex>;
				friend class scoped_lock<recursive_timed_mutex>;
				friend class unique_lock<mutex>;
				friend class unique_lock<recursive_mutex>;
				friend class unique_lock<timed_mutex>;
				friend class unique_lock<recursive_timed_mutex>;
				friend class condition<mutex>;
				friend class condition<recursive_mutex>;
				friend class condition<timed_mutex>;
				friend class condition<recursive_timed_mutex>;
			#elif defined DIVERSALIS__COMPILER__CLANG
				template<typename, typename> friend class boost_timed_mutex_wrapper;
				template<typename> friend class stdlib::scoped_lock;
				template<typename> friend class stdlib::unique_lock;
				template<typename> friend class stdlib::condition;
			#else
				template<typename, typename> friend class boost_timed_mutex_wrapper;
				template<typename> friend class scoped_lock;
				template<typename> friend class unique_lock;
				template<typename> friend class condition;
			#endif
	};

	template<typename Boost_Timed_Mutex, typename Boost_Timed_Lock>
	class boost_timed_mutex_wrapper : public boost_mutex_wrapper<Boost_Timed_Mutex, Boost_Timed_Lock> {
		public:
			/// see the standard header date_time for duration types implementing the Elapsed_Time concept
			template<typename Elapsed_Time>
			bool timed_lock(Elapsed_Time const & elapsed_time) {
				return this->implementation_lock_.timed_lock(universalis::stdlib::detail::boost_xtime_get_and_add(elapsed_time));
			}
	};
}

class mutex
	: public detail::boost_mutex_wrapper<
		boost::try_mutex, // same as boost::mutex since version 1.35
		boost::try_mutex::scoped_try_lock> {};

class recursive_mutex
	: public detail::boost_mutex_wrapper<
		boost::recursive_try_mutex, // same as boost::recursive_mutex since version 1.35
		boost::recursive_try_mutex::scoped_try_lock> {};

class timed_mutex
	: public detail::boost_timed_mutex_wrapper<
		boost::timed_mutex,
		boost::timed_mutex::scoped_timed_lock> {};

class recursive_timed_mutex
	: public detail::boost_timed_mutex_wrapper<
		boost::recursive_timed_mutex,
		boost::recursive_timed_mutex::scoped_timed_lock> {};

class defer_lock_type {};
class try_lock_type {};
class accept_ownership_type {};

extern defer_lock_type       const defer_lock;
extern try_lock_type         const try_to_lock;
extern accept_ownership_type const accept_ownership;

template<typename Mutex>
class scoped_lock : private boost::noncopyable {
	public:
		typedef Mutex mutex_type;

		explicit scoped_lock(mutex_type & mutex)
		:
			implementation_lock_(mutex.implementation_,
				#if BOOST_VERSION >= 103500
					boost::defer_lock
				#else
					false
				#endif
			)
		{
			implementation_lock_.lock();
		}

		scoped_lock(mutex_type & mutex, accept_ownership_type)
		:
			implementation_lock_(mutex.implementation_,
				#if BOOST_VERSION >= 103500
					boost::defer_lock
				#else
					false
				#endif
			)
		{}

		/*constexpr*/ bool owns() const { return true; }
	private:
		typename mutex_type::implementation_lock_type implementation_lock_;
		#if defined DIVERSALIS__COMPILER__MICROSOFT && DIVERSALIS__COMPILER__VERSION < 1500
			//friend class condition<scoped_lock<mutex_type> >;
			friend class condition<scoped_lock<mutex> >;
			friend class condition<scoped_lock<recursive_mutex> >;
			friend class condition<scoped_lock<timed_mutex> >;
			friend class condition<scoped_lock<recursive_timed_mutex> >;
		#else
			template<typename> friend class condition;
		#endif
};

template<typename Mutex>
class unique_lock : private boost::noncopyable {
	public:
		typedef Mutex mutex_type;

		unique_lock() : mutex_(), owns_() {}

		explicit unique_lock(mutex_type & mutex)
		:
			mutex_(&mutex),
			implementation_lock_(mutex.implementation_,
				#if BOOST_VERSION >= 103500
					boost::defer_lock
				#else
					false
				#endif
			),
			owns_(true)
		{
			implementation_lock_.lock();
		}

		unique_lock(mutex_type & mutex, defer_lock_type)
		:
			mutex_(&mutex),
			implementation_lock_(mutex.implementation_,
				#if BOOST_VERSION >= 103500
					boost::defer_lock
				#else
					false
				#endif
			),
			owns_(false)
		{}

		unique_lock(mutex_type & mutex, try_lock_type)
		:
			mutex_(&mutex),
			implementation_lock_(mutex.implementation_,
				#if BOOST_VERSION >= 103500
					boost::defer_lock
				#else
					false
				#endif
			),
			owns_(implementation_lock_.try_lock()) {}

		unique_lock(mutex_type & mutex, accept_ownership_type)
		:
			mutex_(&mutex),
			implementation_lock_(mutex.implementation_,
				#if BOOST_VERSION >= 103500
					boost::defer_lock
				#else
					false
				#endif
			),
			owns_(true)
		{}

		~unique_lock() { if(owns_) implementation_lock_.unlock(); } //?

		bool owns() const { return owns_; }
		operator void const * () const { return owns_ ? this : 0; }

		mutex_type * mutex() const { return mutex_; }
		mutex_type * release() { owns_ = false; return mutex_; }

		void lock() throw(lock_error) {
			if(owns_) throw lock_error();
			implementation_lock_.lock();
			owns_ = true;
		}

		void unlock() throw(lock_error) {
			if(!owns_) throw lock_error();
			implementation_lock_.unlock();
			owns_ = false;
		}

		bool try_lock() throw(lock_error) {
			if(owns_) throw lock_error();
			owns_ = implementation_lock_.try_lock();
			return owns_; 
		}

		/// see the standard header date_time for duration types implementing the Elapsed_Time concept
		template<typename Elapsed_Time>
		bool timed_lock(Elapsed_Time const & elapsed_time) {
			if(owns_) throw lock_error();
			owns_ = implementation_lock_.timed_lock(elapsed_time);
			return owns_;
		}

	private:
		mutex_type * mutex_;
		typename mutex_type::implementation_lock_type implementation_lock_;
		#if defined DIVERSALIS__COMPILER__MICROSOFT && DIVERSALIS__COMPILER__VERSION < 1500
			//friend class condition<unique_lock<mutex_type> >;
			friend class condition<unique_lock<std::mutex> >;
			friend class condition<unique_lock<recursive_mutex> >;
			friend class condition<unique_lock<timed_mutex> >;
			friend class condition<unique_lock<recursive_timed_mutex> >;
		#else
			template<typename> friend class condition;
		#endif
		bool owns_;
};

}}

/****************************************************************************/
// injection in std namespace
namespace std { using namespace universalis::stdlib; }

#endif
