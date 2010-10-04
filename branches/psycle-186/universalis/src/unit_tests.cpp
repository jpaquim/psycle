#define BOOST_LIB_NAME boost_unit_test_framework
#define BOOST_DYN_LINK
#include <boost/config/auto_link.hpp>

#include <boost/version.hpp>
#define BOOST_TEST_DYN_LINK
#define BOOST_AUTO_TEST_MAIN
#include <boost/test/auto_unit_test.hpp>

#define UNIVERSALIS__OS__LOGGERS__LEVELS__COMPILED_THRESHOLD trace

#include <universalis/stdlib/date_time.hpp>
#include <universalis/stdlib/thread.hpp>
#include <universalis/stdlib/mutex.hpp>
#include <universalis/stdlib/condition.hpp>
#include <universalis/os/sched.hpp>
#include <universalis/os/aligned_memory_alloc.hpp>
