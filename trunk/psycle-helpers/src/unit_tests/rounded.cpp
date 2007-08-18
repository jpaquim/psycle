int x_main(int, char*[]);
#if 0
	#include <boost/test/included/prg_exec_monitor.hpp> // doesn't need linking, int cpp_main(int, char*[])
	//#include <boost/test/prg_exec_monitor.hpp> // needs linking , int cpp_main(int, char*[])
	int cpp_main(int argc, char*argv[]) { return x_main(argc, argv); }
	#include <stdexcept>
	#define $(predicate) if(!(predicate)) throw std::runtime_error(#predicate);
#elif 1
	#include <boost/test/minimal.hpp> // doesn't need linking, int test_main(int, char*[])
	int test_main(int argc, char*argv[]) { return x_main(argc, argv); }
	#define $(predicate) BOOST_CHECK(predicate);
#else
	#define BOOST_TEST_MODULE psycle helpers
	#include <boost/test/unit_test.hpp>
	#define $(predicate) BOOST_CHECK(predicate);
#endif

#include <psycle/helpers/math/round.hpp>
#include <psycle/helpers/math/truncate.hpp>

int x_main(int, char*[])
{
	$(1 == 2);
	$(1 == 1);
	$(1 == 3);
	using psycle::helpers::math::rounded;
	$(rounded(+1.6) == +2);
	$(rounded(+1.4) == +1);
	$(rounded(-1.6) == -2);
	$(rounded(-1.4) == -1);
	$(rounded(+1.6f) == +2);
	$(rounded(+1.4f) == +1);
	$(rounded(-1.6f) == -2);
	$(rounded(-1.4f) == -1);
	using psycle::helpers::math::truncated;
	$(truncated(+1.6) == +1);
	$(truncated(+1.4) == +1);
	$(truncated(-1.6) == -2);
	$(truncated(-1.4) == -2);
	$(truncated(+1.6f) == +1);
	$(truncated(+1.4f) == +1);
	$(truncated(-1.6f) == -2);
	$(truncated(-1.4f) == -2);
	return 0;
}
