#pragma once
#include <cmath>
namespace psycle
{
	namespace common
	{
		namespace math
		{
			template<typename Real>
			Real inline remainder(Real const & numerator, Real const & denominator)
			{
				Real result(numerator);
				if(result >= denominator) result -= denominator;
				if(result >= denominator) result = std::fmod(result, denominator);
				return result;
			}
		}
	}
}
