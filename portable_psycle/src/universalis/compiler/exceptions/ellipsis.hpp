// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// Copyright (C) 1999-2005 Psycledelics http://psycle.pastnotecut.org : Johan Boule

///\file
///\interface gets information from ellipsis exceptions
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
// arch-tag: 7bdcd8a1-3e2d-4524-a1b1-7d9993f8a69b
