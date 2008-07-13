// -*- mode:c++; indent-tabs-mode:t -*-
// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2008 members of the music-dsp mailing list http://www.music.columbia.edu/cmc/music-dsp and of the psycle project http://psycle.sourceforge.net 

#pragma once
#include <cmath>
namespace psycle { namespace helpers { namespace math {

// worth reading
// http://www.audiomulch.com/~rossb/code/sinusoids/
// http://www.devmaster.net/forums/showthread.php?t=5784

/// polynomial approximation of the sin function.
/// two variants: inaccurate using a 2nd degree positive polynomial (parabola), and accurate using a 4th degree polynomial (square of the same parabola).
/// input range: [-pi, pi]
/// output range: [0, 1]
/// constraints applied: sin(0) = 0, sin(pi / 2) = 1, sin(pi) = 0
/// THD = 3.8% with only odd harmonics (in the accurate variant THD is only 0.078% with Q = 0.775, P = 0.225)
template<typename T, bool accurate = false>
T fast_sin(T const & theta) {
	const float PI     = 3.14159265358979323846264338327950288f;
	const float PI2    = 6.28318530717958647692528676655900577f;
	const float PID2   = 1.57079632679489661923132169163975144f;
	const float PI_SQR = 9.86960440108935861883449099987615114f;
	const T B = 4 / PI;
	const T C = -4 / PI_SQR;
	T y = B * theta + C * theta * std::abs(theta);
	if(accurate) {
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
	return y;
}

}}}
