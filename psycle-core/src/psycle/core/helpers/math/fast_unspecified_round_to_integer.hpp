#pragma once
#include <diversalis/processor.hpp>
#include <universalis/compiler.hpp>
#include <boost/static_assert.hpp>
#include <cstdint>
namespace psy { namespace common { namespace math {

	/// converts a floating point number to an integer by rounding in an unspecified way
	template<typename Integer, typename Real> UNIVERSALIS__COMPILER__CONST
	Integer inline fast_unspecified_round_to_integer(Real x)
	{
		return x;
	}
}}}

// inline implementation
namespace psy { namespace common { namespace math {
	
	#if __STDC__VERSION__ >= 199901 || \
		(defined DIVERSALIS__COMPILER__GNU && DIVERSALIS__COMPILER__VERSION__MAJOR >= 4)
		
		template<> UNIVERSALIS__COMPILER__CONST
		long long int inline fast_unspecified_round_to_integer<>(long double ld)
		{
			return ::llrintl(ld);
		}

		template<> UNIVERSALIS__COMPILER__CONST
		long long int inline fast_unspecified_round_to_integer<>(double d)
		{
			return ::llrint(d);
		}

		template<> UNIVERSALIS__COMPILER__CONST
		long long int inline fast_unspecified_round_to_integer<>(float f)
		{
			return ::llrintf(f);
		}

		template<> UNIVERSALIS__COMPILER__CONST
		long int inline fast_unspecified_round_to_integer<>(long double ld)
		{
			return ::lrintl(ld);
		}

		template<> UNIVERSALIS__COMPILER__CONST
		long int inline fast_unspecified_round_to_integer<>(double d)
		{
			return ::lrint(d);
		}

		template<> UNIVERSALIS__COMPILER__CONST
		long int inline fast_unspecified_round_to_integer<>(float f)
		{
			return ::lrintf(f);
		}
		
		template<> UNIVERSALIS__COMPILER__CONST
		int inline fast_unspecified_round_to_integer<>(long double ld)
		{
			return ::lrintl(ld);
		}

		template<> UNIVERSALIS__COMPILER__CONST
		int inline fast_unspecified_round_to_integer<>(double d)
		{
			return ::lrint(d);
		}

		template<> UNIVERSALIS__COMPILER__CONST
		int inline fast_unspecified_round_to_integer<>(float f)
		{
			return ::lrintf(f);
		}
		
	#else
	
		template<> UNIVERSALIS__COMPILER__CONST
		std::int32_t inline fast_unspecified_round_to_integer<>(double d)
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
		std::int32_t inline fast_unspecified_round_to_integer<>(float f)
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
				return fast_unspecified_round_to_integer<std::int32_t>(double(f));
			#endif
		}

	#endif
}}}
