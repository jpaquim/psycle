// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2010 members of the psycle project http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>

///\file
///\brief stringization

#ifndef UNIVERSALIS__COMPILER__STRINGIZE__INCLUDED
#define UNIVERSALIS__COMPILER__STRINGIZE__INCLUDED
#pragma once

#include <universalis/detail/project.hpp>
#include <boost/preprocessor/stringize.hpp>

#define UNIVERSALIS__COMPILER__STRINGIZE(tokens) BOOST_PP_STRINGIZE(tokens)

#endif
