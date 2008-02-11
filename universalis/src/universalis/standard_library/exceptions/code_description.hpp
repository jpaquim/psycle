// -*- mode:c++; indent-tabs-mode:t -*-
// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2007 johan boule <bohan@jabber.org>
// copyright 2004-2007 psycledelics http://psycle.pastnotecut.org

///\file
///\brief
#pragma once
#include <universalis/detail/project.hpp>
#include <string>
#include <cerrno>
#define UNIVERSALIS__COMPILER__DYNAMIC_LINK UNIVERSALIS__STANDARD_LIBRARY__EXCEPTIONS__CODE_DESCRIPTION
#include <universalis/compiler/dynamic_link/begin.hpp>
namespace universalis
{
	namespace standard_library
	{
		namespace exceptions
		{
			UNIVERSALIS__COMPILER__DYNAMIC_LINK std::string code_description(int const & code = errno) throw();
		}
	}
}
#include <universalis/compiler/dynamic_link/end.hpp>
