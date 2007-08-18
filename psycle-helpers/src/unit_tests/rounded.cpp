#define BOOST_AUTO_TEST_MAIN
#include <boost/test/auto_unit_test.hpp>

//BOOST_AUTO_TEST_SUITE(psycle)
//BOOST_AUTO_TEST_SUITE_END

#if 0	
BOOST_AUTO_TEST_CASE(fail)
{
	BOOST_CHECK(1 == 1);
	BOOST_CHECK(1 == 2);
	BOOST_CHECK(1 == 3);
}
#endif

#include <psycle/helpers/math/round.hpp>
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

#include <psycle/helpers/math/truncate.hpp>
BOOST_AUTO_TEST_CASE(truncated)
{
	using psycle::helpers::math::truncated;
	BOOST_CHECK(truncated(+1.6) == +1);
	BOOST_CHECK(truncated(+1.4) == +1);
	BOOST_CHECK(truncated(-1.6) == -2);
	BOOST_CHECK(truncated(-1.4) == -2);
	BOOST_CHECK(truncated(+1.6f) == +1);
	BOOST_CHECK(truncated(+1.4f) == +1);
	BOOST_CHECK(truncated(-1.6f) == -2);
	BOOST_CHECK(truncated(-1.4f) == -2);
}
