// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// Copyright (C) 1999-2006 Johan Boule <bohan@jabber.org>
// Copyright (C) 2004-2006 Psycledelics http://psycle.pastnotecut.org

///\file
///\interface psycle::paths
#pragma once
#include <psycle/detail/project.hpp>
#include <universalis/operating_system/paths/interface.hpp>
#define UNIVERSALIS__COMPILER__DYNAMIC_LINK  PSYCLE__PATHS
#include <universalis/compiler/dynamic_link/begin.hpp>
namespace psycle
{
	namespace paths
	{
		#include <universalis/operating_system/paths/injection/interface.hpp>
	}
}
#include <universalis/compiler/dynamic_link/end.hpp>
