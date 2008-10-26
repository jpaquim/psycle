// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2006-2008 members of the psycle project http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>

#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <iostream>

#define USE_GCC

#if !defined USE_GCC
	#include <glib/gatomic.h>
#endif

template<typename Value>
bool inline compare_and_swap(Value * const address, Value const old_value, Value const new_value) {
	#if defined USE_GCC
		return __sync_bool_compare_and_swap(address, old_value, new_value);
	#else
		// no return statement to error on purpose -- see template specialisations below
	#endif
}

#if !defined USE_GCC
	template<>
	bool inline compare_and_swap< ::gpointer>(::gpointer * const address, ::gpointer const old_value, ::gpointer const new_value) {
		return ::g_atomic_pointer_compare_and_exchange(address, old_value, new_value);
	}

	template<>
	bool inline compare_and_swap< ::gint>(::gint * const address, ::gint const old_value, ::gint const new_value) {
		return ::g_atomic_int_compare_and_exchange(address, old_value, new_value);
	}
#endif

int const threads = 4;
int const iterations = 10000000;
int results[threads][iterations];
int shared_counter(0);

void loop(int thread, int flip, int flop) {
	{ // dummy loop to make sure all threads have had time to start
		int i(300000000);
		while(--i);
		results[threads][0] = i;
	}
	for(int i(0); i < iterations; ++i) {
		retry:
		bool b(compare_and_swap(&shared_counter, flip, flop));
		//if(!b) goto retry;
		results[thread][i] = b;//shared_counter;
	}
}

int main() {
	for(int t(0); t < threads; ++t) for(int i(0); i < iterations; ++i) results[t][i] = 0;
	boost::thread thread1(boost::bind(&loop, 0, 0, 1));
	boost::thread thread2(boost::bind(&loop, 1, 1, 2));
	boost::thread thread3(boost::bind(&loop, 2, 2, 3));
	boost::thread thread4(boost::bind(&loop, 3, 3, 0));
	thread1.join();
	thread2.join();
	thread3.join();
	thread4.join();
	for(int t(0); t < threads; ++t) {
		int prev_r(~results[t][0]), prev_i(0), diffs(0), diff_tot(0), max_diff(0);
		for(int i(0); i < iterations; ++i) {
			int const r(results[t][i]);
			if(prev_r != r) {
				++diffs;
				int const diff = i - prev_i;
				diff_tot += diff;
				prev_i = i;
				prev_r = r;
				if(diff > max_diff) max_diff = diff;
			}
		}
		std::cout
			<< "thread " << t
			<< "\tdiffs " << diffs
			<< "\tmax diff " << max_diff
			<< "\tdiff avg " << double(diff_tot) / diffs
			<< '\n';
	}
}
