// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// Copyright (C) 1999-2006 Johan Boule <bohan@jabber.org>
// Copyright (C) 2004-2006 Psycledelics http://psycle.pastnotecut.org

///\implementation psycle::engine::exception
#include PACKAGENERIC__PRE_COMPILED
#include PACKAGENERIC
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
