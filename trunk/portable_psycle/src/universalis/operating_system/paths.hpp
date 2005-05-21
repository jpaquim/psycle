// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// Copyright (C) 1999-2005 Psycledelics http://psycle.pastnotecut.org : Johan Boule

///\file
///\interface universalis::operating_system::paths
#pragma once
#include <universalis/detail/project.hpp>
#include <boost/filesystem/path.hpp>
#include <string>
//#namespace UNIVERSALIS
	//#namespace OPERATING_SYSTEM
		#define UNIVERSALIS__OPERATING_SYSTEM__PATHS(x) \
		namespace paths \
		{ \
			using boost::filesystem::path; \
			path const & bin(); \
			path const & lib(); \
			path const & share(); \
			path const & var(); \
			path const & etc(); \
			path const & home(); \
			namespace package \
			{ \
				std::string const & name(); \
				namespace version \
				{ \
					std::string const & string(); \
					/** it couldn't be named just 'major', because 'major' is a #define in <sys/sysmacros.h> used for device nodes and <cstdint> includes it. */ \
					unsigned int major_number() throw(); \
					/** it couldn't be named just 'minor', because 'minor' is a #define in <sys/sysmacros.h> used for device nodes and <cstdint> includes it. */ \
					unsigned int minor_number() throw(); \
					unsigned int patch_number() throw(); \
				} \
				path const & lib(); \
				path const & share(); \
				path const & pixmaps(); \
				path const & doc(); \
				path const & var(); \
				path const & log(); \
				path const & etc(); \
				path const & home(); \
			} \
		}
	//#endnamespace
//#endnamespace

namespace universalis
{
	namespace operating_system
	{
		UNIVERSALIS__OPERATING_SYSTEM__PATHS(x)
	}
}

// arch-tag: 77d39186-fddf-4486-8b7e-8e6ec34cdff7
