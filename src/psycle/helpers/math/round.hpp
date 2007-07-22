#pragma once
#include <cmath>
namespace psycle
{
	namespace common
	{
		namespace math
		{
			template<typename Real>
			int inline rounded(Real const & x)
			{
				return x > 0 ? std::floor(x + Real(0.5)) : std::ceil(x - Real(0.5));
			}
		}
	}
}
