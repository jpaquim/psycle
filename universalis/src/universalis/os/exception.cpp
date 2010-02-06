// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2008 members of the psycle project http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>

///\implementation universalis::os::exception
#include <universalis/detail/project.private.hpp>
#include "exception.hpp"
#include "exceptions/code_description.hpp"
#if defined DIVERSALIS__OS__MICROSOFT
	#include <universalis/stdlib/exceptions/code_description.hpp>
#endif

namespace universalis { namespace os {

char const * exception::what() const throw() {
	if(!what_) what_ = new std::string(exceptions::code_description(code()));
	return what_->c_str();
}

#if defined DIVERSALIS__OS__MICROSOFT
	namespace exceptions { namespace detail {
		char const * posix::what() const throw() {
			if(!what_) what_ = new std::string(stdlib::exceptions::code_description(code()));
			return what_->c_str();
		}
	}}
#endif

}}
