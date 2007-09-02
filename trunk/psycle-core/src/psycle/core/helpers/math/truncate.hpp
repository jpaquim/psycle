#pragma once
#include <diversalis/processor.hpp>
#include <universalis/compiler.hpp>
#include <boost/static_assert.hpp>
#include <cstdint>
namespace psycle { namespace helpers { namespace math {

	/// converts a floating point number to an integer by truncating toward -infinity
	std::int32_t inline UNIVERSALIS__COMPILER__CONST
	truncated(double d)
	{
		BOOST_STATIC_ASSERT((sizeof d == 8));
		union result_union
		{
			double d;
			std::int32_t i;
		} result;
		result.d = d - 0.5 + 6755399441055744.0; // 2^51 + 2^52
		return result.i;
	}

	/// converts a floating point number to an integer by truncating toward -infinity
	std::int32_t inline UNIVERSALIS__COMPILER__CONST
	truncated(float f)
	{
		#if defined DIVERSALIS__PROCESSOR__X86 && defined DIVERSALIS__COMPILER__MICROSOFT // also intel's compiler?
			///\todo not always the fastest when using sse(2)
			///\todo we can also use C1999's lrint if available
			///\todo this custom asm is not very fast on some arch, the double "2^51 + 2^52" version might be faster
			///\todo specify the rounding mode.. is this really a truncation toward -infinity, even with negative numbers?
			std::int32_t i;
			double const half(0.5);
			__asm
			{ 
				fld f;
				fsub half;
				fistp i;
			}
			return i;
		#else
			return truncated(double(f));
		#endif
	}
	
	#if defined BOOST_AUTO_TEST_CASE
		BOOST_AUTO_TEST_CASE(truncated_test)
		{
			BOOST_CHECK(truncated(+1.6) == +1);
			BOOST_CHECK(truncated(+1.4) == +1);
			BOOST_CHECK(truncated(-1.6) == -2);
			BOOST_CHECK(truncated(-1.4) == -2);
			BOOST_CHECK(truncated(+1.6f) == +1);
			BOOST_CHECK(truncated(+1.4f) == +1);
			BOOST_CHECK(truncated(-1.6f) == -2);
			BOOST_CHECK(truncated(-1.4f) == -2);
		}
	#endif
}}}
