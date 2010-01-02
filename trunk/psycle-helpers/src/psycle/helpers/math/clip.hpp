// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2004-2009 members of the psycle project http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>

#ifndef PSYCLE__HELPERS__MATH__CLIP__INCLUDED
#define PSYCLE__HELPERS__MATH__CLIP__INCLUDED
#pragma once

#include "lrint.hpp"
#include <universalis/compiler.hpp>
#if defined DIVERSALIS__CPU__X86__SSE && defined DIVERSALIS__COMPILER__FEATURE__XMM_INTRINSICS 
	#include <xmmintrin.h>
	#include <emmintrin.h>
#endif
#include <limits>
#include <boost/static_assert.hpp>
namespace psycle { namespace helpers { namespace math {

/// ensures a value stays between two bounds
template<typename X> UNIVERSALIS__COMPILER__CONST
X inline clipped(X const & minimum, X const & value, X const & maximum) {
	// it looks a bit dumb to write a function to do that code,
	// but maybe someone will find an optimised way to do this.
	return value < minimum ? minimum : value > maximum ? maximum : value;
}

/// combines float to signed integer conversion with clipping.
template<typename SignedIntegralResult, const unsigned int bits, typename Real> UNIVERSALIS__COMPILER__CONST
SignedIntegralResult inline clipped_lrint(Real x) {
	// check that Result is signed
	BOOST_STATIC_ASSERT((std::numeric_limits<SignedIntegralResult>::is_signed));
	BOOST_STATIC_ASSERT((std::numeric_limits<SignedIntegralResult>::is_integer));

	int const max((1u << (bits - 1)) - 1); // The compiler is able to compute this statically.
	int const min(-max - 1);
	return lrint<SignedIntegralResult>(clipped(Real(min), x, Real(max)));
}

/// combines float to signed integer conversion with clipping.
template<typename SignedIntegralResult, typename Real> UNIVERSALIS__COMPILER__CONST
SignedIntegralResult inline clipped_lrint(Real x) {
	return clipped_lrint<SignedIntegralResult, (sizeof(SignedIntegralResult) << 3)>(x);
}



/// combines float to signed integer conversion with clipping.
// amount has to be a multiple of 8. and both in and out be 16byte aligned.
inline void clip16_lrint(const float in[], std::int16_t out[], int amount) {
#if DIVERSALIS__CPU__X86__SSE >= 2 && defined DIVERSALIS__COMPILER__FEATURE__XMM_INTRINSICS
	__m128 *psrc = (__m128*)in;
	__m128i *pdst = (__m128i*)out;
	do
	{
		__m128i tmpps1 = _mm_cvttps_epi32(*psrc++);
		__m128i tmpps2 = _mm_cvttps_epi32(*psrc++);
		*pdst = _mm_packs_epi32(tmpps1,tmpps2);
		pdst++;
		amount-=8;
	} while(amount>0);
#elif defined DIVERSALIS__CPU__X86__SSE && defined DIVERSALIS__COMPILER__ASSEMBLER__INTEL
	__asm
	{
		mov esi, pSrcSamples
		mov edi, pDstSamples
		mov eax, [amount]
LOOPSTART:
		CVTTPS2DQ xmm0, [esi]
		add esi, 10H
		CVTTPS2DQ xmm1, [esi]
		add esi, 10H
		PACKSSDW xmm1, xmm0
		movaps [edi], xmm1

		add edi, 10H
		sub eax, 8
		cmp eax, 0
		jle END
		jmp LOOPSTART
END:
	}

#else
	int const max((1u << (16 - 1)) - 1); // The compiler is able to compute this statically.
	int const min(-max - 1);
	do {
		*out++ = lrint<std::int16_t>(clipped(float(min), *in++, float(max)));
	} while(--amount);

#endif
}


}}}

#endif
