// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2004-2009 members of the psycle project http://psycle.sourceforge.net

#ifndef PSYCLE__HELPERS__MATH__RINT__INCLUDED
#define PSYCLE__HELPERS__MATH__RINT__INCLUDED
#pragma once

#include <universalis.hpp>
#include <cmath>
#include <limits>

namespace psycle { namespace helpers { namespace math {

using namespace universalis::stdlib;

/// C1999 *rint* - converts a floating point number to an integer by rounding in an unspecified way.
/// This function has the same semantic as C1999's *rint* series of functions,
/// but with C++ overload support, we don't need different names for each type.
/// On C1999, the rounding mode may be set with fesetround, but msvc does not support it, so the mode is unspecified.
/// Because the rounding mode is left as is, this is faster than using static_cast (trunc), floor, ceil, or round, which need to change it temporarily.
template<typename IntegralResult, typename Real> UNIVERSALIS__COMPILER__CONST_FUNCTION
IntegralResult inline rint(Real x) {
	// check that the Result type is an integral number
	BOOST_STATIC_ASSERT((std::numeric_limits<IntegralResult>::is_integer));

	return x;
}

#if DIVERSALIS__STDLIB__MATH >= 199901

	// signed long long int

	template<> UNIVERSALIS__COMPILER__CONST_FUNCTION signed long long int inline rint<>(long double ld) { return ::llrintl(ld); }
	template<> UNIVERSALIS__COMPILER__CONST_FUNCTION signed long long int inline rint<>(double d) { return ::llrint(d); }
	template<> UNIVERSALIS__COMPILER__CONST_FUNCTION signed long long int inline rint<>(float f) { return ::llrintf(f); }

	// unsigned long long int

	template<> UNIVERSALIS__COMPILER__CONST_FUNCTION unsigned long long int inline rint<>(long double ld) { return ::llrintl(ld); }
	template<> UNIVERSALIS__COMPILER__CONST_FUNCTION unsigned long long int inline rint<>(double d) { return ::llrint(d); }
	template<> UNIVERSALIS__COMPILER__CONST_FUNCTION unsigned long long int inline rint<>(float f) { return ::llrintf(f); }

	// signed long int

	template<> UNIVERSALIS__COMPILER__CONST_FUNCTION signed long int inline rint<>(long double ld) { return ::lrintl(ld); }
	template<> UNIVERSALIS__COMPILER__CONST_FUNCTION signed long int inline rint<>(double d) { return ::lrint(d); }
	template<> UNIVERSALIS__COMPILER__CONST_FUNCTION signed long int inline rint<>(float f) { return ::lrintf(f); }
	
	// unsigned long int

	template<> UNIVERSALIS__COMPILER__CONST_FUNCTION unsigned long int inline rint<>(long double ld) { return ::lrintl(ld); }
	template<> UNIVERSALIS__COMPILER__CONST_FUNCTION unsigned long int inline rint<>(double d) { return ::lrint(d); }
	template<> UNIVERSALIS__COMPILER__CONST_FUNCTION unsigned long int inline rint<>(float f) { return ::lrintf(f); }

	// signed int

	template<> UNIVERSALIS__COMPILER__CONST_FUNCTION signed int inline rint<>(long double ld) { return ::lrintl(ld); }
	template<> UNIVERSALIS__COMPILER__CONST_FUNCTION signed int inline rint<>(double d) { return ::lrint(d); }
	template<> UNIVERSALIS__COMPILER__CONST_FUNCTION signed int inline rint<>(float f) { return ::lrintf(f); }

	// unsigned int

	template<> UNIVERSALIS__COMPILER__CONST_FUNCTION unsigned int inline rint<>(long double ld) { return ::lrintl(ld); }
	template<> UNIVERSALIS__COMPILER__CONST_FUNCTION unsigned int inline rint<>(double d) { return ::lrint(d); }
	template<> UNIVERSALIS__COMPILER__CONST_FUNCTION unsigned int inline rint<>(float f) { return ::lrintf(f); }

	// signed short

	template<> UNIVERSALIS__COMPILER__CONST_FUNCTION signed short inline rint<>(long double ld) { return ::lrintl(ld); }
	template<> UNIVERSALIS__COMPILER__CONST_FUNCTION signed short inline rint<>(double d) { return ::lrint(d); }
	template<> UNIVERSALIS__COMPILER__CONST_FUNCTION signed short inline rint<>(float f) { return ::lrintf(f); }

	// unsigned short

	template<> UNIVERSALIS__COMPILER__CONST_FUNCTION unsigned short inline rint<>(long double ld) { return ::lrintl(ld); }
	template<> UNIVERSALIS__COMPILER__CONST_FUNCTION unsigned short inline rint<>(double d) { return ::lrint(d); }
	template<> UNIVERSALIS__COMPILER__CONST_FUNCTION unsigned short inline rint<>(float f) { return ::lrintf(f); }

	// signed char

	template<> UNIVERSALIS__COMPILER__CONST_FUNCTION signed char inline rint<>(long double ld) { return ::lrintl(ld); }
	template<> UNIVERSALIS__COMPILER__CONST_FUNCTION signed char inline rint<>(double d) { return ::lrint(d); }
	template<> UNIVERSALIS__COMPILER__CONST_FUNCTION signed char inline rint<>(float f) { return ::lrintf(f); }

	// unsigned char

	template<> UNIVERSALIS__COMPILER__CONST_FUNCTION unsigned char inline rint<>(long double ld) { return ::lrintl(ld); }
	template<> UNIVERSALIS__COMPILER__CONST_FUNCTION unsigned char inline rint<>(double d) { return ::lrint(d); }
	template<> UNIVERSALIS__COMPILER__CONST_FUNCTION unsigned char inline rint<>(float f) { return ::lrintf(f); }

