#pragma once
#include "duration.hpp"
#include "utc_time.hpp"
#include <universalis/operating_system/clocks.hpp>
namespace std {
	/*
		This file implements the C++ standards proposal at http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2007/n2328.html
		This intent is to use these types with the standard thread api timed_wait and sleep functions.
		For example:
			std::unique_lock<std::mutex> lk(mut);
			// Wait for 2 seconds on a condition variable
			std::utc_time time_out = std::hiresolution_clock::universal_time() + std::seconds(2);
			{
				bool timed_out = !cv.timed_wait(lk, time_out);
				if (timed_out)
					// deal with time out
			}
			
			std::this_thread::sleep(std::hiresolution_clock::universal_time() + std::nanoseconds(123456789));
			std::this_thread::sleep(std::hiresolution_clock::universal_time() + std::microseconds(123456));
			std::this_thread::sleep(std::hiresolution_clock::universal_time() + std::milliseconds(123));
	*/

	/*******************************************************************/
	// time clock

	template<typename Time_Point>
	class hiresolution_clock {
		public:
			Time_Point static universal_time() {
				std::nanoseconds const ns(operating_system::clocks::utc_since_epoch::current());
				Time_Point t(ns);
				return t;
			}
	};
}
