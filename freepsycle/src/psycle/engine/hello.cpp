// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// Copyright (C) 1999-2006 Johan Boule <bohan@jabber.org>
// Copyright (C) 2004-2006 Psycledelics http://psycle.pastnotecut.org

///\implementation psycle::engine::hello
#include PACKAGENERIC__PRE_COMPILED
#include PACKAGENERIC
#include <psycle/detail/project.private.hpp>
#include "hello.hpp"
namespace psycle
{
	namespace engine
	{
		std::string hello::say_hello() const throw()
		{
			std::string const static s("From psycle's engine: Hello, I'm Psycle... and I know I'm ridiculous!");
			return s;
		}
	}
}
