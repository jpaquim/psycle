#include <packageneric/pre-compiled.private.hpp>

#define BOOST_LIB_NAME boost_unit_test_framework
#include <boost/config/auto_link.hpp>

#define BOOST_AUTO_TEST_MAIN
#include <boost/test/auto_unit_test.hpp>

//BOOST_AUTO_TEST_SUITE(psycle)

#include <psycle/helpers/helpers.hpp>
#include <psycle/helpers/dsp.hpp>
#include <psycle/helpers/math/round.hpp>
#include <psycle/helpers/math/truncate.hpp>
#include <psycle/helpers/math/log.hpp>

#if __STDC_VERSION__ >= 199901 // some test of C1999's features
	#include <cmath>
	#include <fenv.h>
	BOOST_AUTO_TEST_CASE(lrint_test)
	{
		int const initial_feround(fegetround());
		try {
			fesetround(FE_TONEAREST);
			BOOST_CHECK(lrint(+2.6) == +3);
			BOOST_CHECK(lrint(+1.4) == +1);
			BOOST_CHECK(lrint(-2.6) == -3);
			BOOST_CHECK(lrint(-1.4) == -1);
			fesetround(FE_TOWARDZERO);
			BOOST_CHECK(lrint(+1.6) == +1);
			BOOST_CHECK(lrint(+1.4) == +1);
			BOOST_CHECK(lrint(-1.6) == -1);
			BOOST_CHECK(lrint(-1.4) == -1);
			fesetround(FE_DOWNWARD);
			BOOST_CHECK(lrint(+1.6) == +1);
			BOOST_CHECK(lrint(+1.4) == +1);
			BOOST_CHECK(lrint(-1.6) == -2);
			BOOST_CHECK(lrint(-1.4) == -2);
			fesetround(FE_UPWARD);
			BOOST_CHECK(lrint(+1.6) == +2);
			BOOST_CHECK(lrint(+1.4) == +2);
			BOOST_CHECK(lrint(-1.6) == -1);
			BOOST_CHECK(lrint(-1.4) == -1);
		} catch(...) {
			fesetround(initial_feround);
			throw;
		}
	}
#endif

#if 0 // some dummy test case that always fail (to test failure)
	BOOST_AUTO_TEST_CASE(dummy_failure)
	{
		BOOST_CHECK(false);
	}
#endif

//BOOST_AUTO_TEST_SUITE_END
