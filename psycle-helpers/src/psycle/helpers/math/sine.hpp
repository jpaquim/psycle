// -*- mode:c++; indent-tabs-mode:t -*-
// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2008 members of the music-dsp mailing list http://www.music.columbia.edu/cmc/music-dsp and of the psycle project http://psycle.sourceforge.net 

#pragma once
#include <cmath>
#if defined BOOST_AUTO_TEST_CASE
	#include "pi.hpp"
	#include <universalis/operating_system/clocks.hpp>
	#include <sstream>
#endif
namespace psycle { namespace helpers { namespace math {

// worth reading
// http://www.audiomulch.com/~rossb/code/sinusoids/
// http://www.devmaster.net/forums/showthread.php?t=5784

/// polynomial approximation of the sin function.
/// two variants: 2nd degree positive polynomial (parabola), 4th degree polynomial (square of the same parabola).
/// input range: [-pi, pi]
/// output range: [0, 1]
/// constraints applied: sin(0) = 0, sin(pi / 2) = 1, sin(pi) = 0
/// THD = 3.8% with only odd harmonics (in the accurate variant THD is only 0.078% with Q = 0.775, P = 0.225)
template<unsigned int degree, typename T>
T fast_sin(T const & theta) {
	//assert(-pi <= theta && theta <= pi);
	const float PI     = 3.14159265358979323846264338327950288f;
	const float PI_SQR = 9.86960440108935861883449099987615114f;
	const T B = 4 / PI;
	const T C = -4 / PI_SQR;
	T y = B * theta + C * theta * std::abs(theta);
	if(degree > 2) {
		// Q + P = 1
		// Q = 0.775991821224, P = 0.224008178776 for minimal absolute error (0.0919%)
		// Q = 0.782, P = 0.218 for minimal relative error
		// Q = ?, P = ? for minimal THD error
		#if 1
			const T P = 0.225;
			y = P * (y * std::abs(y) - y) + y;
		#else
			const float Q = 0.775;
			y = Q * y + P * y * std::abs(y);
		#endif
	}
	//assert(-1 <= y && y <= 1);
	return y;
}

#if defined BOOST_AUTO_TEST_CASE
	BOOST_AUTO_TEST_CASE(fast_sin_test) {
		using namespace universalis::operating_system::clocks;
		//typedef thread clock;
		typedef monotonic clock;
		int const iterations(1000000);
		std::nanoseconds const t1(clock::current());
		typedef double real;
		real f1(1);
		for(int i(0); i < iterations; ++i) {
			f1 = std::fmod(f1 + pi, 2 * pi) - pi;
			f1 += fast_sin<2>(f1);
			f1 -= fast_sin<2>(f1);
		}
		std::nanoseconds const t2(clock::current());
		float f2(1);
		for(int i(0); i < iterations; ++i) {
			f2 += std::sin(f2);
			f2 -= std::sin(f2);
		}
		std::nanoseconds const t3(clock::current());
		{
			std::ostringstream s; s << "fast_sin: " << f1;
			BOOST_MESSAGE(s.str());
		}
		{
			std::ostringstream s; s << "std::sin: " << f2;
			BOOST_MESSAGE(s.str());
		}
		{
			std::ostringstream s;
			s << (t2 - t1).get_count() * 1e-9 << "s < " << (t3 - t2).get_count() * 1e-9 << "s";
			BOOST_MESSAGE(s.str());
		}
		BOOST_CHECK(t2 - t1 < t3 - t2);
	}
#endif

}}}
