// -*- mode:c++; indent-tabs-mode:t -*-
/**************************************************************************
*   Copyright 2007 Psycledelics http://psycle.sourceforge.net             *
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
#include <psycle/core/psycleCorePch.hpp>
#include "plugincatcher.h"
#include "file.h"
#include "fileio.h"
#include <iostream> // only for debug output
#include <sstream>

namespace psy { namespace core {


PluginFinderCache::PluginFinderCache()
:_numPlugins(0)
{
	loadCache();
}

PluginFinderCache::~PluginFinderCache()
{
}

bool PluginFinderCache::loadCache(){
#if 0
	//FIXME:std::string cache((universalis::operating_system::paths::package::home() / "psycle.plugin-scan.cache").native_file_string());
	std::string cache(File::home() + "/psycle.plugin-scan.cache");

	RiffFile file;

	if (!file.Open(cache.c_str()))
		return false;
	
	char Temp[1024];
	file.ReadArray(Temp,8);
	Temp[8]=0;
	if (strcmp(Temp,"PSYCACHE")!=0)
	{
		file.Close();
		deleteCache();
		return false;
	}
	
	std::uint32_t version;
	file.Read(version);
	if (version != CURRENT_CACHE_MAP_VERSION)
	{
		file.Close();
		deleteCache();
		return false;
	}
	
	std::uint32_t fileNumPlugs;
	file.Read(fileNumPlugs);
	for (std::uint32_t i = 0; i < fileNumPlugs; i++)
	{
		PluginInfo p;
		file.ReadString(Temp,sizeof(Temp));
		{
			//FIXME: types
			time_t filetime;
			//file.ReadArray(&filetime,sizeof(filetime));p.setFileTime(filetime);
			std::uint32_t size;
			file.Read(size);
			if(size)
			{
				char *chars(new char[size + 1]);
				file.ReadArray(chars, size);
				chars[size] = '\0';
				p.setError((const char*)chars);
				delete [] chars;
			}
		}
		{
			std::string s_temp;
			bool b_temp;
			std::int32_t identifier;
			MachineType mtype;
			MachineMode mmode;
			//FIXME: types
			file.Read(b_temp); p.setAllow(b_temp);
			//file.ReadArray(&mmode,sizeof(mmode)); p.setMode(mmode);
			//file.ReadArray(&mtype,sizeof(mtype)); p.setType(mtype);
			file.ReadString(s_temp); p.setName(s_temp);
			file.Read(identifier);
			file.ReadString(s_temp); p.setAuthor(s_temp);
			file.ReadString(s_temp); p.setDesc(s_temp);
			file.ReadString(s_temp); p.setVersion(s_temp);
		}

//FIXME:Unfinished
		// Temp here contains the full path to the .dll
		/*if(finder.FindFile(Temp))
		{
			time_t t_time;
			finder.FindNextFile();
			if (finder.GetLastWriteTime(&time))
			{
				// Only add the information to the cache if the dll hasn't been modified (say, a new version)
				// Else, we want to get the new information, and that will happen in the plugins scan.
				if ( p.fileTime() == t_time )
				{
					p.setLibName( Temp );
					PluginFinderKey key(fileName, ladspa_path + File::slash() + fileName, identifier);
					map_[key] = info;
					_numPlugins++;

				}
			}
		}
*/
	}
	
	file.Close();
#endif
	return true;
}
bool PluginFinderCache::saveCache(){
	return false;
}
void PluginFinderCache::deleteCache(){
}


}}

