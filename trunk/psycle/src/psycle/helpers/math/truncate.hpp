#pragma once
#include <boost/static_assert.hpp>
#include <cstdint>
namespace psycle
{
	namespace common
	{
		namespace math
		{
			std::int32_t inline truncated(double x)
			{
				#if !defined DIVERSALIS__PROCESSOR__X86
					#error please verify this code
				#endif
				BOOST_STATIC_ASSERT((sizeof x == 8));
				double const result(x - 0.5 + 6755399441055744ULL); // 2^51 + 2^52
				return *reinterpret_cast<std::int32_t const *>(&result);
			}
		}
	}
}
