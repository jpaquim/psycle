// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2007 johan boule <bohan@jabber.org>
// copyright 2004-2007 psycledelics http://psycle.pastnotecut.org

///\file
///\brief
#include <universalis/detail/project.private.hpp>

// weird, must be included last or mingw 3.4.1 segfaults
//#include "code_description.hpp"

#include <universalis/os/exceptions/code_description.hpp>

// weird, must be included last or mingw 3.4.1 segfaults
#include "code_description.hpp"

namespace universalis { namespace cpu { namespace exceptions {

std::string code_description(int const & code) throw() {
	return os::exceptions::detail::code_description(
		code
		#if defined DIVERSALIS__OS__MICROSOFT
			, /* from_cpu */ true
		#endif
	);
}

}}}
