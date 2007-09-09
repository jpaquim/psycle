// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2007 johan boule <bohan@jabber.org>
// copyright 2004-2007 psycledelics http://psycle.pastnotecut.org

// This is a unit test for universalis::processor::atomic::compare_and_swap
#if defined UNIVERSALIS__PROCESSOR__ATOMIC__COMPARE_AND_SWAP__UNIT_TEST
	#include <pthread.h>
	#include <sched.h>
	//#include "compare_and_swap.hpp"
	template<typename Value>
	bool inline compare_and_swap(Value * const address, Value const old_value, Value const new_value)
	{
		return __sync_bool_compare_and_swap(address, old_value, new_value);
	}
	#include <boost/thread.hpp>
	#include <iostream>
	int shared_counter(0);
	int * shared_counter_pointer(&shared_counter);
	void f1()
	{
		int local_counter(0);
		for(int i(0); i < 10000; ++i)
		{
			retry:
			bool b(compare_and_swap(shared_counter_pointer, 0, 1));
			if(!b) { sched_yield(); goto retry; }
			std::cout << "thread id: " << pthread_self() << ", b: " << b << ", local counter: " << local_counter << "\n";
		}
	}
	void f2()
	{
		int local_counter(0);
		for(int i(0); i < 10000; ++i)
		{
			retry:
			bool b(compare_and_swap(shared_counter_pointer, 1, 0));
			if(!b) { /*sched_yield();*/ goto retry; }
			std::cout << "thread id: " << pthread_self() << ", b: " << b << ", local counter: " << local_counter << "\n";
		}
	}
	int main()
	{
		boost::thread thread1(&f1);
		boost::thread thread2(&f2);
		thread1.join();
		thread2.join();
	}
#endif
