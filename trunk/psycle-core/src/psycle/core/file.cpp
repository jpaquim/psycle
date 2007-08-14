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

#include "file.h"
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <iostream>
#include <algorithm>

#if defined __unix__ || defined __APPLE__
	#include <sys/types.h>
	#include <sys/stat.h>
	#include <unistd.h>
#else
	#include <windows.h>
	#include <winreg.h>
#endif

#ifdef  _MSC_VER
	#include <direct.h> /* Visual C++ */
#else
	#include <dirent.h>
#endif

namespace psy {
	namespace core {
		using namespace std;

		std::string File::readFile( const std::string & filename )
		{
			std::stringstream buf;
			std::ifstream file(filename.c_str());
			if (!file) {
				std::cerr << "\nFile not open!\n" << std::ends;
				return "";
			}
			buf << file.rdbuf();
			return buf.str();
		}

		std::vector< std::string > File::fileList( const std::string & path, int fMode )
		{
			std::string saveCurrentDir = workingDir();

			std::vector<std::string> destination;
#if defined __unix__ || defined __APPLE__

			DIR *dhandle;
			struct dirent *drecord;
			struct stat sbuf;
			int x;

			dhandle = opendir(path.c_str());
			if(dhandle == NULL)
			{
				//printf("Error opening directory '%s'\n",path);
				return destination;
			}
			x = chdir(path.c_str());
			if( x != 0)
			{
				//printf("Error changing to '%s'\n",path);
				return destination;
			}

			while( (drecord = readdir(dhandle)) != NULL )
			{
				stat(drecord->d_name,&sbuf);
				if ( S_ISDIR(sbuf.st_mode) && (fMode & nDirs) )
				{
					// dir handled here
					destination.push_back(std::string(drecord->d_name));
				} 
				if (fMode & nFiles)
				{
					// files handled here
					destination.push_back(std::string(drecord->d_name));
				}
			} 
			putchar('\n');
			closedir(dhandle);
#else
			WIN32_FIND_DATAA dir;
			HANDLE fhandle;
			char directory[8196];
			// unsecure, better if there snprintf
			sprintf(directory,"%s\\*.*",path.c_str());
			// Handle to directory
			if ((fhandle=FindFirstFileA(directory,&dir)) != INVALID_HANDLE_VALUE) {
				do {  // readout directory
					if (
						( ( fMode & nDirs ) && ( dir.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) )
						|| 
						( ( fMode & nFiles ) && !( dir.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) ) 
						)
					{
						destination.push_back( dir.cFileName );
					}
				} while(FindNextFileA(fhandle,&dir));
			}
			FindClose(fhandle);
#endif
			sort(destination.begin(),destination.end());
			cd ( saveCurrentDir );
			return destination;
		}

		std::string File::home() 
		{
#if defined __unix__ || defined __APPLE__
			char home[8000]; 
			strncpy(home,getenv("HOME"),7999);
			return home;
#else            
			// first check homepath
			const char* homepath = getenv("HOMEPATH");
			if ( homepath ) {
				std::cout << "the homepath is " << std::string( homepath ) << std::endl;
				return std::string( homepath );
			}


			HKEY hKeyRoot = HKEY_CURRENT_USER;
			LPCWSTR pszPath = L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders";

			HKEY m_hKey = NULL;
			LONG ReturnValue =  RegOpenKeyExW (hKeyRoot, pszPath, 0L,
								KEY_ALL_ACCESS, &m_hKey);

			if(ReturnValue == ERROR_SUCCESS)
			{
				LPCWSTR pszKey = L"Personal";
				std::string sVal;

				DWORD dwType;
				DWORD dwSize = 200;
				char  szString[255];

				LONG lReturn = RegQueryValueExW (m_hKey, pszKey, NULL,
									&dwType, (BYTE *) szString, &dwSize);

				if(lReturn == ERROR_SUCCESS)
				{
					sVal = szString;
				}

				if (m_hKey)
				{
					RegCloseKey (m_hKey);
					m_hKey = NULL;
				}
				return sVal;
			}    
			return "";
#endif
		}            

		void File::cdHome() { 
			cd( home() );
		}

		void File::cd( const std::string & path )
		{
			chdir( path.c_str() );
		}


		std::string File::workingDir( )
		{
			char puffer[8000];
			if(getcwd(puffer,sizeof(puffer)) == NULL)
			{
				return "";
			}
			return std::string(puffer);
		}

		std::string File::parentWorkingDir( )
		{
			string oldDir = workingDir();
			cd("..");
			string parentDir = workingDir();
			cd(oldDir);
			return parentDir;
		}

		std::string File::replaceTilde( std::string const & path )
		{
			std::string nvr(path);
			if(!path.length() || path[0] != '~') return nvr;
			nvr.replace( 0, 1, home().c_str() );
			return nvr;
		}

		bool File::fileIsReadable( const std::string & file )
		{
			std::ifstream _stream (file.c_str (), std::ios_base::in | std::ios_base::binary);
			if (!_stream.is_open ()) return false;
			return true;
		}

		std::string File::env( const std::string & envName )
		{
#if 1
			char env[8000];
			strncpy(env,getenv(envName.c_str()),7999);
			return std::string(env);
#else // no length limit
			char const * const value(std::getenv(envName.c_str()));
			if(!value)
			{
				std::string nvr;
				return nvr;
			}
			else
			{
				std::string nvr(value);
				return nvr;
			}
#endif
		}

		std::string File::extractFileNameFromPath(const std::string & fileName, bool slash) {
			std::string fileWithoutPathName = "";

			std::string::size_type i = std::string::npos;
			if ( slash )
				i = fileName.rfind("/");
			else
				i = fileName.rfind("\\");

			if ( i != std::string::npos  &&  i != fileName.length() - 1 ) {
				fileWithoutPathName = fileName.substr(i+1);
			}

			return fileWithoutPathName;
		}


		// replaces entitys`s for xml writing
		// There are 5 predefined entity references in XML:
		// &lt;                 <               less than 
		// &gt;                 >               greater than
		// &amp;                &               ampersand 
		// &apos;               '               apostrophe
		// &quot;               "               quotation mark
		// Only the characters "<" and "&" are strictly illegal in XML. Apostrophes, quotation marks and greater than signs are legal. strict = true  replaces all.
		std::string File::replaceIllegalXmlChr( const std::string & text, bool strict )
		{
			std::string xml = text;

			// replace ampersand
			std::string::size_type search_pos = 0;
			while ( ( search_pos = xml.find("&", search_pos) ) != std::string::npos )
				xml.replace(search_pos++, 1, "&amp;" );

			// replace less than
			while ( ( search_pos = xml.find("<") ) != std::string::npos )
				xml.replace(search_pos, 1, "&lt;" );

			if ( strict ) {
				// replace greater than
				while ( ( search_pos = xml.find(">") ) != std::string::npos )
					xml.replace(search_pos, 1, "&gt;" );
				// replace apostrophe
				while ( ( search_pos = xml.find("'") ) != std::string::npos )
					xml.replace(search_pos, 1, "&apos;" );
				// replace quotation mark
				while ( ( search_pos = xml.find("\"") ) != std::string::npos ) 
					xml.replace(search_pos, 1, "&quot;" );
			}
			return xml;
		}

		std::string File::slash() {
#if defined __unix__ || defined __APPLE__
			return "/";
#else
			return "\\";
#endif
		}

	}
}
