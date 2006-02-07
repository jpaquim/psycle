// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// Copyright (C) 2006 Psycledelics http://psycle.pastnotecut.org

///\interface psycle::coal::hello
#pragma once
#include <string>
#define UNIVERSALIS__COMPILER__DYNAMIC_LINK PSYCLE__COAL__HELLO
#include <universalis/compiler/dynamic_link/begin.hpp>

namespace psycle
{
	namespace coal
	{
		namespace hello
		{
			class UNIVERSALIS__COMPILER__DYNAMIC_LINK hello
			{
				public:
					std::string say_hello();
			};
		}
	}
}

#include <universalis/compiler/dynamic_link/end.hpp>
