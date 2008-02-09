/* -*- mode:c++, indent-tabs-mode:t -*- */
#pragma once
#include <universalis/compiler.hpp>
#include <cmath>
#include <cstdint>
#include "fast_unspecified_round_to_integer.hpp"
namespace psy { namespace common { namespace math {

	/// converts a floating point number to an integer by rounding to the nearest integer.
	/// note: it is unspecified whether rounding x.5 rounds up or down.
	template<typename Real> UNIVERSALIS__COMPILER__CONST
	std::int32_t inline rounded(Real x)
	{
		return x > 0 ? std::floor(x + Real(0.5)) : std::ceil(x - Real(0.5));
	}
}}}

// inline implementation
namespace psy { namespace common { namespace math {
	
	#if __STDC__VERSION__ >= 199901 || \
		(defined DIVERSALIS__COMPILER__GNU && DIVERSALIS__COMPILER__VERSION__MAJOR >= 4)
		
		template<> UNIVERSALIS__COMPILER__CONST
		std::int32_t inline rounded<>(long double ld)
		{
			return fast_unspecified_round_to_integer<std::int32_t>(::roundl(ld));
		}

		template<> UNIVERSALIS__COMPILER__CONST
		std::int32_t inline rounded<>(double d)
		{
			return fast_unspecified_round_to_integer<std::int32_t>(::round(d));
		}

		template<> UNIVERSALIS__COMPILER__CONST
		std::int32_t inline rounded<>(float f)
		{
			return fast_unspecified_round_to_integer<std::int32_t>(::roundf(f));
		}

	#else
	
		template<> UNIVERSALIS__COMPILER__CONST
		std::int32_t inline rounded<>(double d)
		{
			BOOST_STATIC_ASSERT((sizeof d == 8));
			union result_union
			{
				double d;
				std::int32_t i;
			} result;
			result.d = d + 6755399441055744.0; // 2^51 + 2^52
			return result.i;
		}
		
		template<> UNIVERSALIS__COMPILER__CONST
		std::int32_t inline rounded<>(float f)
		{
			#if defined DIVERSALIS__PROCESSOR__X86 && defined DIVERSALIS__COMPILER__MICROSOFT // also intel's compiler?
				///\todo not always the fastest when using sse(2)
				///\todo the double "2^51 + 2^52" version might be faster.
				///\todo the rounding mode is UNSPECIFIED! (potential bug some code changes the FPU's rounding mode)...
				std::int32_t i;
				__asm
				{ 
					fld f;
					fistp i;
				}
				return i;
			#else
				return rounded(double(f));
			#endif
		}

	#endif

	#if defined BOOST_AUTO_TEST_CASE
		BOOST_AUTO_TEST_CASE(rounded_test)
		{
			BOOST_CHECK(rounded(+2.6) == +3);
			BOOST_CHECK(rounded(+1.4) == +1);
			BOOST_CHECK(rounded(-2.6) == -3);
			BOOST_CHECK(rounded(-1.4) == -1);
			BOOST_CHECK(rounded(+2.6f) == +3);
			BOOST_CHECK(rounded(+1.4f) == +1);
			BOOST_CHECK(rounded(-2.6f) == -3);
			BOOST_CHECK(rounded(-1.4f) == -1);
		}
	#endif
}}}
