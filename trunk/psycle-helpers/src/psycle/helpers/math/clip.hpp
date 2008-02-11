// -*- mode:c++; indent-tabs-mode:t -*-
#pragma once
#include <universalis/compiler.hpp>
namespace psycle
{
	namespace helpers
	{
		namespace math
		{
			template<typename Real> UNIVERSALIS__COMPILER__CONST
			Real inline clipped(Real const & minimum, Real const & value, Real const & maximum)
			{
				if(minimum > value) return minimum;
				else if(value > maximum) return maximum;
				else return value;
			}
		}
	}
}
