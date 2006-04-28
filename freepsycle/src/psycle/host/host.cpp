// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2006 johan boule <bohan@jabber.org>
// copyright 2004-2006 psycledelics http://psycle.pastnotecut.org

///\implementation psycle::host
#include PACKAGENERIC__PRE_COMPILED
#include PACKAGENERIC
#include <psycle/detail/project.private.hpp>
#include <psycle/engine/hello.hpp>
#include "host.hpp"
namespace psycle
{
	namespace host
	{
		void hello()
		{
			engine::hello hello;
			hello.say_hello();
		}
	}
}
