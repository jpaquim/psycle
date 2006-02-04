// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// Copyright (C) 1999-2005 Psycledelics http://psycle.pastnotecut.org : Johan Boule, Magnus Johnson

///\file
///\implementation universalis::operating_system::exception
#include PACKAGENERIC__PRE_COMPILED
#include PACKAGENERIC
#include <universalis/detail/project.private.hpp>
#include "exception.hpp"
#include "exceptions/code_description.hpp"
#include "loggers.hpp"
#include "threads/id.hpp"
#include <universalis/compiler/typenameof.hpp>
namespace universalis
{
	namespace operating_system
	{
		char const * exception::what() const throw()
		{
			if(!what_) what_ = new std::string(exceptions::code_description(code()));
			return what_->c_str();
		}

		namespace exceptions
		{
			runtime_error::runtime_error(std::string const & what, compiler::location const & location, void const * cause) throw()
			:
				universalis::exceptions::runtime_error(what, location, cause)
			{
			}
		}
	}
}
