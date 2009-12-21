// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2004-2008 members of the psycle project http://psycle.pastnotecut.org ; johan boule <bohan@jabber.org>

///\file
#include <universalis/detail/project.private.hpp>
#include "code_description.hpp"
#include <universalis/stdlib/mutex.hpp>
#include <cstring> // iso std::strerror
#include <sstream>
namespace universalis { namespace stdlib { namespace exceptions {

std::string code_description(int code) throw() {
	std::ostringstream s;
	s << "standard: " << code << " 0x" << std::hex << code << ": ";
	{
		static mutex m;
		scoped_lock<mutex> lock(m);
		s << std::strerror(code);
	}
	return s.str();
}

}}}