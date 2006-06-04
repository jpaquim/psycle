#pragma once
#include <cmath>
namespace psycle
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
