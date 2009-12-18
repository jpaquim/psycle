// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2007 johan boule <bohan@jabber.org>
// copyright 2004-2007 psycledelics http://psycle.pastnotecut.org

///\file
///\brief

#ifndef UNIVERSALIS__CPU__EXCEPTIONS__CODE_DESCRIPTION__INCLUDED
#define UNIVERSALIS__CPU__EXCEPTIONS__CODE_DESCRIPTION__INCLUDED
#pragma once

#include <universalis/detail/project.hpp>
#include <string>

#define UNIVERSALIS__COMPILER__DYNAMIC_LINK UNIVERSALIS__SOURCE
#include <universalis/compiler/dynamic_link/begin.hpp>

namespace universalis { namespace cpu { namespace exceptions {

std::string UNIVERSALIS__COMPILER__DYNAMIC_LINK code_description(int const &) throw();

}}}

#include <universalis/compiler/dynamic_link/end.hpp>

#endif
