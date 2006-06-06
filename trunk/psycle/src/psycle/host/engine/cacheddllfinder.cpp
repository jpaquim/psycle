#include <packageneric/pre-compiled.private.hpp>
#include PACKAGENERIC
#include "cacheddllfinder.hpp"
#include "afxwin.h"	// For CFileFind. If an alternative method is found, this can be removed.

UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(psycle)
UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(host)


MappedDllFinder::MappedDllFinder()
{
}
MappedDllFinder::~MappedDllFinder()
{
	dllNames.clear();
}

///< Adds the search path, and initializes any needed variable/process.
void MappedDllFinder::AddPath(std::string &path)
{
	DllFinder::AddPath(path);
	populate_dll_map(path);
}
///< Resets the Finder to the original state.
void MappedDllFinder::ResetFinder()
{
	dllNames.clear();
}
///< searches in the map the full path for a specified dll name
bool MappedDllFinder::LookupDllPath(std::string& name)
{
	std::map<std::string,std::string>::iterator iterator
		= dllNames.find(name);
	if(iterator != dllNames.end())
	{
		name=iterator->second;
		return true;
	}
	return false;
}

///< Adds the dll name -> full path mapping to the map.
void MappedDllFinder::LearnDllName(const std::string & fullpath)
{
	std::string str=fullpath;
	// strip off path
	std::string::size_type pos=str.rfind('\\');
	if(pos != std::string::npos) str=str.substr(pos+1);
	// transform string to lower case
	std::transform(str.begin(),str.end(),str.begin(),std::tolower);
	dllNames[str]=fullpath;
}

void MappedDllFinder::populate_dll_map(std::string directory)
{
	::CFileFind finder;
	int loop = finder.FindFile(::CString((directory + "\\*").c_str()));
	while(loop)
	{
		loop = finder.FindNextFile();
		if(finder.IsDirectory()) {
			if(!finder.IsDots())
			{
				std::string sfilePath = finder.GetFilePath();
				populate_dll_map(sfilePath);
			}
		}
		else
		{
			CString filePath=finder.GetFilePath();
			filePath.MakeLower();
			if(filePath.Right(4) == ".dll")
			{
				const std::string sfilePath = filePath;
				LearnDllName(sfilePath);
			}
		}
	}
	finder.Close();
}


CachedDllFinder::CachedDllFinder()
{
	//Set up the cache file path
	char modulefilename[_MAX_PATH];
	GetModuleFileName(NULL,modulefilename,_MAX_PATH);
	std::string path=modulefilename;
	std::string::size_type pos=path.rfind('\\');
	if(pos != std::string::npos)
		path=path.substr(0,pos);
	cachefile=path + "\\psycle.plugin-scan.cache";

	LoadCacheFile();
}
CachedDllFinder::~CachedDllFinder()
{
	dllInfo.clear();
}

///< Adds the search path, and initializes any needed variable/process.
void CachedDllFinder::AddPath(std::string &path)
{
	DllFinder::AddPath(path);
	populate_plugin_map(path);
}
///< Resets the Finder to the original state.
void CachedDllFinder::ResetFinder()
{
	//\todo: verify that we don't need to manually delete the PluginInfo's.
	dllInfo.clear();
	DeleteFile(cachefile.c_str());
}
///< searches in the map the full path for a specified dll name
bool CachedDllFinder::LookupDllPath(std::string& name)
{
	std::map<std::string,PluginInfo>::iterator iterator
		= dllInfo.find(name);
	if(iterator != dllInfo.end())
	{
		name=iterator->second.dllname;
		if(iterator->second.allow) return true;
		std::ostringstream s; s
			<< "Plugin " << name << " is disabled because:" << std::endl
			<< iterator->second.error << std::endl
			<< "Try to load anyway?";
		return ::MessageBox(0, s.str().c_str(), "Plugin Warning!", MB_YESNO | MB_ICONWARNING) == IDYES;
	}
	return false;
}

