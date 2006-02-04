// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// Copyright (C) 1999-2005 Psycledelics http://psycle.pastnotecut.org : Johan Boule

///\file
///\interface universalis::operating_system::paths
#pragma once
#include <universalis/detail/project.hpp>
#include <boost/filesystem/path.hpp>
#include <string>
#include "paths/interface.hpp"
#define UNIVERSALIS__COMPILER__DYNAMIC_LINK PACKAGENERIC__MODULE__SOURCE__UNIVERSALIS__OPERATING_SYSTEM__PATHS
#include <universalis/compiler/dynamic_link/begin.hpp>
namespace universalis
{
	namespace operating_system
	{
		namespace paths
		{
			#include "paths/injection/interface.hpp"
		}
	}
}
#include <universalis/compiler/dynamic_link/end.hpp>
