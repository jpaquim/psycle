// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// Copyright (C) 1999-2006 Johan Boule <bohan@jabber.org>
// Copyright (C) 2004-2006 Psycledelics http://psycle.pastnotecut.org

///\implementation universalis::exception
#include <packageneric/pre-compiled.private.hpp>
#include PACKAGENERIC
#include <universalis/detail/project.private.hpp>
#include "exception.hpp"
#include "operating_system/loggers.hpp"
#include "operating_system/threads/id.hpp"
#include "compiler/typenameof.hpp"
namespace universalis
{
	namespace exceptions
	{
		runtime_error::runtime_error(std::string const & what, compiler::location const & location, void const * cause) throw()
		:
			std::runtime_error(what.c_str()),
			locatable(location),
			causality(cause)
		{
			if(operating_system::loggers::trace()())
			{
				std::ostringstream s;
				s 
					<< "exception: thread id: " << operating_system::threads::id::current() << ", "
					<< compiler::typenameof(*this) << ": " << this->what();
				operating_system::loggers::exception()(s.str(), location);
			}
		}

		#if !defined NDEBUG
			bad_cast::bad_cast(std::type_info const & from, std::type_info const & to, compiler::location const & location, void const * cause) throw()
			:
				locatable(location),
				causality(cause),
				from_(from),
				to_(to)
			{
			}
		#endif
	}
}
