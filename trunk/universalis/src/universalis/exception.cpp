// -*- mode:c++; indent-tabs-mode:t -*-
// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2008 psycle development team http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>

///\implementation universalis::exception
#include <packageneric/pre-compiled.private.hpp>
#include <packageneric/module.private.hpp>
#include <universalis/detail/project.private.hpp>
#include "exception.hpp"
#include "operating_system/loggers.hpp"
#include "compiler/typenameof.hpp"
#include <thread>
namespace universalis { namespace exceptions {

runtime_error::runtime_error(std::string const & what, compiler::location const & location, void const * cause) throw()
: std::runtime_error(what), locatable(location), causality(cause)
{
	if(operating_system::loggers::trace()()) {
		std::ostringstream s;
		s 
			<< "exception: thread id: " << std::this_thread::id() << ", "
			<< compiler::typenameof(*this) << ": " << this->what();
		operating_system::loggers::exception()(s.str(), location);
	}
}

}}
