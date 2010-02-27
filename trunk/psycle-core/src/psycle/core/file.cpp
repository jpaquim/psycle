// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2009 members of the psycle project http://psycle.sourceforge.net

#include <psycle/core/config.private.hpp>
#include "file.h"

#include <diversalis.hpp>

#if defined DIVERSALIS__OS__MICROSOFT
	#include <universalis/os/include_windows_without_crap.hpp>
	#include <winreg.h>
#else
	#include <sys/types.h>
	#include <sys/stat.h>
	#include <unistd.h>
	#include <cerrno>
#endif

#if defined DIVERSALIS__COMPILER__MICROSOFT
	#include <direct.h> // Visual C++ specific
#else
	#include <dirent.h>
#endif

#include <stdexcept>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <boost/filesystem/operations.hpp>

namespace psycle { namespace core {
	std::string File::home() {
		#if defined DIVERSALIS__OS__MICROSOFT
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
			return "";
		#else
			std::string nrv;
			char const * const e(std::getenv("HOME"));
			if(e) nrv = e;
			return nrv;
		#endif
	}

	std::string File::replaceTilde(std::string const & path) {
		std::string nvr(path);
		if(!path.length() || path[0] != '~') return nvr;
		nvr.replace(0, 1, home().c_str());
		return nvr;
	}
}}
