// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2008-2008 members of the psycle project http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>

#pragma once
#include <diversalis/compiler.hpp>
#include <universalis/compiler.hpp>
#include <cmath>
#if defined BOOST_AUTO_TEST_CASE
	#include "pi.hpp"
	#include <universalis/compiler/typenameof.hpp>
	#include <sstream>
#endif
namespace psycle { namespace helpers { namespace math {

	/// computes both the sine and the cosine at the same time
	template<typename Real>
	void inline sin_cos(Real x, Real & restrict sine, Real & restrict cosine) {
		// some compilers are able to optimise those two calls into one.
		sine = std::sin(x);
		cosine = std::cos(x);
	
		#if 0 // doesn't work for negative cosines
			sine = std::sin(x);
			cosine = std::sqrt(1 - sine * sine);
			// we need to do cosine = -cosine for some values of x
		#endif
	}

	#if __STDC__VERSION__ >= 199901 || (defined DIVERSALIS__COMPILER__GNU && DIVERSALIS__COMPILER__VERSION__MAJOR >= 4)
		
		template<>
		void inline sin_cos<>(long double const x, long double & restrict sine, long double & restrict cosine) {
			::sincosl(x, &sine, &cosine);
		}

		template<>
		void inline sin_cos<>(double const x, double & restrict sine, double & restrict cosine) {
			::sincos(x, &sine, &cosine);
		}

		template<>
		void inline sin_cos<>(float const x, float & restrict sine, float & restrict cosine) {
			::sincosf(x, &sine, &cosine);
		}

	#endif
	
	#if defined BOOST_AUTO_TEST_CASE
		template<typename Real>
		void sin_cos_test_template() {
			Real const tolerance = 1e-15;
			Real const input_values[] = { 0, pi / 4, pi / 3, pi };
			for(unsigned int i(0); i < sizeof input_values / sizeof *input_values; ++i) {
				Real const x(input_values[i]);
				Real s, c;
				sin_cos(x, s, c);
				std::ostringstream oss;
				oss << "sin_cos<" << universalis::compiler::typenameof(x) << ">(" << x << "): sin = " << s << ", cos = " << c;
				BOOST_MESSAGE(oss.str());
				BOOST_CHECK(std::abs(s - std::sin(x)) < tolerance);
				BOOST_CHECK(std::abs(c - std::cos(x)) < tolerance);
			}
		}
		BOOST_AUTO_TEST_CASE(sin_cos_test) {
			sin_cos_test_template<float>();
			sin_cos_test_template<double>();
			#if __STDC__VERSION__ >= 199901 || (defined DIVERSALIS__COMPILER__GNU && DIVERSALIS__COMPILER__VERSION__MAJOR >= 4)
				sin_cos_test_template<long double>();
			#endif
		}
	#endif
	
}}}
