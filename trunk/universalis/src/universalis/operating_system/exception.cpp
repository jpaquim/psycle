// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2008 members of the psycle project http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>

///\implementation universalis::operating_system::exception
#include <packageneric/pre-compiled.private.hpp>
#include <packageneric/module.private.hpp>
#include <universalis/detail/project.private.hpp>
#include "exception.hpp"
#include "exceptions/code_description.hpp"
namespace universalis { namespace operating_system {

char const * exception::what() const throw() {
	if(!what_) what_ = new std::string(exceptions::code_description(code()));
	return what_->c_str();
}

}}
