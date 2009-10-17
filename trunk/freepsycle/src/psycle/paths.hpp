// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2009 members of the psycle project http://psycle.pastnotecut.org : johan boule <bohan@jabber.org>

///\interface psycle::paths
#pragma once
#include <psycle/detail/project.hpp>
#include <universalis/os/paths/interface.hpp>
#define UNIVERSALIS__COMPILER__DYNAMIC_LINK  PSYCLE__PATHS
#include <universalis/compiler/dynamic_link/begin.hpp>
namespace psycle { namespace paths {
	#include <universalis/os/paths/injection/interface.hpp>
}}
#include <universalis/compiler/dynamic_link/end.hpp>
