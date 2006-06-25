#pragma once
namespace psycle
{
	namespace common
	{
		namespace math
		{
			template<typename Real>
			Real inline clipped(Real const & minimum, Real const & value, Real const & maximum)
			{
				if(minimum > value) return minimum;
				else if(value > maximum) return maximum;
				else return value;
			}
		}
	}
}
