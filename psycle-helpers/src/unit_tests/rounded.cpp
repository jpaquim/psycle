#include <psycle/helpers/math/round.hpp>
#include <psycle/helpers/math/truncate.hpp>
#include <boost/test/included/prg_exec_monitor.hpp>
#include <stdexcept>
int cpp_main(int, char*[])
{
	#define $(call, result) if(call != result) throw std::runtime_error(#call);
	using psycle::helpers::math::rounded;
	$(rounded(+1.6), +2)
	$(rounded(+1.4), +1)
	$(rounded(-1.6), -2)
	$(rounded(-1.4), -1)
	$(rounded(+1.6f), +2)
	$(rounded(+1.4f), +1)
	$(rounded(-1.6f), -2)
	$(rounded(-1.4f), -1)
	using psycle::helpers::math::truncated;
	$(truncated(+1.6), +1)
	$(truncated(+1.4), +1)
	$(truncated(-1.6), -2)
	$(truncated(-1.4), -2)
	$(truncated(+1.6f), +1)
	$(truncated(+1.4f), +1)
	$(truncated(-1.6f), -2)
	$(truncated(-1.4f), -2)
	#undef $
	return 0;
}