///< Adds the new Plugin data to the map.
void CachedDllFinder::LearnPlugin(PluginInfo &plugininfo)
{
	std::string str=plugininfo.dllname;
	// strip off path
	std::string::size_type pos=str.rfind('\\');
	if(pos != std::string::npos) str=str.substr(pos+1);
	// transform string to lower case
	std::transform(str.begin(),str.end(),str.begin(),std::tolower);
	dllInfo[str]=plugininfo;
}
///< fills the dllInfo with the information of all plugins found in the directory specified 
///< and its subdirectories. If a Cache exists it will only load/fill those that are new
///< or modified since the cache creation.
void CachedDllFinder::populate_plugin_map(std::string directory)
{
	//\todo!!!
}

bool CachedDllFinder::LoadCacheFile()
{
	RiffFile file;
	CFileFind finder;

	//Open
	if (!file.Open(cachefile)) 
	{
		return false;
	}
	//Verify that it's a cache
	char Temp[MAX_PATH];
	file.ReadChunk(Temp,8);
	Temp[8]=0;
	if (strcmp(Temp,"PSYCACHE")!=0)
	{
		file.Close();
		DeleteFile(cachefile.c_str());
		return false;
	}
	//Verify version.
	UINT version;
	file.Read(version);
	if (version != CURRENT_CACHE_MAP_VERSION)
	{
		file.Close();
		DeleteFile(cachefile.c_str());
		return false;
	}
	//Read Contents
	std::uint32_t _numPlugins;
	file.Read(_numPlugins);
	for (std::uint32_t i = 0; i < _numPlugins; i++)
	{
		PluginInfo p;
		file.ReadString(Temp,sizeof(Temp));
		file.ReadChunk(&p.FileTime,sizeof(FILETIME));
		{
			//\todo: why isn't this a null terminated string!?
			UINT size;
			file.Read(size);
			if(size)
			{
				char *chars(new char[size + 1]);
				file.ReadChunk(chars, size);
				chars[size] = '\0';
				p.error = (const char*)chars;
				zapArray(chars);
			}
		}
		file.Read(p.allow);
		file.Read(p.mode);
		file.Read(p.type);
		file.ReadString(p.name);
		file.ReadString(p.desc);
		file.ReadString(p.version);
		//file.ReadString(p.category);

		//Add the plugin to our map, only if it hasn't been modified.
		//If it has, our populate_plugin_map will add it with the new information.
		if(finder.FindFile(Temp))
		{
			FILETIME time;
			finder.FindNextFile();
			if (finder.GetLastWriteTime(&time))
			{
				if(	p.FileTime.dwHighDateTime == time.dwHighDateTime &&
					p.FileTime.dwLowDateTime == time.dwLowDateTime	)
				{
					p.dllname = Temp;
					LearnPlugin(p);
				}
			}
		}
	}
	file.Close();

	return true;
}
bool CachedDllFinder::SaveCacheFile()
{
	DeleteFile(cachefile.c_str());

	//Create and initialize.
	RiffFile file;
	if(!file.Create(cachefile,true)) return false;
	file.WriteChunk("PSYCACHE",8);
	std::uint32_t version = CURRENT_CACHE_MAP_VERSION;
	file.Write(version);
	file.Write(dllInfo.size());
	//Add the plugins.
	std::map<std::string,PluginInfo>::iterator iterator;
	iterator = dllInfo.begin();
	for (; iterator != dllInfo.end();iterator++ )
	{
		file.WriteChunk(iterator->second.dllname.c_str(),iterator->second.dllname.length()+1);
		file.Write(iterator->second.FileTime);
		{
			const std::string error(iterator->second.error);
			std::uint32_t size(static_cast<int>(error.size()));
			file.Write(size);
			if(size) file.WriteChunk(error.data(), size);
		}
		file.Write(iterator->second.allow);
		file.Write(iterator->second.mode);
		file.Write(iterator->second.type);
		file.WriteChunk(iterator->second.name.c_str(),iterator->second.name.length()+1);
		file.WriteChunk(iterator->second.desc.c_str(),iterator->second.desc.length()+1);
		file.WriteChunk(iterator->second.version.c_str(),iterator->second.version.length()+1);
	}
	file.Close();
	return true;
}

UNIVERSALIS__COMPILER__NAMESPACE__END
UNIVERSALIS__COMPILER__NAMESPACE__END