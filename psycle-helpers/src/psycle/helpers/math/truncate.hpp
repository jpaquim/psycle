#pragma once
#include <diversalis/processor.hpp>
#include <universalis/compiler.hpp>
#include <boost/static_assert.hpp>
#include <cstdint>
namespace psycle
{
	namespace helpers
	{
		namespace math
		{
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
					///\todo do we really need to write this in custom asm? wouldn't it be better to rely on the compiler?
					#if 1 // note: this is not as fast as one might expect.
						///\todo specify the rounding mode.. this is not a truncation!
						std::int32_t i;
						double const half(0.5);
						_asm
						{ 
							fld f;
							fsub half;
							fistp i;
						} 
						return i;
					#else
						return truncated(double(f));
					#endif
				#else
					return truncated(double(f));
				#endif
			}
		}
	}
}
