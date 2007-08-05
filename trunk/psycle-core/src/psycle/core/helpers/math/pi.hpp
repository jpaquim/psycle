

///\todo This file is not up-to-date and should be replaced with the common psycle/helpers/math/pi.hpp



#pragma once
#include <cmath>
namespace psy
{
	namespace common
	{
		namespace math
		{
			double const pi
			(
				#if defined M_PI
					M_PI
				#else
					3.14159265358979323846
				#endif
			);
		}
	}
}
