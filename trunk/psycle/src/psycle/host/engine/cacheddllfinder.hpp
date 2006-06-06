#pragma once
#include "dllfinder.hpp"
#include "machine.hpp"
#include "afxwin.h"	// For CFileFind. If an alternative method is found, this can be removed.

UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(psycle)
UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(host)

class PluginInfo
{
public:
	PluginInfo()
		: type(MACH_UNDEFINED)
		, mode(MACHMODE_UNDEFINED)
		, allow(true)
	{
		std::memset(&FileTime, 0, sizeof FileTime);
	}

	~PluginInfo() {}

	std::string dllname;
	std::string error;
	Machine::type_type type;
	Machine::mode_type mode;
	std::string name;
	std::string desc;
	std::string version;
	FILETIME FileTime;
	bool allow;
	std::string category;
#if 0
	void operator=(PluginInfo& newinfo)
	{
		mode=newinfo.mode;
		type=newinfo.type;
		strcpy(version,newinfo.version);
		strcpy(name,newinfo.name);
		strcpy(desc,newinfo.desc);
		zapArray(dllname,new char[sizeof(newinfo.dllname)+1]);
		strcpy(dllname,newinfo.dllname);
	}
	friend bool operator!=(PluginInfo& info1,PluginInfo& info2)
	{
		if ((info1.type != info2.type) ||
			(info1.mode != info2.mode) ||
			(strcmp(info1.version,info2.version) != 0 ) ||
			(strcmp(info1.desc,info2.desc) != 0 ) ||
			(strcmp(info1.name,info2.name) != 0 ) ||
			(strcmp(info1.dllname,info2.dllname) != 0)) return true;
		else return false;
	}
#endif
};


class MappedDllFinder: public DllFinder
{
public:
	///< Generates a string vector of all the dlls that exist in the directory speficied 
	///< and its subdirectories. Note that it's simply a file list. No check is done.
	class populate_plugin_list
	{
	public:
		populate_plugin_list(std::vector<std::string> & result, std::string directory)
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
						populate_plugin_list(result,sfilePath);
					}
				}
				else
				{
					CString filePath=finder.GetFilePath();
					filePath.MakeLower();
					if(filePath.Right(4) == ".dll")
					{
						std::string sfilePath = filePath;
						result.push_back(sfilePath);
					}
				}
			}
			finder.Close();
		}
	};
	MappedDllFinder();
	virtual ~MappedDllFinder();

	///< Adds the search path, and initializes any needed variable/process.
	void AddPath(std::string &path);
	///< Resets the Finder to the original state.
	void ResetFinder();
	///< searches in the map the full path for a specified dll name
	bool LookupDllPath(std::string& name, std::string& fullpath);
protected:
	///< Adds the dll name -> full path mapping to the map.
	void learnDllName(const std::string & fullpath);

protected:
	std::map<std::string,std::string> dllNames;
};


class CachedDllFinder: public DllFinder
{
public:
	CachedDllFinder();
	virtual ~CachedDllFinder();

	///< Adds the search path, and initializes any needed variable/process.
	void AddPath(std::string &path);
	///< Resets the Finder to the original state.
	void ResetFinder();
	///< searches in the map the full path for a specified dll name
	bool LookupDllPath(std::string& name, std::string& fullpath);

protected:
	///< Adds the new Plugin data to the map.
	void learnPlugin(PluginInfo &plugininfo);
	void CollectPlugins();
	///< Checks if the plugin that is going to be loaded is allowed to be loaded.
	bool TestFilename(const std::string & name);

	bool LoadCacheFile(int & currentPlugsCount, int & currentBadPlugsCount);
	bool SaveCacheFile();
protected:
	std::map<std::string,PluginInfo> dllInfo;

};

UNIVERSALIS__COMPILER__NAMESPACE__END
UNIVERSALIS__COMPILER__NAMESPACE__END