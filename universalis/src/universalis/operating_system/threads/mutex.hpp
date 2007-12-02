// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2004-2007 psycle development team http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>

///\interface universalis::operating_system::threads::thread

/**************************** note ****************************
the C++ standardisation commitee is working on a threading api that will provide an equivalent interface:
#include <mutex>
class std::mutex;
class std::recursive_mutex;
class std::timed_mutex;
class std::recursive_timed_mutex;
template<typename Mutex> class scoped_lock;
template<typename Mutex> class unique_lock;
TR2:
class std::read_write_mutex;
template<typename Read_Write_Mutex> class shared_lock;

see reference:
	Document number: N2320=07-0180
	http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2007/n2320.html
***************************************************************/

#pragma once
#include <boost/thread/mutex.hpp>
#include <boost/thread/recursive_mutex.hpp>
namespace universalis { namespace operating_system { namespace threads {

	/*
		Citation of the standard draft:
		"
			Lock Rationale and Examples
			
			Unlike boost locks, the locks proposed herein are not nested types of the mutex classes
			but class templates which are templated on the mutex type.
			The locks thus become far more reusable.
			They can be instantiated with any standard or user-defined mutex which meets the mutex requirements.
		"
		
		So, to enforce conformance with the (future) standard thread library,
		we hide the scoped_lock nested type contained in boost mutexes,
		and we provide scoped_lock as a free-standing template class.
	*/

	template<typename Mutex>
	class scoped_lock {
		public:
			scoped_lock(Mutex & m) : lock_(m);
		private:
			typename Mutex::scoped_lock lock_;
	};

	class mutex : private boost::mutex {
		friend class scoped_lock<mutex>;
		public:
			void lock();
			bool try_lock();
			void unlock();
	};
	
	class recursive_mutex : private boost::recursive_mutex {
		friend class scoped_lock<recursive::mutex>;
			void lock();
			bool try_lock();
			void unlock();
	};
	
	#if 0 ///\todo
		class timed_mutex : private boost::mutex {
			friend class scoped_lock<mutex>;
			public:
				template<typename Elapsed_Time> void timed_lock<Elapsed_Time const & elapsed_time);
				void lock();
				bool try_lock();
				void unlock();
		};
		
		class timed_recursive_mutex : private boost::recursive::mutex {
			friend class scoped_lock<recursive::mutex>;
				template<typename Elapsed_Time> void timed_lock<Elapsed_Time const & elapsed_time);
				void lock();
				bool try_lock();
				void unlock();
		};
	#endif

}}}
