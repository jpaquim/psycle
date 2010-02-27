// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2009 members of the psycle project http://psycle.sourceforge.net

#ifndef PSYCLE__CORE__FILE__INCLUDED
#define PSYCLE__CORE__FILE__INCLUDED
#pragma once

#include <psycle/core/config.hpp>

#include <diversalis/os.hpp>
#include <vector>
#include <string>

namespace psycle { namespace core {

	/// misc file operations
	class PSYCLE__CORE__DECL File {
		public:
			/// home dir
			UNIVERSALIS__COMPILER__DEPRECATED("use universalis::os::paths::home()")
			static std::string home();

			/// replaces ~ with home dir
			UNIVERSALIS__COMPILER__DEPRECATED("move this to universalis::os::path")
			static std::string replaceTilde(std::string const & path);
	};

}}
#endif
