/***************************************************************************
	*   Copyright (C) 2007 Psycledelics Community
	*   psycle.sf.net
	*                                                                         *
	*   This program is free software; you can redistribute it and/or modify  *
	*   it under the terms of the GNU General Public License as published by  *
	*   the Free Software Foundation; either version 2 of the License, or     *
	*   (at your option) any later version.                                   *
	*                                                                         *
	*   This program is distributed in the hope that it will be useful,       *
	*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
	*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
	*   GNU General Public License for more details.                          *
	*                                                                         *
	*   You should have received a copy of the GNU General Public License     *
	*   along with this program; if not, write to the                         *
	*   Free Software Foundation, Inc.,                                       *
	*   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
	***************************************************************************/
#ifndef FILE_H
#define FILE_H

#include <string>
#include <vector>

namespace psy {
	namespace core {

		const int nFiles = 1;
		const int nDirs  = 2;
	
		class File{
		public:
			static std::string readFile(const std::string & filename);
			static std::vector<std::string> fileList( const std::string & path, int fMode = nFiles | nDirs );   
			static std::string workingDir();
			static bool fileIsReadable(const std::string & file);
			static std::string parentWorkingDir();
			static void cd( const std::string & path );
			static void cdHome();
			static std::string home();
			static std::string replaceTilde(const std::string & path);
			static std::string env(const std::string & envName);
#if defined __unix__ || defined __APPLE__
			static std::string extractFileNameFromPath( const std::string& fileName, bool slash = true );
#else
			static std::string extractFileNameFromPath( const std::string& fileName, bool slash = false );
#endif
			static std::string slash();

			static std::string replaceIllegalXmlChr( const std::string & text, bool strict = true );
		};

	}
}

#endif
