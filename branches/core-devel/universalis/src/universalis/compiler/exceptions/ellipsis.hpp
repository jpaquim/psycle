// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2007 johan boule <bohan@jabber.org>
// copyright 2004-2007 psycledelics http://psycle.pastnotecut.org

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
			/// provides information about the exception in an ellipsis catch(...) clause.
			/// Not all compilers makes it possible to obtain information.
			/// The GNU compiler and Borland's one do.
			std::string UNIVERSALIS__COMPILER__DYNAMIC_LINK ellipsis();
		}
	}
}
#include <universalis/compiler/dynamic_link/end.hpp>
