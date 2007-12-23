// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2007 psycledelics http://psycle.pastnotecut.org ; johan boule <bohan@jabber.org>

///\file \brief mutex standard header
/// This file implements the C++ standards proposal at http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2007/n2320.html
#pragma once
#include <boost/thread/mutex.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include "hiresolution_clock.hpp"
namespace std {

	class mutex;
	class timed_mutex;
	class recursive_mutex;
	class recursive_timed_mutex;

	template<typename Mutex>
	class scoped_lock;

	template<typename Mutex>
	class unique_lock;
	
	template<typename Lock>
	class condition;

	typedef boost::lock_error lock_error;

	namespace detail {

		template<typename Boost_Timed_Mutex>
		class boost_timed_mutex_wrapper;

		template<typename Boost_Try_Mutex>
		class boost_try_mutex_wrapper;

		template<typename Boost_Mutex>
		class boost_mutex_wrapper : private boost::noncopyable {
			public:
				boost_mutex_wrapper() : implementation_lock_(implementation_, false) {}
				void lock() throw(lock_error) { implementation_lock_.lock(); }
				void unlock() throw(lock_error) { implementation_lock_.unlock(); }
			private:
				Boost_Mutex   implementation_;
				Boost_Mutex & implementation() { return implementation_; }
					friend class boost_try_mutex_wrapper<Boost_Mutex>;
					friend class boost_timed_mutex_wrapper<Boost_Mutex>;

				typedef typename Boost_Mutex::scoped_lock implementation_lock_type;
				implementation_lock_type   implementation_lock_;
				implementation_lock_type & implementation_lock() { return implementation_lock_; }
					friend class condition<mutex>;
					friend class condition<recursive_mutex>;
					friend class scoped_lock<mutex>;
					friend class scoped_lock<recursive_mutex>;
					friend class unique_lock<mutex>;
					friend class unique_lock<recursive_mutex>;
		};

		template<typename Boost_Try_Mutex>
		class boost_try_mutex_wrapper : public boost_mutex_wrapper<Boost_Try_Mutex> {
			public:
				boost_try_mutex_wrapper() : implementation_try_lock_(this->implementation(), false) {}
				bool try_lock() throw(lock_error) { return implementation_try_lock_.try_lock(); }
			private:
				typename Boost_Try_Mutex::scoped_try_lock implementation_try_lock_;
		};

		template<typename Boost_Timed_Mutex>
		class boost_timed_mutex_wrapper : public boost_try_mutex_wrapper<Boost_Timed_Mutex> {
			public:
				boost_timed_mutex_wrapper() : implementation_timed_lock_(this->implementation(), false) {}

				/// see the standard header date_time for duration types implementing the Elapsed_Time concept
				template<typename Elapsed_Time>
				bool timed_lock(Elapsed_Time const & elapsed_time) {
					implementation_timed_lock_.timed_lock(universalis::standard_library::detail::boost_xtime_get_and_add(elapsed_time));
				}
			private:
				typedef typename Boost_Timed_Mutex::scoped_timed_lock implementation_timed_lock_type;
				implementation_timed_lock_type   implementation_timed_lock_;
				implementation_timed_lock_type & implementation_timed_lock() { return implementation_timed_lock_; }
					friend class condition<timed_mutex>;
					friend class condition<recursive_timed_mutex>;
					friend class scoped_lock<timed_mutex>;
					friend class scoped_lock<recursive_timed_mutex>;
					friend class unique_lock<timed_mutex>;
					friend class unique_lock<recursive_timed_mutex>;
		};
	}

	class mutex : public detail::boost_mutex_wrapper<boost::mutex> {};
	class timed_mutex : public detail::boost_timed_mutex_wrapper<boost::timed_mutex> {};
	class recursive_mutex : public detail::boost_mutex_wrapper<boost::recursive_mutex> {};
	class recursive_timed_mutex : public detail::boost_timed_mutex_wrapper<boost::recursive_timed_mutex> {};
	
	class defer_lock_type {};
	class try_lock_type {};
	class accept_ownership_type {};

