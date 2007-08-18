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

#if 0
	#include <cmath>
	#include <fenv.h>
	BOOST_AUTO_TEST_CASE(lrint_test)
	{
		//int const previous_feround(fesetround(FE_DOWNWARD));
		int const previous_feround(fesetround(FE_TONEAREST));
		//int const previous_feround(fesetround(FE_TOWARDZERO));
		//int const previous_feround(fesetround(FE_UPWARD));
		try {
			BOOST_CHECK(lrint(+1.6) == +2);
			BOOST_CHECK(lrint(+1.4) == +1);
			BOOST_CHECK(lrint(-1.6) == -2);
			BOOST_CHECK(lrint(-1.4) == -1);
			BOOST_CHECK(lrint(+2.6) == +3);
			BOOST_CHECK(lrint(-2.6) == -3);
		} catch(...) {
			fesetround(previous_feround);
			throw;
		}
	}
#endif

#include <psycle/helpers/math/round.hpp>
#include <psycle/helpers/math/truncate.hpp>
