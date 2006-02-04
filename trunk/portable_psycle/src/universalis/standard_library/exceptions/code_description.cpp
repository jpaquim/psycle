// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// Copyright (C) 1999-2005 Psycledelics http://psycle.pastnotecut.org : Johan Boule

///\file
///\brief
#include PACKAGENERIC__PRE_COMPILED
#include PACKAGENERIC
#include <universalis/detail/project.private.hpp>
#include "code_description.hpp"
#include <cstring> // iso std::strerror
#include <boost/thread/mutex.hpp>
#include <sstream>
namespace universalis
{
	namespace standard_library
	{
		namespace exceptions
		{
			std::string code_description(int const & code) throw()
			{
				std::ostringstream s;
				s << "standard: " << code << " 0x" << std::hex << code << ": ";
				static boost::mutex mutex;
				boost::mutex::scoped_lock lock(mutex);
				s << std::strerror(code);
				return s.str();
			}
		}
	}
}
