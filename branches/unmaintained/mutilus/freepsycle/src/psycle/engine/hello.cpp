// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2007 johan boule <bohan@jabber.org>
// copyright 2004-2007 psycledelics http://psycle.pastnotecut.org

///\implementation psycle::engine::hello
#include <packageneric/pre-compiled.private.hpp>
#include <packageneric/module.private.hpp>
#include <psycle/detail/project.private.hpp>
#include "hello.hpp"
namespace psycle
{
	namespace engine
	{
		std::string hello::say_hello() const throw()
		{
			std::string const static s("From psycle's engine: Hello World! I'm Psycle... and I know I'm ridiculous!");
			return s;
		}
	}
}
