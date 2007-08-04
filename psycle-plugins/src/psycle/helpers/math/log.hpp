#pragma once
#include <boost/static_assert.hpp>
#include <cstdint>
namespace psycle
{
	namespace common
	{
		namespace math
		{
			float inline log(float const & x)
			{ 
				#if !defined DIVERSALIS__PROCESSOR__X86
					#error please verify this code
				#endif
				BOOST_STATIC_ASSERT((sizeof x == 4));
				//assert(f > 0); 
				std::int32_t const i(*reinterpret_cast<const std::int32_t *>(&x));
				return
					(  (i & 0x7f800000) >> 23 )
					+  (i & 0x007fffff)
					/ float(0x00800000)
					-       0x0000007f;
			}
		}
	}
}
