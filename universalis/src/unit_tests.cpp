#include <packageneric/pre-compiled.private.hpp>

#define BOOST_LIB_NAME boost_unit_test_framework
#if BOOST_VERSION >= 103400
	#define BOOST_DYN_LINK
#endif
#include <boost/config/auto_link.hpp>

#define BOOST_TEST_DYN_LINK
#define BOOST_AUTO_TEST_MAIN
#include <boost/test/auto_unit_test.hpp>

#if 0 // some dummy test case that always fail (to test failure)
	BOOST_AUTO_TEST_CASE(dummy_failure)
	{
		BOOST_CHECK(false);
	}
#else
	BOOST_AUTO_TEST_CASE(dummy_success)
	{
		BOOST_CHECK(true);
	}
#endif

