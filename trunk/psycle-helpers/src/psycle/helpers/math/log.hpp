#pragma once
#include <diversalis/processor.hpp>
#include <universalis/compiler.hpp>
#if defined DIVERSALIS__PROCESSOR__X86 // we should verify the code for other architectures.
	#include <boost/static_assert.hpp>
	#include <cstdint>
#else
	#include <cmath>
#endif
#if defined BOOST_AUTO_TEST_CASE
	#include <universalis/operating_system/clocks.hpp>
	#include <cmath>
	#include <sstream>
#endif
namespace psycle
{
	namespace helpers
	{
		namespace math
		{
			///\todo doc
			float inline UNIVERSALIS__COMPILER__CONST log2(float f)
			{
				#if !defined DIVERSALIS__PROCESSOR__X86 // we should verify the code for other architectures.
					return std::log(f);
				#endif
				BOOST_STATIC_ASSERT((sizeof f == 4));
				//assert(f > 0); 
				union result_union {
					float f;
					std::uint32_t i;
				} result;
				result.f = f;
				return
					(  (result.i & 0x7f800000) >> 23 )
					+  (result.i & 0x007fffff)
					/     float(0x00800000)
					-           0x0000007f;
			}

			#if defined BOOST_AUTO_TEST_CASE
				BOOST_AUTO_TEST_CASE(log2_test)
				{
					using namespace universalis::operating_system::clocks;
					typedef thread clock;
					int const iterations(1000000);
					opaque_time const t1(clock::current());
					{
						float f(1);
						for(int i(0); i < iterations; ++i) f *= log2(f);
					}
					opaque_time const t2(clock::current());
					{
						float f(1);
						for(int i(0); i < iterations; ++i) f *= std::log(f);
					}
					opaque_time const t3(clock::current());
					std::ostringstream s;
					s << (t3 - t2).to_real_time() << " < " << (t2 - t1).to_real_time();
					BOOST_MESSAGE(s.str());
					BOOST_CHECK(t3 - t2 < t2 - t1);
				}
			#endif
		}
	}
}
