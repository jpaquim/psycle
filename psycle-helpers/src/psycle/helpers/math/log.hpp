#pragma once
#include <diversalis/processor.hpp>
#include <boost/static_assert.hpp>
#include <cstdint>
namespace psycle
{
	namespace helpers
	{
		namespace math
		{
			///\todo doc
			float inline log2(float f)
			{ 
				#if !defined DIVERSALIS__PROCESSOR__X86
					#error please verify this code
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
		}
	}
}
