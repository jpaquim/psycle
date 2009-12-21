// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2004-2009 members of the psycle project http://psycle.sourceforge.net

#ifndef PSYCLE__HELPERS__MATH__CONSTANTS__INCLUDED
#define PSYCLE__HELPERS__MATH__CONSTANTS__INCLUDED
#pragma once

namespace psycle { namespace helpers { namespace math {

/// john napier's log(2) aka ln(2) constant
double const log2 = /* std::log(2) */ .69314718055994530941723212145817656807550013436025/*...*/;
/// apery's zeta(3) constant
double const zeta3 = 1.20205690315959428539973816151144999076498629234049/*...*/;
/// leibniz's and euler's e constant
double const e = /* std::exp(1) */ 2.71828182845904523536028747135266249775724709369995/*...*/;
/// pythagoreans' nu2 constant
double const nu2 = 1.41421356237309504880168872420969807856967187537694/*...*/;
/// leonhard euler's gamma constant
double const gamma = .57721566490153286060651209008240243104215933593992/*...*/;
/// archimedes' pi constant
double const pi = /* std::acos(-1) */ 3.1415926535897932384626433832795028841971693993751/*...*/;
float const pi_f = float(pi);
double const radians_per_degree = pi / 180;

}}}

#endif
