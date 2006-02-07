// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// Copyright (C) 2006 Psycledelics http://psycle.pastnotecut.org

///\implementation psycle::coal::hello
#include PACKAGENERIC__PRE_COMPILED
#include PACKAGENERIC
#include <psycle/coal/detail/project.private.hpp>
#include "hello.hpp"

namespace psycle
{
	namespace coal
	{
		namespace hello
		{
			std::string hello::say_hello()
			{
				return "psycoal";
			}
		}
	}
}
