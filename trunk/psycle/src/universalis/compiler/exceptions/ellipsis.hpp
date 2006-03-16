// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// Copyright (C) 1999-2006 Johan Boule <bohan@jabber.org>
// Copyright (C) 2004-2006 Psycledelics http://psycle.pastnotecut.org

///\interface gets information from ellipsis exceptions: catch(...)
#pragma once
#include <universalis/detail/project.hpp>
#include <string>
#define UNIVERSALIS__COMPILER__DYNAMIC_LINK UNIVERSALIS__COMPILER__EXCEPTIONS__ELLIPSIS
#include <universalis/compiler/dynamic_link/begin.hpp>
namespace universalis
{
	namespace compiler
	{
		namespace exceptions
		{
			std::string UNIVERSALIS__COMPILER__DYNAMIC_LINK ellipsis();
		}
	}
}
#include <universalis/compiler/dynamic_link/end.hpp>
