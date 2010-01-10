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
			/// reads an ascii text file
			static std::string readFile(std::string const & path);
			struct list_modes {
				int const static files = 1;
				int const static dirs = 2;
			};
			char const static path_env_var_name[];
			/// lists the file in the dir
			static std::vector<std::string> fileList(std::string const & path, int list_mode = list_modes::files | list_modes::dirs );   
			/// the current dir
			static std::string workingDir();
			/// append the specified path to the PATH environment and returns it.
			static std::string appendDirToEnvPath(std::string const & path);
			/// sets the PATH environment variable to the specified path and returns it.
			static bool setEnvPath(std::string const & new_path);
			/// gets the PATH environment variable.
			static std::string getEnvPath();
			/// whether the file can be read
			static bool fileIsReadable(std::string const & path);
			/// the parent of the current dir
			static std::string parentWorkingDir();
			/// changes the current dir
			static void cd(std::string const & path);
			/// changes the current dir to the home dir
			static void cdHome();
			/// home dir
			static std::string home();
			/// replaces ~ with home dir
			static std::string replaceTilde(std::string const & path);
			/// appends a path separator if needed
			static void ensurePathTerminated(std::string & path);
			static std::string env(std::string const & name);
			/// path separator
			static std::string const & slash();
			static std::string replaceIllegalXmlChr(std::string const & text, bool strict = true);
			/// basename
			static std::string extractFileNameFromPath(std::string const & path);
			static void inline unlink(std::string const & path) {
				#if defined DIVERSALIS__OS__MICROSOFT
					DeleteFile
				#else
					unlink
				#endif
				(path.c_str());
			}
	};

}}
#endif
