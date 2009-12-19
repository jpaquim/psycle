// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2004-2009 members of the psycle project http://psycle.sourceforge.net

#ifndef PSYCLE__HELPERS__MATH__LRINT__INCLUDED
#define PSYCLE__HELPERS__MATH__LRINT__INCLUDED
#pragma once

#include <diversalis/stdlib.hpp>
#if DIVERSALIS__STDLIB__MATH < 199901
	#include <diversalis/compiler.hpp>
	#include <diversalis/cpu.hpp>
	#include <boost/static_assert.hpp>
#endif
#include <universalis/compiler.hpp>
#include <universalis/stdlib/cstdint.hpp>
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
	
	template<> UNIVERSALIS__COMPILER__CONST
	long long int inline lrint<>(long double ld) {
		return ::llrintl(ld);
	}

	template<> UNIVERSALIS__COMPILER__CONST
	long long int inline lrint<>(double d) {
		return ::llrint(d);
	}

	template<> UNIVERSALIS__COMPILER__CONST
	long long int inline lrint<>(float f) {
		return ::llrintf(f);
	}

	template<> UNIVERSALIS__COMPILER__CONST
	long int inline lrint<>(long double ld) {
		return ::lrintl(ld);
	}

	template<> UNIVERSALIS__COMPILER__CONST
	long int inline lrint<>(double d) {
		return ::lrint(d);
	}

	template<> UNIVERSALIS__COMPILER__CONST
	long int inline lrint<>(float f) {
		return ::lrintf(f);
	}
	
	template<> UNIVERSALIS__COMPILER__CONST
	int inline lrint<>(long double ld) {
		return ::lrintl(ld);
	}

	template<> UNIVERSALIS__COMPILER__CONST
	int inline lrint<>(double d) {
		return ::lrint(d);
	}

	template<> UNIVERSALIS__COMPILER__CONST
	int inline lrint<>(float f) {
		return ::lrintf(f);
	}
	
#else

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
		#if defined DIVERSALIS__CPU__X86 && defined DIVERSALIS__COMPILER__MICROSOFT // also intel's compiler?
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
			return lrint<int32_t>(double(f));
		#endif
	}

#endif

#if defined BOOST_AUTO_TEST_CASE && DIVERSALIS__STDLIB__MATH >= 199901 // some test of C1999's features
	#include <cmath>
	#include <fenv.h>
	BOOST_AUTO_TEST_CASE(lrint_test) {
		int const initial_feround(fegetround());
		try {
			fesetround(FE_TONEAREST);
			BOOST_CHECK(lrint<long int>(+2.6) == +3);
			BOOST_CHECK(lrint<long int>(+1.4) == +1);
			BOOST_CHECK(lrint<long int>(-2.6) == -3);
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

}}}

#endif
