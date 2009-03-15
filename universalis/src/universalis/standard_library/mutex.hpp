// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2008 members of the psycle project http://psycle.pastnotecut.org ; johan boule <bohan@jabber.org>

///\file \brief mutex standard header
/// This file implements the C++ standards proposal at http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2007/n2320.html
#pragma once
#include "detail/boost_xtime.hpp"
#include <boost/thread/mutex.hpp>
#include <boost/thread/recursive_mutex.hpp>
namespace std {

class mutex;
class recursive_mutex;
class timed_mutex;
class recursive_timed_mutex;

template<typename Mutex>
class scoped_lock;

template<typename Mutex>
class unique_lock;

typedef boost::lock_error lock_error;

///\internal
namespace detail {
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
			bool try_lock() throw(lock_error) { return this->implementation_lock_.try_lock(); }
			void unlock() throw(lock_error) { implementation_lock_.unlock(); }
		private:
			Boost_Mutex implementation_;
			Boost_Lock  implementation_lock_;
			typedef Boost_Lock implementation_lock_type;
			template<typename, typename> friend class boost_timed_mutex_wrapper;
			template<typename, typename> friend class boost_condition_wrapper;
			template<typename> friend class scoped_lock;
			template<typename> friend class unique_lock;
	};

	template<typename Boost_Timed_Mutex, typename Boost_Timed_Lock>
	class boost_timed_mutex_wrapper : public boost_mutex_wrapper<Boost_Timed_Mutex, Boost_Timed_Lock> {
		public:
			/// see the standard header date_time for duration types implementing the Elapsed_Time concept
			template<typename Elapsed_Time>
			bool timed_lock(Elapsed_Time const & elapsed_time) {
				return this->implementation_lock_.timed_lock(universalis::standard_library::detail::boost_xtime_get_and_add(elapsed_time));
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

		explicit scoped_lock(Mutex & mutex)
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

		~scoped_lock() {}

		/*constexpr*/ bool owns() const { return true; }
	private:
		typename Mutex::implementation_lock_type implementation_lock_;
		template<typename, typename> friend class detail::boost_condition_wrapper;
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

		~unique_lock() { if(owns_) implementation_lock_.unlock(); }

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
		Mutex * mutex_;
		typename Mutex::implementation_lock_type implementation_lock_;
			template<typename, typename> friend class detail::boost_condition_wrapper;
		bool owns_;
};

}
