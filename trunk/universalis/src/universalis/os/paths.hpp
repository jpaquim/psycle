// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2010 members of the psycle project http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>

///\interface universalis::os::paths

#ifndef UNIVERSALIS__OS__PATHS__INCLUDED
#define UNIVERSALIS__OS__PATHS__INCLUDED
#pragma once

#include <universalis/detail/project.hpp>
#include <boost/filesystem/path.hpp>
#include <string>

#define UNIVERSALIS__COMPILER__DYNAMIC_LINK UNIVERSALIS__SOURCE
#include <universalis/compiler/dynamic_link/begin.hpp>

namespace universalis { namespace os { namespace paths {
	using namespace boost::filesystem;

	#if 0 ///\todo not implemented yet on posix
	/// the path to the image file of the currently executing process
	path const UNIVERSALIS__COMPILER__DYNAMIC_LINK & process_executable_file_path()
	#endif

	/// the user's home dir
	path const UNIVERSALIS__COMPILER__DYNAMIC_LINK & home();

	/// the path in the user's home dir where an application should write data that are local to the computer
	path const UNIVERSALIS__COMPILER__DYNAMIC_LINK & home_app_local(std::string const & app_name);

	/// the path in the user's home dir where an application should write data that are not local to the computer
	path const UNIVERSALIS__COMPILER__DYNAMIC_LINK & home_app_roaming(std::string const & app_name);
}}}

#include <universalis/compiler/dynamic_link/end.hpp>

#endif
