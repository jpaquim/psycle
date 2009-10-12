// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2004-2008 members of the psycle project http://psycle.pastnotecut.org ; johan boule <bohan@jabber.org>

///\file
#include <packageneric/pre-compiled.private.hpp>
#include <packageneric/module.private.hpp>
#include <universalis/detail/project.private.hpp>
#include "code_description.hpp"
#include <cstring> // iso std::strerror
#include <mutex>
#include <sstream>
namespace universalis { namespace standard_library { namespace exceptions {

std::string code_description(int const & code) throw() {
	std::ostringstream s;
	s << "standard: " << code << " 0x" << std::hex << code << ": ";
	{
		static std::mutex mutex;
		std::scoped_lock<std::mutex> lock(mutex);
		s << std::strerror(code);
	}
	return s.str();
}

}}}
