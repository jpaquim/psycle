// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2007 johan boule <bohan@jabber.org>
// copyright 2004-2007 psycledelics http://psycle.pastnotecut.org

///\file
///\brief meta header
#pragma once
#include "stdc_secure_lib.hpp"
#include "detail/iso646.hpp"
#include <cassert>



/// same as std::min, but without type checking
#define UNIVERSALIS__STANDARD_LIBRARY__LOOSE_MIN(a, b) ((a) < (b) ? (a) : (b))
/// same as std::max, but without type checking
#define UNIVERSALIS__STANDARD_LIBRARY__LOOSE_MAX(a, b) ((a) > (b) ? (a) : (b))
