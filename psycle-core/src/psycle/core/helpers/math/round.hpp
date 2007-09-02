#pragma once
#include <universalis/compiler.hpp>
#include <cmath>
#include <cstdint>
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
		std::int32_t inline rounded<>(long double x)
		{
			return ::roundl(x);
		}

		template<> UNIVERSALIS__COMPILER__CONST
		std::int32_t inline rounded<>(double x)
		{
			return ::round(x);
		}

		template<> UNIVERSALIS__COMPILER__CONST
		std::int32_t inline rounded<>(float x)
		{
			return ::roundf(x);
		}

	#elif defined DIVERSALIS__PROCESSOR__X86 && defined DIVERSALIS__COMPILER__MICROSOFT // also intel's compiler?
	
		template<> UNIVERSALIS__COMPILER__CONST
		std::int32_t inline rounded<>(float x)
		{
			///\todo not always the fastest when using sse(2)
			///\todo we can also use C1999's lrint if available
			///\todo this custom asm is not very fast on some arch, the double "2^51 + 2^52" version might be faster
			///\todo verify the rounding mode..
			std::int32_t i;
			__asm
			{ 
				fld x;
				fistp i;
			}
			return i;
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
