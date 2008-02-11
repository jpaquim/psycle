/* -*- mode:c++, indent-tabs-mode:t -*- */
#pragma once
#include <universalis/compiler.hpp>
#include <cmath>
namespace psycle
{
	namespace helpers
	{
		namespace math
		{
			template<typename Real> UNIVERSALIS__COMPILER__CONST
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
