// This program is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
// You should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
// copyright 2007-2009 members of the psycle project http://psycle.sourceforge.net

#include <psycle/core/config.private.hpp>
#include "plugincatcher.h"
#include <universalis/os/paths.hpp>

#include "file.h"
#include "fileio.h"
#include <iostream> // only for debug output
#include <sstream>

namespace psy { namespace core {


PluginFinderCache::PluginFinderCache()
:_numPlugins(0)
{
}

PluginFinderCache::~PluginFinderCache()
{
}
void PluginFinderCache::Initialize(bool clear) {
	if (clear) {
		deleteCache();
	}
	loadCache();
}
void PluginFinderCache::EnablePlugin(const MachineKey & key, bool enable) {
	PluginFinder::EnablePlugin(key, enable);
	saveCache();
}

bool PluginFinderCache::loadCache(){
#if 0
	std::string cache((universalis::os::paths::package::home() / "plugin-scan.cache").native_file_string());
	RiffFile file;
	CFileFind finder;

	char temp[9];
	std::uint32_t version;
	std::uint32_t fileNumPlugs;

	file.ReadArray(temp,8);
	temp[8]=0;
	if (strcmp(temp,"PSYCACHE")!=0)
	{
		file.Close();
		deleteCache();
		return false;
	}

	file.Read(version);
	if (version != CURRENT_CACHE_MAP_VERSION)
	{
		file.Close();
		DeleteFile(cache.c_str());
		return false;
	}

	file.Read(fileNumPlugs);
	for (std::uint32_t i = 0; i < fileNumPlugs; i++)
	{
		PluginInfo p;
		Hosts::type host;
		int index;

		file.ReadString(temp,sizeof(temp));

		{
			time_t filetime;
			std::string error_msg;

			file.Read(filetime); p.setFileTime(filetime);
			file.ReadString(error_msg); p.setError(error_msg);
		}
		{
			std::string s_temp;
			bool b_temp;
			MachineRole::type rl_temp;
			file.Read(host);
			file.Read(index);
			file.Read(b_temp); p.setAllow(b_temp);
			file.Read(rl_temp); p.setRole(rl_temp);
			file.ReadString(s_temp); p.setName(s_temp);
			file.ReadString(s_temp); p.setAuthor(s_temp);
			file.ReadString(s_temp); p.setDesc(s_temp);
			file.ReadString(s_temp); p.setVersion(s_temp);
		}

		// Temp here contains the full path to the .dll
		if(finder.FindFile(temp))
		{
			time_t t_time;
			finder.FindNextFile();
			if (finder.GetLastWriteTime(&t_time))
			{
				// Only add the information to the cache if the dll hasn't been modified (say, a new version)
				// Else, we want to get the new information, and that will happen in the plugins scan.
				if ( p.fileTime() == t_time )
				{
					MachineKey key( host, temp, index);
					AddInfo( key, p);
				}
			}
		}
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
