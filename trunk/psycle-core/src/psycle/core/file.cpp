// -*- mode:c++; indent-tabs-mode:t -*-
/******************************************************************************
*  copyright 2007 members of the psycle project http://psycle.sourceforge.net *
*                                                                             *
*  This program is free software; you can redistribute it and/or modify       *
*  it under the terms of the GNU General Public License as published by       *
*  the Free Software Foundation; either version 2 of the License, or          *
*  (at your option) any later version.                                        *
*                                                                             *
*  This program is distributed in the hope that it will be useful,            *
*  but WITHOUT ANY WARRANTY; without even the implied warranty of             *
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the              *
*  GNU General Public License for more details.                               *
*                                                                             *
*  You should have received a copy of the GNU General Public License          *
*  along with this program; if not, write to the                              *
*  Free Software Foundation, Inc.,                                            *
*  59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.                  *
******************************************************************************/


#include "file.h"

#if defined __unix__ || defined __APPLE__
	#include <sys/types.h>
	#include <sys/stat.h>
	#include <unistd.h>
#else
	#include <windows.h>
	#include <winreg.h>
#endif

#ifdef _MSC_VER
	#include <direct.h> /* Visual C++ */
#else
	#include <dirent.h>
#endif

#include <stdexcept>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cstdlib>

namespace psy { namespace core {
	using namespace std;

	char const File::path_env_var_name[] =
	{
		#if defined __unix__ || defined __APPLE__
			"LD_LIBRARY_PATH"
		#elif defined _WIN64 || defined _WIN32
			"PATH"
		#else
			#error unknown dynamic linker
		#endif
	};
	std::string File::readFile(std::string const & path) {
		std::ifstream is(path.c_str());
		if(!is) {
			std::ostringstream s;
			s << "could not open file: " << path;
			throw std::runtime_error(s.str().c_str());
		}
		std::ostringstream s;
		s << is.rdbuf();
		return s.str();
	}