	extern defer_lock_type       defer_lock;
	extern try_lock_type         try_to_lock;
	extern accept_ownership_type accept_ownership;

	template<typename Mutex>
	class scoped_lock : private boost::noncopyable {
		public:
			typedef Mutex mutex_type;

			explicit scoped_lock(mutex_type & mutex) : mutex_(mutex), implementation_lock_(mutex.implementation()) {}
			scoped_lock(mutex_type & mutex, accept_ownership_type) : mutex_(mutex), implementation_lock_(mutex.implementation(), false) {}
			~scoped_lock() { implementation_lock_.unlock(); /** \todo other locks too */ }
			/*constexpr*/ bool owns() const { return true; }
		private:
			mutex_type & mutex_;
			typename mutex_type::implementation_lock_type   implementation_lock_;
			typename mutex_type::implementation_lock_type & implementation_lock() { return implementation_lock_; }
				friend class condition<scoped_lock<std::mutex> >;
				friend class condition<scoped_lock<recursive_mutex> >;

			#if 0 ///\todo
			typename mutex_type::implementation_timed_lock_type implementation_timed_lock_;
			typename mutex_type::implementation_lock_type & implementation_lock() { return implementation_timed_lock_; }
				friend class condition<scoped_lock<timed_mutex> >;
				friend class condition<scoped_lock<recursive_timed_mutex> >;
			#endif
	};

	template<typename Mutex>
	class unique_lock : private boost::noncopyable {
		public:
			typedef Mutex mutex_type;

			unique_lock() : mutex_(), owns_() {}
			explicit unique_lock(mutex_type & mutex) : mutex_(&mutex), owns_(true), implementation_lock_(mutex.implementation()) {}
			unique_lock(mutex_type & mutex, defer_lock_type) : mutex_(&mutex), owns_(), implementation_lock_(mutex.implementation(), false) {}
			unique_lock(mutex_type & mutex, try_lock_type) : mutex_(&mutex), implementation_lock_(mutex.implementation(), false) { owns_ = /** \todo use implementation_try_lock_.try_lock() **/ mutex.try_lock(); }
			unique_lock(mutex_type & mutex, accept_ownership_type) : mutex_(&mutex), owns_(true), implementation_lock_(mutex.implementation()) {}
			~unique_lock() { if(owns_) implementation_lock_.unlock(); /** \todo other locks too */ }

			//unique_lock(unique_lock && u);
			//unique_lock & operator=(unique_lock && u);

			bool owns() const { return owns_; }
			operator void const * () const { return owns_ ? this : 0; }
			mutex_type * mutex() const { return mutex_; }
			//void swap(unique_lock && u);
			mutex_type * release() { owns_ = false; return mutex_; }

			void lock() throw(lock_error) {
				if(owns_) throw lock_error();
				mutex_->lock();
				owns_ = true;
			}

			void unlock() throw(lock_error) {
				if(!owns_) throw lock_error();
				mutex_->unlock();
				owns_ = false;
			}

			bool try_lock() throw(lock_error) {
				if(owns_) throw lock_error();
				owns_ = mutex_->try_lock();
				return owns_; 
			}

			template<typename Elapsed_Time>
			bool timed_lock(Elapsed_Time const & elapsed_time) {
				if(owns_) throw lock_error();
				owns_ = mutex_->timed_lock(elapsed_time);
				return owns_;
			}
		private:
			mutex_type * mutex_;
			bool owns_;

			typename mutex_type::implementation_lock_type   implementation_lock_;
			typename mutex_type::implementation_lock_type & implementation_lock() { return implementation_lock_; }
				friend class condition<unique_lock<std::mutex> >;
				friend class condition<unique_lock<recursive_mutex> >;

			#if 0 ///\todo
			typename mutex_type::implementation_timed_lock_type implementation_timed_lock_;
			typename mutex_type::implementation_lock_type & implementation_lock() { return implementation_timed_lock_; }
				friend class condition<unique_lock<timed_mutex> >;
				friend class condition<unique_lock<recursive_timed_mutex> >;
			#endif
	};
}
