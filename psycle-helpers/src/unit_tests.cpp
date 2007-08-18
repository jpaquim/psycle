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
#include <psycle/helpers/math/truncate.hpp>