	std::vector<std::string> File::fileList(std::string const & path, int list_mode) {
		std::vector<std::string> destination;
		std::string saveCurrentDir(workingDir());
		#if defined __unix__ || defined __APPLE__
			///\todo using scandir would be simpler (not in posix yet as of 2007)
			DIR * dhandle(opendir(path.c_str()));
			if(!dhandle) {
				std::ostringstream s;
				s << "could not open directory: " << path;
				throw std::runtime_error(s.str());
			}
			int x(chdir(path.c_str()));
			if(x) {
				std::ostringstream s;
				s << "could not enter directory: " << path;
				throw std::runtime_error(s.str());
			}
			dirent * drecord;
			while((drecord = readdir(dhandle)) != 0) {
				struct stat sbuf;
				stat(drecord->d_name, &sbuf);
				std::string name(drecord->d_name);
				if(
					((list_mode & list_modes::dirs)
						&& S_ISDIR(sbuf.st_mode)
						&& ( name != "." && name != "..")) ||
					((list_mode & list_modes::files)
						&& !S_ISDIR(sbuf.st_mode)) 
				){
					destination.push_back(name);
				}
			}
			closedir(dhandle);
		#else
			WIN32_FIND_DATAA dir;
			HANDLE fhandle;
			char directory[8196];
			// unsecure, better using snprintf
			sprintf(directory,"%s\\*.*",path.c_str());
			// Handle to directory
			if((fhandle=FindFirstFileA(directory,&dir)) != INVALID_HANDLE_VALUE) {
				do {  // readout directory
					if(
						((list_mode & list_modes::dirs) && (dir.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) || 
						((list_mode & list_modes::files) && !(dir.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
					) destination.push_back( dir.cFileName );
				} while(FindNextFileA(fhandle,&dir));
			}
			FindClose(fhandle);
		#endif
		sort(destination.begin(),destination.end());
		cd(saveCurrentDir);
		return destination;
	}

	std::string File::home() {
		#if defined __unix__ || defined __APPLE__
			std::string nrv(env("HOME"));
			return nrv;
		#else
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
		#endif
	}

	void File::cdHome() {
		cd(home());
	}

	void File::cd(std::string const & path) {
		chdir(path.c_str());
	}

	std::string File::workingDir() {
		std::string nvr;
		char buffer[8000];
		if(getcwd(buffer,sizeof buffer)) nvr = buffer;
		return nvr;
	}
	
	
	std::string File::appendDirToEnvPath(std::string const & path) {

		// append the plugin dir to the path env var
		std::string new_path(getEnvPath());
		if(new_path.length()) {
			new_path +=
				#if defined __unix__ || defined __APPLE__
					":";
				#elif defined _WIN64 || defined _WIN32
					";";
				#else
					#error unknown dynamic linker
				#endif
		}
		new_path += path;
		if ( setEnvPath(new_path) == true ) {
			return new_path;
		} else {
			return getEnvPath();
		}
	}
	
	bool File::setEnvPath(std::string const & new_path) {
		// append the plugin dir to the path env var
		std::string pathToSet = path_env_var_name;
		pathToSet += "=" + new_path;
		if(::putenv(pathToSet.c_str())) {
			std::cerr << "psycle: plugin: warning: could not alter " << path_env_var_name << " env var.\n";
			return false;
		}
		return true;
	}
	
	std::string File::getEnvPath() {
		std::string path;
		char const * const env(std::getenv(path_env_var_name));

		if(env) path = env;

		return path;
	}

	std::string File::parentWorkingDir() {
		string oldDir(workingDir());
		cd("..");
		string parentDir(workingDir());
		cd(oldDir);
		return parentDir;
	}

	std::string File::replaceTilde(std::string const & path) {
		std::string nvr(path);
		if(!path.length() || path[0] != '~') return nvr;
		nvr.replace(0, 1, home().c_str());
		return nvr;
	}

	std::string const & File::slash() {
		std::string const static once(
			#if defined __unix__ || defined __APPLE__
				"/"
			#else
				"\\"
			#endif
		);
		return once;
	}
	
	void File::ensurePathTerminated(std::string & path) {
		if(!path.length()) path = slash();
		if(path[path.length() - 1] != slash()[0]) path += slash();
	}

	bool File::fileIsReadable(std::string const & file) {
		std::ifstream stream_(file.c_str(), std::ios_base::in | std::ios_base::binary);
		return stream_.is_open();
	}

	std::string File::env(std::string const & envName) {
		std::string nvr;
		char const * const value(std::getenv(envName.c_str()));
		if(value) nvr = value;
		return nvr;
	}

	std::string File::extractFileNameFromPath(std::string const & fileName) {
		std::string nvr;
		std::string::size_type i(fileName.rfind(slash()));
		if(i != std::string::npos && i != fileName.length() - 1) nvr = fileName.substr(i + 1);
		return nvr;
	}

	/// replaces with xml entities for xml writing.
	// There are 5 predefined entity references in XML:
	// &lt;   < less than 
	// &gt;   > greater than
	// &amp;  & ampersand 
	// &apos; ' apostrophe
	// &quot; " quotation mark
	// Only the characters "<" and "&" are strictly illegal in XML. Apostrophes, quotation marks and greater than signs are legal. strict = true  replaces all.
	std::string File::replaceIllegalXmlChr( const std::string & text, bool strict ) {
		std::string xml = text;
		std::string::size_type search_pos = 0;
		// replace ampersand
		while((search_pos = xml.find("&", search_pos)) != std::string::npos) xml.replace(search_pos++, 1, "&amp;" );
		// replace less than
		while((search_pos = xml.find("<")) != std::string::npos) xml.replace(search_pos, 1, "&lt;" );
		if(strict) {
			// replace greater than
			while((search_pos = xml.find(">") ) != std::string::npos) xml.replace(search_pos, 1, "&gt;" );
			// replace apostrophe
			while((search_pos = xml.find("'") ) != std::string::npos) xml.replace(search_pos, 1, "&apos;" );
			// replace quotation mark
			while((search_pos = xml.find("\"") ) != std::string::npos) xml.replace(search_pos, 1, "&quot;" );
		}
		return xml;
	}
}}
