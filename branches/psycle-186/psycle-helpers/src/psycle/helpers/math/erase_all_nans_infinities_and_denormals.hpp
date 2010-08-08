// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2004-2009 members of the psycle project http://psycle.sourceforge.net

#ifndef PSYCLE__HELPERS__MATH__ERASE_ALL_NANS_INFINITES_AND_DENORMALS__INCLUDED
#define PSYCLE__HELPERS__MATH__ERASE_ALL_NANS_INFINITES_AND_DENORMALS__INCLUDED
#pragma once

#include <universalis.hpp>
#if 0 && defined DIVERSALIS__COMPILER__FEATURE__XMM_INTRINSICS
	//#include <xmmintrin.h>
#endif

namespace psycle { namespace helpers { namespace math {

/// Cure for malicious samples
/// Type : Filters Denormals, NaNs, Infinities
/// References : Posted by urs[AT]u-he[DOT]com
void inline erase_all_nans_infinities_and_denormals(float & sample) {
	#if !defined DIVERSALIS__CPU__X86
		// just do nothing.. not crucial for other archs
	#else
		BOOST_STATIC_ASSERT((sizeof sample == 4));
		std::uint32_t const bits(reinterpret_cast<std::uint32_t&>(sample));
		std::uint32_t const exponent_mask
		(
			#if defined DIVERSALIS__CPU__ENDIAN__LITTLE
				0x7f800000
			#else
				#error sorry, was not much thought
			#endif
		);
		std::uint32_t const exponent(bits & exponent_mask);

		// exponent < exponent_mask is 0 if NaN or Infinity, otherwise 1
		std::uint32_t const not_nan_nor_infinity(exponent < exponent_mask);

		// exponent > 0 is 0 if denormalized, otherwise 1
		std::uint32_t const not_denormal(exponent > 0);

		// It does not work for nans!
		//sample *= not_nan_nor_infinity & not_denormal;
		
		if (!not_nan_nor_infinity) {
			sample = 0;
		} else {
			sample *= not_denormal;
		}
	#endif
}

///\todo This works, but uses too much CPU probably. (at least on 32bit processors)
void inline erase_all_nans_infinities_and_denormals(double & sample) {
	#if !defined DIVERSALIS__CPU__X86
		// just do nothing.. not crucial for other archs
	#else
		std::uint64_t const bits(reinterpret_cast<std::uint64_t&>(sample));
		std::uint64_t const exponent_mask(
			#if defined DIVERSALIS__CPU__ENDIAN__LITTLE
				0x7f80000000000000ULL
			#else
				#error sorry, was not much thought
			#endif
		);
		std::uint64_t const exponent(bits & exponent_mask);

		// exponent < exponent_mask is 0 if NaN or Infinity, otherwise 1
		std::uint64_t const not_nan_nor_infinity(exponent < exponent_mask);

		// exponent > 0 is 0 if denormalized, otherwise 1
		std::uint64_t const not_denormal(exponent > 0);

		// It does not work for nans!
		//sample *= not_nan_nor_infinity & not_denormal;

		if (!not_nan_nor_infinity) {
			sample = 0;
		} else {
			sample *= not_denormal;
		}

	#endif
}

void inline erase_all_nans_infinities_and_denormals(float samples[], unsigned int const sample_count) {
	for(unsigned int i(0); i < sample_count; ++i) erase_all_nans_infinities_and_denormals(samples[i]);
}

void inline erase_all_nans_infinities_and_denormals(double samples[], unsigned int const sample_count) {
	for(unsigned int i(0); i < sample_count; ++i) erase_all_nans_infinities_and_denormals(samples[i]);
}

}}}

#endif
