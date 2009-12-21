// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2001-2009 members of the psycle project http://psycle.pastnotecut.org : johan boule <bohan@jabber.org>

///\interface psycle::engine::sample
#pragma once
#include <psycle/detail/project.hpp>
#include <psycle/helpers/math.hpp>
#define UNIVERSALIS__COMPILER__DYNAMIC_LINK  PSYCLE__ENGINE
#include <universalis/compiler/dynamic_link/begin.hpp>
namespace psycle { namespace engine {

/// double (64-bit ieee-754 format) by default.
typedef double real;

/// mathematical operations on floating point numbers
///\todo merge with psycle::helpers
namespace math {

using namespace helpers::math;

}}}
#include <universalis/compiler/dynamic_link/end.hpp>
