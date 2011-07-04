// -*- mode:c++; indent-tabs-mode:t -*-


///\todo This file is not up-to-date and should be replaced with the common psycle/helpers/math/pi.hpp



#pragma once
#include <cmath>
namespace psy
{
	namespace common
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
