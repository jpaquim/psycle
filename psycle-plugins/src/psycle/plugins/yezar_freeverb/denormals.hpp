>/* -*- mode:c++, indent-tabs-mode:t -*- */
#pragma once

/*
// Macro for killing denormalled numbers
//
// Written by Jezar at Dreampoint, June 2000
// http://www.dreampoint.co.uk
// Based on IS_DENORMAL macro by Jon Watte
// This code is public domain

#define undenormalise(sample) if(((*(unsigned int*)&sample)&0x7f800000)==0) sample=0.0f
*/

/*
	The above code breaks strict aliasing rules when
	-fstrict-aliasing optimization is enabled. This version is safer, and
	probably faster too:
*/
#include <psycle/helpers/math/erase_denormals.hpp>
#define undenormalise(sample) (psy::helpers::math::fast_erase_denormals_inplace(sample))
