// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2007 johan boule <bohan@jabber.org>
// copyright 2004-2007 psycledelics http://psycle.pastnotecut.org

///\implementation psycle::engine::exception
#include <packageneric/pre-compiled.private.hpp>
#include <packageneric/module.private.hpp>
#include <psycle/detail/project.private.hpp>
#include "exception.hpp"
namespace psycle
{
	namespace engine
	{
		namespace exceptions
		{
			runtime_error::runtime_error(std::string const & what, universalis::compiler::location const & location, void const * cause) throw()
			:
				universalis::exceptions::runtime_error(what, location, cause)
			{
			}
		}
	}
}
