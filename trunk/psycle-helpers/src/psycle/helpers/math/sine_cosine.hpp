// -*- mode:c++; indent-tabs-mode:t -*-
// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2008-2008 members of the psycle project http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>

#pragma once
#include <diversalis/compiler.hpp>
#include <universalis/compiler.hpp>
#include <cmath>
namespace psycle { namespace helpers { namespace math {

	/// computes both the sine and the cosine at the same time
	template<typename Real> UNIVERSALIS__COMPILER__CONST
	void inline sin_cos(Real const & restrict x, Real & restrict sine, Real & restrict cosine) {
		// some compilers are able to optimise those two calls into one.
		sine = std::sin(x);
		cosine = std::cos(x);
	
		#if 0 // doesn't work for negative cosines
			sine = std::sin(x);
			cosine = std::sqrt(1 - sine * sine);
			// we need to do cosine = -cosine for some values of x
		#endif	
	}
}}}

// inline implementation
namespace psycle { namespace helpers { namespace math {

	#if __STDC__VERSION__ >= 199901 || \
		(defined DIVERSALIS__COMPILER__GNU && DIVERSALIS__COMPILER__VERSION__MAJOR >= 4)
		
		template<> UNIVERSALIS__COMPILER__CONST
		void inline sin_cos<>(long double const & restrict x, long double & restrict sine, long double & restrict cosine) {
			::sincosl(x, &sine, &cosine);
		}

		template<> UNIVERSALIS__COMPILER__CONST
		void inline sin_cos<>(double const & restrict x, double & restrict sine, double & restrict cosine) {
			::sincos(x, &sine, &cosine);
		}

		template<> UNIVERSALIS__COMPILER__CONST
		void inline sin_cos<>(float const & restrict x, float & restrict sine, float & restrict cosine) {
			::sincosf(x, &sine, &cosine);
		}

	#endif
}}}
