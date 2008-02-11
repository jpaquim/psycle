/* -*- mode:c++, indent-tabs-mode:t -*- */
#pragma once
#include <cmath>
namespace psycle
{
	namespace helpers
	{
		namespace math
		{
			/// the pi constant as a 64-bit floating point number
			double const pi
			(
				#if defined M_PI
					M_PI
				#else
					3.14159265358979323846
				#endif
			);

			/// the pi constant as a 32-bit floating point number
			float const pi_f = float(pi);
		}
	}
}
