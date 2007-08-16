#pragma once
#include <universalis/compiler.hpp>
namespace psycle
{
	namespace helpers
	{
		namespace math
		{
			template<typename Real>
			Real inline clipped(Real const & minimum, Real const & value, Real const & maximum) UNIVERSALIS__COMPILER__CONST
			{
				if(minimum > value) return minimum;
				else if(value > maximum) return maximum;
				else return value;
			}
		}
	}
}
