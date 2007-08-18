#pragma once
#include <universalis/compiler.hpp>
#include <cmath>
namespace psycle { namespace helpers { namespace math {

	/// converts a floating point number to an integer by rounding to the nearest integer.
	/// note: it is unspecified whether rounding x.5 rounds up or down.
	template<typename Real> UNIVERSALIS__COMPILER__CONST
	int inline rounded(Real const & x)
	{
		return x > 0 ? std::floor(x + Real(0.5)) : std::ceil(x - Real(0.5));
	}
}}}

#if defined BOOST_AUTO_TEST_CASE
	BOOST_AUTO_TEST_CASE(rounded)
	{
		using psycle::helpers::math::rounded;
		BOOST_CHECK(rounded(+1.6) == +2);
		BOOST_CHECK(rounded(+1.4) == +1);
		BOOST_CHECK(rounded(-1.6) == -2);
		BOOST_CHECK(rounded(-1.4) == -1);
		BOOST_CHECK(rounded(+1.6f) == +2);
		BOOST_CHECK(rounded(+1.4f) == +1);
		BOOST_CHECK(rounded(-1.6f) == -2);
		BOOST_CHECK(rounded(-1.4f) == -1);
	}
#endif
