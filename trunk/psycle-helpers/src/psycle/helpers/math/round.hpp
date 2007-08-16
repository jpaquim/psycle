#pragma once
#include <universalis/compiler.hpp>
#include <cmath>
namespace psycle
{
	namespace helpers
	{
		namespace math
		{
			template<typename Real>
			int inline rounded(Real const & x) UNIVERSALIS__COMPILER__CONST
			{
				return x > 0 ? std::floor(x + Real(0.5)) : std::ceil(x - Real(0.5));
			}
		}
	}
}
