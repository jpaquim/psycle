/***************************************************************************
 *   Copyright (C) 2005 by Stefan   *
 *   natti@linux   *
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

#include "nfile.h"
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <iostream>
#include <algorithm>

#ifdef __unix__
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

using namespace std;

NFile::NFile()
{
}


NFile::~NFile()
{
}

std::string NFile::readFile( const std::string & filename )
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

std::vector< std::string > NFile::fileList( const std::string & path )
{
  std::vector<std::string> destination;

  #ifdef __unix__
  
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
 
 while( (drecord = readdir(dhandle)) != NULL)
 {
  stat(drecord->d_name,&sbuf);
  if(S_ISDIR(sbuf.st_mode))
  {
    // dirs not handled here 
  } else
 {
   destination.push_back(std::string(drecord->d_name));
 }
 }
 putchar('\n');
 closedir(dhandle);
 #else
  WIN32_FIND_DATA dir;
  HANDLE fhandle;
  char directory[8196];
  // unsecure, better if there snprintf
  sprintf(directory,"%s\\*.*",path.c_str());
  // Handle to directory
  if ((fhandle=FindFirstFile(directory,&dir)) !=
                             INVALID_HANDLE_VALUE) {
    do {  // readout directory
      destination.push_back( dir.cFileName );      
    } while(FindNextFile(fhandle,&dir));
  }
  FindClose(fhandle);
 #endif
 sort(destination.begin(),destination.end());
 return destination;
}

std::string NFile::home() {
 #ifdef __unix__
 char home[8000]; 
 strncpy(home,getenv("HOME"),7999);
 return home;
 #else            
 HKEY hKeyRoot = HKEY_CURRENT_USER;
 LPCTSTR pszPath = "Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders";
 
 HKEY m_hKey = NULL;
 LONG ReturnValue = RegOpenKeyEx (hKeyRoot, pszPath, 0L,
		KEY_ALL_ACCESS, &m_hKey);
		
 if(ReturnValue == ERROR_SUCCESS)
 {
   LPCTSTR pszKey = "Personal";
   std::string sVal;
        
   DWORD dwType;
   DWORD dwSize = 200;
   char  szString[255];

   LONG lReturn = RegQueryValueEx (m_hKey, (LPSTR) pszKey, NULL,
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
 #endif
}            

void NFile::cdHome() {
 #ifdef __unix__
 char home[8000]; 
 strncpy(home,getenv("HOME"),7999);
 chdir(home); 
 #else
 cd( home() );
 #endif
}

void NFile::cd( const std::string & path )
{
 //string p = replaceTilde(path);
 chdir(path.c_str());
}


std::string NFile::workingDir( )
{
  char puffer[8000];
  if(getcwd(puffer,sizeof(puffer)) == NULL)
      {
         return "";
      }
  return std::string(puffer);
}

std::string NFile::parentWorkingDir( )
{
  string oldDir = workingDir();
  cd("..");
  string parentDir = workingDir();
  cd(oldDir);
  return parentDir;
}


std::vector< std::string > NFile::parentDirList( const std::string & path ) {
   std::string oldDir = workingDir();
   cd("..");
   string parentDir = workingDir();
   cd(oldDir);
   return dirList(parentDir);
}



std::vector< std::string > NFile::dirList( const std::string & path )
{
  std::vector<std::string> destination;
  #ifdef __unix__  
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
 
 while( (drecord = readdir(dhandle)) != NULL)
 {
  stat(drecord->d_name,&sbuf);
  if(S_ISDIR(sbuf.st_mode))
  {
    destination.push_back(std::string(drecord->d_name));
  } else
  {
    // files not handled here 
  }
 }
 putchar('\n');
 closedir(dhandle);
 #endif
 sort(destination.begin(),destination.end());
 return destination;
}

std::string NFile::replaceTilde( std::string const & path )
{
  std::string nvr(path);
  if(!path.length() || path[0] != '~') return nvr;
  nvr.replace( 0, 1, home().c_str() );
  return nvr;
}

bool NFile::fileIsReadable( const std::string & file )
{
   std::ifstream _stream (file.c_str (), std::ios_base::in | std::ios_base::binary);
   if (!_stream.is_open ()) return false;
   return true;
}

std::string NFile::env( const std::string & envName )
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

std::string NFile::extractFileNameFromPath(const std::string & fileName) {
  std::string fileWithoutPathName = "";

  int i = fileName.rfind("/");

  if (i != std::string::npos  &&  i != fileName.length() - 1 ) {
     fileWithoutPathName = fileName.substr(i+1);
  }

  return fileWithoutPathName;
}

std::string NFile::replaceIllegalXmlChr( const std::string & text, bool strict )
{
	std::string xml = text;

// replace ampersand
			unsigned int search_pos = 0;
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
