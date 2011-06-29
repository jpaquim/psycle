// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2004-2009 members of the psycle project http://psycle.sourceforge.net

#ifndef PSYCLE__HELPERS__MATH__LRINT__INCLUDED
#define PSYCLE__HELPERS__MATH__LRINT__INCLUDED
#pragma once

#include <universalis.hpp>
#include <cmath>

namespace psycle { namespace helpers { namespace math {

/// C1999 lrint - converts a floating point number to an integer by rounding in an unspecified way.
/// This function has the same semantic as C1999's lrint series of functions,
/// but with C++ overload support, we don't need different names for each type.
/// On C1999, the rounding mode may be set with fesetround, but msvc does not support it, so the mode is unspecified.
template<typename Integer, typename Real> UNIVERSALIS__COMPILER__CONST
Integer inline lrint(Real x) {
	return x;
}

#if DIVERSALIS__STDLIB__MATH >= 199901

	// signed long long int

	template<> UNIVERSALIS__COMPILER__CONST signed long long int inline lrint<>(long double ld) { return ::llrintl(ld); }
	template<> UNIVERSALIS__COMPILER__CONST signed long long int inline lrint<>(double d) { return ::llrint(d); }
	template<> UNIVERSALIS__COMPILER__CONST signed long long int inline lrint<>(float f) { return ::llrintf(f); }

	// unsigned long long int

	template<> UNIVERSALIS__COMPILER__CONST unsigned long long int inline lrint<>(long double ld) { return ::llrintl(ld); }
	template<> UNIVERSALIS__COMPILER__CONST unsigned long long int inline lrint<>(double d) { return ::llrint(d); }
	template<> UNIVERSALIS__COMPILER__CONST unsigned long long int inline lrint<>(float f) { return ::llrintf(f); }

	// signed long int

	template<> UNIVERSALIS__COMPILER__CONST signed long int inline lrint<>(long double ld) { return ::lrintl(ld); }
	template<> UNIVERSALIS__COMPILER__CONST signed long int inline lrint<>(double d) { return ::lrint(d); }
	template<> UNIVERSALIS__COMPILER__CONST signed long int inline lrint<>(float f) { return ::lrintf(f); }
	
	// unsigned long int

	template<> UNIVERSALIS__COMPILER__CONST unsigned long int inline lrint<>(long double ld) { return ::lrintl(ld); }
	template<> UNIVERSALIS__COMPILER__CONST unsigned long int inline lrint<>(double d) { return ::lrint(d); }
	template<> UNIVERSALIS__COMPILER__CONST unsigned long int inline lrint<>(float f) { return ::lrintf(f); }

	// signed int

	template<> UNIVERSALIS__COMPILER__CONST signed int inline lrint<>(long double ld) { return ::lrintl(ld); }
	template<> UNIVERSALIS__COMPILER__CONST signed int inline lrint<>(double d) { return ::lrint(d); }
	template<> UNIVERSALIS__COMPILER__CONST signed int inline lrint<>(float f) { return ::lrintf(f); }

	// unsigned int

	template<> UNIVERSALIS__COMPILER__CONST unsigned int inline lrint<>(long double ld) { return ::lrintl(ld); }
	template<> UNIVERSALIS__COMPILER__CONST unsigned int inline lrint<>(double d) { return ::lrint(d); }
	template<> UNIVERSALIS__COMPILER__CONST unsigned int inline lrint<>(float f) { return ::lrintf(f); }

	// signed short

	template<> UNIVERSALIS__COMPILER__CONST signed short inline lrint<>(long double ld) { return ::lrintl(ld); }
	template<> UNIVERSALIS__COMPILER__CONST signed short inline lrint<>(double d) { return ::lrint(d); }
	template<> UNIVERSALIS__COMPILER__CONST signed short inline lrint<>(float f) { return ::lrintf(f); }

	// unsigned short

	template<> UNIVERSALIS__COMPILER__CONST unsigned short inline lrint<>(long double ld) { return ::lrintl(ld); }
	template<> UNIVERSALIS__COMPILER__CONST unsigned short inline lrint<>(double d) { return ::lrint(d); }
	template<> UNIVERSALIS__COMPILER__CONST unsigned short inline lrint<>(float f) { return ::lrintf(f); }

	// signed char

	template<> UNIVERSALIS__COMPILER__CONST signed char inline lrint<>(long double ld) { return ::lrintl(ld); }
	template<> UNIVERSALIS__COMPILER__CONST signed char inline lrint<>(double d) { return ::lrint(d); }
	template<> UNIVERSALIS__COMPILER__CONST signed char inline lrint<>(float f) { return ::lrintf(f); }

	// unsigned char

	template<> UNIVERSALIS__COMPILER__CONST unsigned char inline lrint<>(long double ld) { return ::lrintl(ld); }
	template<> UNIVERSALIS__COMPILER__CONST unsigned char inline lrint<>(double d) { return ::lrint(d); }
	template<> UNIVERSALIS__COMPILER__CONST unsigned char inline lrint<>(float f) { return ::lrintf(f); }

