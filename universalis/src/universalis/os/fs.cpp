// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2010 members of the psycle project http://psycle.sourceforge.net

///\implementation universalis::os::fs

#include <universalis/detail/project.private.hpp>
#include "fs.hpp"
#include "exception.hpp"
#include <cstdlib> // std::getenv for user's home dir
#include <sstream>
#include <iostream>
#if defined DIVERSALIS__OS__MICROSOFT
	#include <shlobj.h> // for SHGetFolderPath
#endif

namespace universalis { namespace os { namespace fs {

path const & process_executable_file_path() {
	struct once {
		path const static get_it() throw(std::exception) {
			#if defined DIVERSALIS__OS__MICROSOFT
				char module_file_name[UNIVERSALIS__OS__MICROSOFT__MAX_PATH];
				if(!::GetModuleFileNameA(0, module_file_name, sizeof module_file_name))
					throw
						///\todo the following is making link error on mingw
						//exceptions::runtime_error(exceptions::code_description(), UNIVERSALIS__COMPILER__LOCATION__NO_CLASS);
						std::runtime_error("could not get filename of program module");
				return path(module_file_name);
			#else
				///\todo use binreloc
				throw std::runtime_error("unimplemented");
			#endif
		}
	};
	path const static once(once::get_it());
	return once;
}

path const & home() {
	struct once {
		path const static get_it() {
			char const env_var[] = {
				#if defined DIVERSALIS__OS__MICROSOFT
					"USERPROFILE"
				#else
					"HOME"
				#endif
			};
			char const * const e(std::getenv(env_var));
			if(!e) {
				std::ostringstream s; s << "The user has no defined home directory: the environment variable " << env_var << " is not set.";
				throw
					///\todo the following is making link error on mingw
					//exceptions::runtime_error(s.str(), UNIVERSALIS__COMPILER__LOCATION__NO_CLASS);
					std::runtime_error(s.str());
			}
			return path(e);
			#if 1
				///\todo use SHGetFolderPath
			#elif 0
				// first check UserProfile
				char const * const user_profile(std::getenv("UserProfile"));
				if(user_profile) return user_profile;

				// next, check HomeDrive and HomePath
				char const * const home_drive(std::getenv("HomeDrive"));
				char const * const home_path(std::getenv("HomePath"));
				if(home_drive && home_path) {
					std::string home(home_drive);
					home += home_path;
					return home;
				}

				// next, try registry db
				HKEY hKeyRoot = HKEY_CURRENT_USER;
				LPCWSTR pszPath = L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders";
				HKEY m_hKey(0);
				LONG ReturnValue = RegOpenKeyExW (hKeyRoot, pszPath, 0L, KEY_ALL_ACCESS, &m_hKey);
				if(ReturnValue == ERROR_SUCCESS) {
					LPCWSTR pszKey = L"Personal";
					DWORD dwType;
					DWORD dwSize = MAX_PATH - 1;
					char szString[MAX_PATH + 1];
					LONG lReturn = RegQueryValueExW (m_hKey, pszKey, 0, &dwType, (BYTE *) szString, &dwSize);
					std::string sVal;
					if(lReturn == ERROR_SUCCESS) sVal = szString;
					if (m_hKey) {
						RegCloseKey (m_hKey);
						m_hKey = NULL;
					}
					return sVal;
				}
			#endif
		}
	};
	path const static once(once::get_it());
	return once;
}

path const & home_app_local(std::string const & app_name) {
	path const static once(home() / path("." + app_name));
	return once;
}

path const & home_app_roaming(std::string const & app_name) {
	path const static once(home() / path("." + app_name));
	return once;
}

}}}
