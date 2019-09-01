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
#pragma once
#ifndef PSYCLE__CORE__FILE
#define PSYCLE__CORE__FILE
#include <vector>
#include <string>
namespace psy { namespace core {
	/// misc file operations
	class File {
		public:
			/// reads an ascii text file
			static std::string readFile(std::string const & path);
			struct list_modes {
				int const static files = 1;
				int const static dirs = 2;
			};
			/// lists the file in the dir
			static std::vector<std::string> fileList(std::string const & path, int list_mode = list_modes::files | list_modes::dirs );   
			/// the current dir
			static std::string workingDir();
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
	};
}}
#endif
