// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2008 members of the psycle project http://psycle.pastnotecut.org ; johan boule <bohan@jabber.org>

///\file \brief date_time standard header
#pragma once
/*
	The files included below implement the C++ standards proposal at http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2007/n2328.html
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
#include "detail/duration.hpp"
#include "detail/utc_time.hpp"
#include "detail/hiresolution_clock.hpp"
