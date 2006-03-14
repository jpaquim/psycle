// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// Copyright (C) 1999-2005 Psycledelics http://psycle.pastnotecut.org : Johan Boule, Magnus Johnson

///\file
///\implementation universalis::operating_system::exception
#include <universalis/detail/project.private.hpp>
#include "exception.hpp"
#include "loggers.hpp"
#include "threads/id.hpp"
#include <universalis/compiler/typenameof.hpp>
namespace universalis
{
	namespace operating_system
	{
		exception::exception(std::string const & what, compiler::location const & location) throw() : universalis::exception(what, location)
		{
		}
	}
}
