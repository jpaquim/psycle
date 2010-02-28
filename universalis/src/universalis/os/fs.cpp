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
#if defined DIVERSALIS__COMPILER__FEATURE__AUTO_LINK
	#pragma comment(lib,"shlwapi")
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

#if defined DIVERSALIS__OS__MICROSOFT
	namespace detail { namespace microsoft {
		path known_folder(int id) {
			path nvr;
			// The ever changing windows mess, changed once again in vista.
			// In vista the SHGetFolderPath(CSIDL) is compatibility wrapper for SHGetKnownFolderPath(FOLDERID).
			// CSIDL_APPDATA <==> FOLDERID_RoamingAppData
			// CSIDL_LOCAL_APPDATA <==> FOLDERID_LocalAppData
			// CSIDL_MYDOCUMENTS == CSIDL_PERSONAL <==> FOLDERID_Documents
			// CSIDL_PROFILE <==> FOLDERID_Profile
			char p[UNIVERSALIS__OS__MICROSOFT__MAX_PATH];
			if(SUCCEEDED(SHGetFolderPath(0, id, 0, 0, p))) nvr = p;
			else throw exception(UNIVERSALIS__COMPILER__LOCATION__NO_CLASS);
			return nvr;
		}
	}}
#endif

path const & home() {
	path const static once(
		#if defined DIVERSALIS__OS__MICROSOFT
			detail::microsoft::known_folder(CSIDL_PROFILE)
		#else
			std::getenv("HOME")
		#endif
	);
	return once;
}

path const & home_app_local(std::string const & app_name) {
	path const static once(
		#if defined DIVERSALIS__OS__MICROSOFT
			detail::microsoft::known_folder(CSIDL_LOCAL_APPDATA) / app_name
		#else
			home() / ("." + app_name)
		#endif
	);
	return once;
}

path const & home_app_roaming(std::string const & app_name) {
	path const static once(
		#if defined DIVERSALIS__OS__MICROSOFT
			detail::microsoft::known_folder(CSIDL_APPDATA) / app_name
		#else
			home() / ("." + app_name)
		#endif
	);
	return once;
}

}}}