	// char

	template<> UNIVERSALIS__COMPILER__CONST char inline lrint<>(long double ld) { return ::lrintl(ld); }
	template<> UNIVERSALIS__COMPILER__CONST char inline lrint<>(double d) { return ::lrint(d); }
	template<> UNIVERSALIS__COMPILER__CONST char inline lrint<>(float f) { return ::lrintf(f); }
#else

	// int32_t
	
	template<> UNIVERSALIS__COMPILER__CONST
	int32_t inline lrint<>(double d) {
		BOOST_STATIC_ASSERT((sizeof d == 8));
		union result_union
		{
			double d;
			int32_t i;
		} result;
		result.d = d + 6755399441055744.0; // 2^51 + 2^52
		return result.i;
	}

	template<> UNIVERSALIS__COMPILER__CONST
	int32_t inline lrint<>(float f) {
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

	template<> UNIVERSALIS__COMPILER__CONST uint32_t inline lrint<>(double d) { return lrint<int32_t>(d); }
	template<> UNIVERSALIS__COMPILER__CONST uint32_t inline lrint<>(float f) { return lrint<int32_t>(f); }

	// int16_t

	template<> UNIVERSALIS__COMPILER__CONST int16_t inline lrint<>(double d) { return lrint<int32_t>(d); }
	template<> UNIVERSALIS__COMPILER__CONST int16_t inline lrint<>(float f) { return lrint<int32_t>(f); }

	// uint16_t

	template<> UNIVERSALIS__COMPILER__CONST uint16_t inline lrint<>(double d) { return lrint<int32_t>(d); }
	template<> UNIVERSALIS__COMPILER__CONST uint16_t inline lrint<>(float f) { return lrint<int32_t>(f); }

	// int8_t

	template<> UNIVERSALIS__COMPILER__CONST int8_t inline lrint<>(double d) { return lrint<int32_t>(d); }
	template<> UNIVERSALIS__COMPILER__CONST int8_t inline lrint<>(float f) { return lrint<int32_t>(f); }

	// uint8_t

	template<> UNIVERSALIS__COMPILER__CONST uint8_t inline lrint<>(double d) { return lrint<int32_t>(d); }
	template<> UNIVERSALIS__COMPILER__CONST uint8_t inline lrint<>(float f) { return lrint<int32_t>(f); }

#endif

}}}

/***************************************************************************************************************/
#ifdef BOOST_AUTO_TEST_CASE
	#if DIVERSALIS__STDLIB__MATH >= 199901 // some test of C1999's features
		#include <fenv.h>
	#endif
	namespace psycle { namespace helpers { namespace math { namespace test {
	
		template<typename Real>
		void lrint_test_template() {
			Real r(-1e6);
			while(r < Real(1e6)) {
				int32_t i = lrint<int32_t>(r);
				int32_t si = static_cast<int32_t>(r);
				BOOST_CHECK(i == si || i == si + (r > 0 ? +1 : -1));
				r += Real(100.1);
			}
		}
	
		BOOST_AUTO_TEST_CASE(lrint_test) {
			lrint_test_template<float>();
			lrint_test_template<double>();
			#if DIVERSALIS__STDLIB__MATH >= 199901
				lrint_test_template<long double>();
			#endif
		}
	
		#if DIVERSALIS__STDLIB__MATH >= 199901 // some test of C1999's features
			BOOST_AUTO_TEST_CASE(lrint_c1999_test) {
				int const initial_feround(fegetround());
				try {
					fesetround(FE_TONEAREST);
					BOOST_CHECK(lrint<long int>(+1.6) == +2);
					BOOST_CHECK(lrint<long int>(+1.4) == +1);
					BOOST_CHECK(lrint<long int>(-1.6) == -2);
					BOOST_CHECK(lrint<long int>(-1.4) == -1);
					fesetround(FE_TOWARDZERO);
					BOOST_CHECK(lrint<long int>(+1.6) == +1);
					BOOST_CHECK(lrint<long int>(+1.4) == +1);
					BOOST_CHECK(lrint<long int>(-1.6) == -1);
					BOOST_CHECK(lrint<long int>(-1.4) == -1);
					fesetround(FE_DOWNWARD);
					BOOST_CHECK(lrint<long int>(+1.6) == +1);
					BOOST_CHECK(lrint<long int>(+1.4) == +1);
					BOOST_CHECK(lrint<long int>(-1.6) == -2);
					BOOST_CHECK(lrint<long int>(-1.4) == -2);
					fesetround(FE_UPWARD);
					BOOST_CHECK(lrint<long int>(+1.6) == +2);
					BOOST_CHECK(lrint<long int>(+1.4) == +2);
					BOOST_CHECK(lrint<long int>(-1.6) == -1);
					BOOST_CHECK(lrint<long int>(-1.4) == -1);
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
