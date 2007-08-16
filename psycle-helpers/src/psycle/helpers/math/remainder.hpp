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
			Real inline remainder(Real const & numerator, Real const & denominator) UNIVERSALIS__COMPILER__CONST
			{
				Real result(numerator);
				if(result >= denominator) result -= denominator;
				if(result >= denominator) result = std::fmod(result, denominator);
				return result;
			}
		}
	}
}
