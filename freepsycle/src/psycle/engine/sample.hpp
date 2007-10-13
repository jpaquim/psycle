// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2001-2007 psycledelics http://psycle.pastnotecut.org ; johan boule <bohan@jabber.org>

///\interface psycle::engine::sample
#pragma once
#include <psycle/detail/project.hpp>
#include <universalis/compiler/numeric.hpp>
#include <limits>
#include <cmath>
#include <cassert>
#include <boost/static_assert.hpp>
#define UNIVERSALIS__COMPILER__DYNAMIC_LINK  PSYCLE__ENGINE__SAMPLE
#include <universalis/compiler/dynamic_link/begin.hpp>
namespace psycle { namespace engine {

/// mathematical operations on floating point numbers
namespace math {
	template<typename Real>
	Real inline deci_bell_to_linear(Real const & deci_bell) throw() {
		return std::pow(static_cast<unsigned int>(10), deci_bell / 20);
	}
	
	template<typename Real>
	Real inline linear_to_deci_bell(Real const & linear) throw() {
		return 20 * std::log10(linear);
	}
	
	template<typename Real>
	Real inline limited(Real const & min, Real const & input, Real const & max) throw() {
		if(input < min) return min;
		else if(input > max) return max;
		else return input;
	}
	
	template<typename Real>
	void inline limit(Real const & min, Real & real, Real const & max) throw() {
		real = limited<Real>(min, real, max);
	}

	/// converts a floating point number to an integer.
	///\post std::numeric_limits<Integral>::min() <= output and output <= std::numeric_limits<Integral>::max()
	template<typename Real, typename Integral>
	void inline convert(Real const & input, Integral & output, Real const & amplification = -std::numeric_limits<Integral>::min()) throw() {
		Real const max(std::numeric_limits<Integral>::max());
		Real const min(std::numeric_limits<Integral>::min());
		output = limited<Real>(min, input * amplification, max);
		assert(min <= output and output <= max);
	}

	/// converts a floating point number to an unsigned integer.
	///\post std::numeric_limits<Integral>::min() <= output and output <= std::numeric_limits<Integral>::max()
	template<typename Real, typename Integral>
	void inline convert_to_unsigned(Real const & input, Integral & output, Real const & amplification = -std::numeric_limits<Integral>::min()) throw() {
		convert<Real, Integral>(input, output, amplification); 
		Real const max(std::numeric_limits<Integral>::max());
		Real const min(std::numeric_limits<Integral>::min());
		output += max / 2;
		assert(min <= output and output <= max);
	}
}

/// double (64-bit ieee-754 format) by default.
typedef universalis::compiler::numeric<64>::floating_point real;
	
namespace math {
	/// john napier's log(2) aka ln(2) constant 
	real const log2 = /* std::log(2) */ .69314718055994530941723212145817656807550013436025/*...*/;
	/// apery's zeta(3) constant
	real const zeta3 = 1.20205690315959428539973816151144999076498629234049/*...*/;
	/// leibniz's and euler's e constant
	real const e = /* std::exp(1) */ 2.71828182845904523536028747135266249775724709369995/*...*/;
	/// pythagoreans' nu2 constant
	real const nu2 = 1.41421356237309504880168872420969807856967187537694/*...*/;
	/// leonhard euler's gamma constant
	real const gamma = .57721566490153286060651209008240243104215933593992/*...*/;
	/// archimedes' pi constant
	real const pi = /* std::acos(-1) */ 3.1415926535897932384626433832795028841971693993751/*...*/;
	real const radian = 1;
	real const degree = radian * pi / 180;
	template real inline deci_bell_to_linear<real>(real const &);
	template real inline linear_to_deci_bell<real>(real const &);

	template<int i>
	struct compile_time_factorial {
		int const static value = i * compile_time_factorial<i - 1>::value;
		BOOST_STATIC_ASSERT(value > 0); // makes constant overflows errors, not just warnings
	};

	template<>
	struct compile_time_factorial<0> {
		int const static value = 1;
	};
}

namespace vst {
	float inline limited(float const & f) throw() {
		return math::limited<float>(0, f, 1); 
	}
	void inline limit(float & f) throw() {
		f = limited(f); 
	}
}

}}
#include <universalis/compiler/dynamic_link/end.hpp>

