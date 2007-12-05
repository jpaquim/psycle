#include <packageneric/pre-compiled.private.hpp>

#define BOOST_LIB_NAME boost_unit_test_framework
#if BOOST_VERSION >= 103400
	#define BOOST_DYN_LINK
#endif
#include <boost/config/auto_link.hpp>

#define BOOST_TEST_DYN_LINK
#define BOOST_AUTO_TEST_MAIN
#include <boost/test/auto_unit_test.hpp>

#include <psycle/helpers/helpers.hpp>
#include <psycle/helpers/dsp.hpp>
#include <psycle/helpers/math/round.hpp>
#include <psycle/helpers/math/truncate.hpp>
#include <psycle/helpers/math/log.hpp>