	// char

	template<> UNIVERSALIS__COMPILER__CONST_FUNCTION char inline rint<>(long double ld) { return ::lrintl(ld); }
	template<> UNIVERSALIS__COMPILER__CONST_FUNCTION char inline rint<>(double d) { return ::lrint(d); }
	template<> UNIVERSALIS__COMPILER__CONST_FUNCTION char inline rint<>(float f) { return ::lrintf(f); }
#else

	// int32_t
	
	template<> UNIVERSALIS__COMPILER__CONST_FUNCTION
	int32_t inline rint<>(double d) {
		BOOST_STATIC_ASSERT((sizeof d == 8));
		union result_union
		{
			double d;
			int32_t i;
		} result;
		result.d = d + 6755399441055744.0; // 2^51 + 2^52
		return result.i;
	}

	template<> UNIVERSALIS__COMPILER__CONST_FUNCTION
	int32_t inline rint<>(float f) {
		#if defined DIVERSALIS__CPU__X86 && defined DIVERSALIS__COMPILER__MICROSOFT && defined DIVERSALIS__COMPILER__ASSEMBLER__INTEL// also intel's compiler?
			///\todo not always the fastest when using sse(2)
			///\todo the double "2^51 + 2^52" version might be faster.
			int32_t i;
			__asm
			{ 
				fld f;
				fistp i;
			}
			return i;
		#else
			return static_cast<int32_t>(f);
		#endif
	}

	// uint32_t

	template<> UNIVERSALIS__COMPILER__CONST_FUNCTION uint32_t inline rint<>(double d) { return rint<int32_t>(d); }
	template<> UNIVERSALIS__COMPILER__CONST_FUNCTION uint32_t inline rint<>(float f) { return rint<int32_t>(f); }

	// int16_t

	template<> UNIVERSALIS__COMPILER__CONST_FUNCTION int16_t inline rint<>(double d) { return rint<int32_t>(d); }
	template<> UNIVERSALIS__COMPILER__CONST_FUNCTION int16_t inline rint<>(float f) { return rint<int32_t>(f); }

	// uint16_t

	template<> UNIVERSALIS__COMPILER__CONST_FUNCTION uint16_t inline rint<>(double d) { return rint<int32_t>(d); }
	template<> UNIVERSALIS__COMPILER__CONST_FUNCTION uint16_t inline rint<>(float f) { return rint<int32_t>(f); }

	// int8_t

	template<> UNIVERSALIS__COMPILER__CONST_FUNCTION int8_t inline rint<>(double d) { return rint<int32_t>(d); }
	template<> UNIVERSALIS__COMPILER__CONST_FUNCTION int8_t inline rint<>(float f) { return rint<int32_t>(f); }

	// uint8_t

	template<> UNIVERSALIS__COMPILER__CONST_FUNCTION uint8_t inline rint<>(double d) { return rint<int32_t>(d); }
	template<> UNIVERSALIS__COMPILER__CONST_FUNCTION uint8_t inline rint<>(float f) { return rint<int32_t>(f); }

#endif

}}}

/***************************************************************************************************************/
#ifdef BOOST_AUTO_TEST_CASE
	#if DIVERSALIS__STDLIB__MATH >= 199901 // some test of C1999's features
		#include <fenv.h>
	#endif
	namespace psycle { namespace helpers { namespace math { namespace test {
	
		template<typename Real>
		void rint_test_template() {
			Real r(-1e6);
			while(r < Real(1e6)) {
				int32_t i = rint<int32_t>(r);
				int32_t si = static_cast<int32_t>(r);
				BOOST_CHECK(i == si || i == si + (r > 0 ? +1 : -1));
				r += Real(100.1);
			}
		}
	
		BOOST_AUTO_TEST_CASE(rint_test) {
			rint_test_template<float>();
			rint_test_template<double>();
			#if DIVERSALIS__STDLIB__MATH >= 199901
				rint_test_template<long double>();
			#endif
		}
	
		#if DIVERSALIS__STDLIB__MATH >= 199901 // some test of C1999's features
			BOOST_AUTO_TEST_CASE(rint_c1999_test) {
				int const initial_feround(fegetround());
				try {
					fesetround(FE_TONEAREST);
					BOOST_CHECK(rint<long int>(+1.6) == +2);
					BOOST_CHECK(rint<long int>(+1.4) == +1);
					BOOST_CHECK(rint<long int>(-1.6) == -2);
					BOOST_CHECK(rint<long int>(-1.4) == -1);
					fesetround(FE_TOWARDZERO);
					BOOST_CHECK(rint<long int>(+1.6) == +1);
					BOOST_CHECK(rint<long int>(+1.4) == +1);
					BOOST_CHECK(rint<long int>(-1.6) == -1);
					BOOST_CHECK(rint<long int>(-1.4) == -1);
					fesetround(FE_DOWNWARD);
					BOOST_CHECK(rint<long int>(+1.6) == +1);
					BOOST_CHECK(rint<long int>(+1.4) == +1);
					BOOST_CHECK(rint<long int>(-1.6) == -2);
					BOOST_CHECK(rint<long int>(-1.4) == -2);
					fesetround(FE_UPWARD);
					BOOST_CHECK(rint<long int>(+1.6) == +2);
					BOOST_CHECK(rint<long int>(+1.4) == +2);
					BOOST_CHECK(rint<long int>(-1.6) == -1);
					BOOST_CHECK(rint<long int>(-1.4) == -1);
				} catch(...) {
					fesetround(initial_feround);
					throw;
				}
				fesetround(initial_feround);
			}
		#endif
	}}}}
#endif

#endif
