// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2008-2011 members of the psycle project http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>

#ifndef PSYCLE__HELPERS__MATH__SINCOS__INCLUDED
#define PSYCLE__HELPERS__MATH__SINCOS__INCLUDED
#pragma once

#include <universalis.hpp>
#include <cmath>
#ifdef BOOST_AUTO_TEST_CASE
	#include "constants.hpp"
	#include <universalis.hpp>
	#include <sstream>
#endif

namespace psycle { namespace helpers { namespace math {

/// computes both the sine and the cosine at the same time
template<typename Real>
void inline sincos(
	Real x,
	Real & UNIVERSALIS__COMPILER__RESTRICT_REF sine,
	Real & UNIVERSALIS__COMPILER__RESTRICT_REF cosine
) {
	// some compilers are able to optimise those two calls into one.
	sine = std::sin(x);
	cosine = std::cos(x);
}

#if DIVERSALIS__STDLIB__MATH >= 199901
	template<>
	void inline sincos<>(
		long double const x,
		long double & UNIVERSALIS__COMPILER__RESTRICT_REF sine,
		long double & UNIVERSALIS__COMPILER__RESTRICT_REF cosine
	) {
		::sincosl(x, &sine, &cosine);
	}

	template<>
	void inline sincos<>(
		double const x,
		double & UNIVERSALIS__COMPILER__RESTRICT_REF sine,
		double & UNIVERSALIS__COMPILER__RESTRICT_REF cosine
	) {
		::sincos(x, &sine, &cosine);
	}

	template<>
	void inline sincos<>(
		float const x,
		float & UNIVERSALIS__COMPILER__RESTRICT_REF sine,
		float & UNIVERSALIS__COMPILER__RESTRICT_REF cosine
	) {
		::sincosf(x, &sine, &cosine);
	}
#endif

/******************************************************************************************/
#ifdef BOOST_AUTO_TEST_CASE
	namespace test {
		template<typename Real>
		void sincos_test_template() {
			Real const tolerance = 1e-7;
			Real const input_values[] = { 0, pi / 4, pi / 3, pi / 2, pi };
			for(unsigned int i(0); i < sizeof input_values / sizeof *input_values; ++i) {
				Real const x(input_values[i]);
				Real s, c;
				sincos(x, s, c);
				std::ostringstream oss;
				oss << "sin_cos<" << universalis::compiler::typenameof(x) << ">(" << x << "): sin = " << s << ", cos = " << c;
				BOOST_MESSAGE(oss.str());
				BOOST_CHECK(std::abs(s - std::sin(x)) < tolerance);
				BOOST_CHECK(std::abs(c - std::cos(x)) < tolerance);
			}
		}
		BOOST_AUTO_TEST_CASE(sincos_test) {
			sincos_test_template<float>();
			sincos_test_template<double>();
			#if DIVERSALIS__STDLIB__MATH >= 199901
				sincos_test_template<long double>();
			#endif
		}
	}
#endif
	
}}}

#endif
