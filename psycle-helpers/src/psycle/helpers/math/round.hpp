#pragma once
#include <universalis/compiler.hpp>
#include <cmath>
namespace psycle
{
	namespace helpers
	{
		namespace math
		{
			/// converts a floating point number to an integer by rounding to the nearest integer.
			/// note: it is unspecified whether rounding x.5 rounds up or down.
			template<typename Real> UNIVERSALIS__COMPILER__CONST
			int inline rounded(Real const & x)
			{
				return x > 0 ? std::floor(x + Real(0.5)) : std::ceil(x - Real(0.5));
			}
		}
	}
}
