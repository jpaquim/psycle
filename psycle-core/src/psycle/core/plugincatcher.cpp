// This program is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
// You should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
// copyright 2007-2009 members of the psycle project http://psycle.sourceforge.net

#include <psycle/core/config.private.hpp>
#include "plugincatcher.h"
#include "file.h"
#include "fileio.h"
#include <universalis/os/paths.hpp>
#include <boost/filesystem/operations.hpp>
#include <iostream> // only for debug output
#include <sstream>

namespace psy { namespace core {



PluginFinderCache::PluginFinderCache(bool delayedScan)
:PluginFinder(delayedScan), _numPlugins(0)
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
	///\todo: Implement this with a better structure (My plan was to use
	// the new riff classes on the helpers project, but they are unfinished)
	char temp[9];
	std::uint32_t version;
	std::uint32_t fileNumPlugs;
	boost::filesystem::path cache(universalis::os::paths::package::home() / "plugin-scan-v2.cache");
	RiffFile file;

	if (!file.Open(cache.native_file_string().c_str()))
	{
		return false;
	}

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
		deleteCache();
		return false;
	}

	file.Read(fileNumPlugs);
	for (std::uint32_t i = 0; i < fileNumPlugs; i++)
	{
		PluginInfo p;

		std::string fullPath;
		file.ReadString(fullPath); p.setLibName(fullPath);
		
		{ time_t filetime; file.Read(filetime); p.setFileTime(filetime); }
		{ std::string error_msg; file.ReadString(error_msg); p.setError(error_msg); }

		Hosts::type host;
		{ std::uint8_t host_int; file.Read(host_int); host = Hosts::type(host_int); }

		int index;
		file.Read(index);
		
		{ bool allow; file.Read(allow); p.setAllow(allow); }
		{ std::uint8_t role_int; file.Read(role_int); p.setRole(MachineRole::type(role_int)); }

		{ std::string s_temp;
			file.ReadString(s_temp); p.setName(s_temp);
			file.ReadString(s_temp); p.setAuthor(s_temp);
			file.ReadString(s_temp); p.setDesc(s_temp);
			file.ReadString(s_temp); p.setVersion(s_temp);
		}

		// Temp here contains the full path to the .dll
		if(File::fileIsReadable(fullPath)) {
			///\todo: implement modification time checking.
			#if 0
				time_t t_time;
				finder.FindNextFile();
				if(finder.GetLastWriteTime(&t_time)) {
					// Only add the information to the cache if the dll hasn't been modified (say, a new version)
					// Else, we want to get the new information, and that will happen in the plugins scan.
					if(p.fileTime() == t_time) {
						MachineKey key( host, File::extractFileNameFromPath(fullPath) , index);
						AddInfo( key, p);
					}
				}
			#endif
			MachineKey key(host, File::extractFileNameFromPath(fullPath), index);
			if(!hasHost(host)) addHost(host);
			AddInfo(key, p);
		}
	}

	file.Close();
	return true;
}
bool PluginFinderCache::saveCache(){
	deleteCache();

	boost::filesystem::path cache(universalis::os::paths::package::home() / "plugin-scan-v2.cache");
	RiffFile file;
	if (!file.Create(cache.native_file_string().c_str(),true)) 
	{
		boost::filesystem::create_directory(cache.branch_path());
		if (!file.Create(cache.native_file_string().c_str(),true)) return false;
	}
	file.WriteArray("PSYCACHE",8);
	std::uint32_t version = CURRENT_CACHE_MAP_VERSION;
	file.Write(version);
	
	std::uint32_t fileNumPlugs = 0;
	
	for(std::uint32_t numHost = 0; hasHost(Hosts::type(numHost)); numHost++) {
		fileNumPlugs += size(Hosts::type(numHost));
	}
	file.Write(fileNumPlugs);

	for(std::uint32_t numHost = 0; hasHost(Hosts::type(numHost)); numHost++) {
		PluginFinder::const_iterator iter = begin(Hosts::type(numHost));
		while(iter != end(Hosts::type(numHost))) {
			PluginInfo info =iter->second;
			file.WriteString(info.libName());
			file.Write(info.fileTime());
			file.WriteString(info.error());
			file.Write(iter->first.host());
			file.Write(iter->first.index());
			file.Write(info.allow());
			file.Write((std::uint32_t)info.role());
			file.WriteString(info.name());
			file.WriteString(info.author());
			file.WriteString(info.desc());
			file.WriteString(info.version());
			iter++;
		}
	}
	file.Close();
	return true;
}
void PluginFinderCache::deleteCache(){
	boost::filesystem::path cache(universalis::os::paths::package::home() / "plugin-scan-v2.cache");
	File::unlink(cache.native_file_string());
}

void PluginFinderCache::PostInitialization() {
	saveCache();
}

}}
