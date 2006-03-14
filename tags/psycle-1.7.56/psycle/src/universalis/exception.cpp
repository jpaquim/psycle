// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// Copyright (C) 1999-2005 Psycledelics http://psycle.pastnotecut.org : Johan Boule, Magnus Johnson

///\file
///\implementation universalis::exception
#include <universalis/detail/project.private.hpp>
#include "exception.hpp"
#include "operating_system/loggers.hpp"
#include "operating_system/threads/id.hpp"
#include "compiler/typenameof.hpp"
namespace universalis
{
	namespace loggers = operating_system::loggers;

	exception::exception(std::string const & what, compiler::location const & location) throw() : std::runtime_error(what), location_(location)
	{
		if(loggers::exception()())
		{
			std::ostringstream s;
			s 
				///\todo thread name
				<< "exception: thread id: " << operating_system::threads::id::current() << ", "
				<< compiler::typenameof(*this) << ": " << this->what();
			loggers::exception()(s.str(), location);
		}
	}

	exception::operator std::string () const throw()
	{
		return what(); 
	}
}

// arch-tag: 754fc453-d958-4c10-9c5f-37672a2f8f7a
