#pragma once
#include "dllfinder.hpp"
#include "machine.hpp"

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
	};
	MappedDllFinder();
	virtual ~MappedDllFinder();

	///< Adds the search path, and initializes any needed variable/process.
	virtual void AddPath(std::string &path);
	///< Resets the Finder to the original state.
	virtual void ResetFinder();
	///< fills in the path for the specified name so that name becomes a fullpath.
	virtual bool LookupDllPath(std::string& name);
protected:
	///< Adds the dll name -> full path mapping to the map.
	void LearnDllName(const std::string & fullpath);
	///< fills the dllNames with all the dlls that exist in the directory specified 
	///< and its subdirectories. Note that it's simply a file list. No check is done.
	void populate_dll_map(std::string directory);

protected:
	std::map<std::string,std::string> dllNames;
};


class CachedDllFinder: public DllFinder
{
public:
	CachedDllFinder();
	virtual ~CachedDllFinder();

	///< Adds the search path, and initializes any needed variable/process.
	virtual void AddPath(std::string &path);
	///< Resets the Finder to the original state.
	virtual void ResetFinder();
	///< fills in the path for the specified name so that name becomes a fullpath.
	virtual bool LookupDllPath(std::string& name);

protected:
	///< Adds the new Plugin data to the map.
	void LearnPlugin(PluginInfo &plugininfo);
	///< fills the dllInfo with the information of all plugins found in the directory specified 
	///< and its subdirectories. If a Cache exists it will only load/fill those that are new
	///< or modified since the cache creation.
	void populate_plugin_map(std::string directory);

	bool LoadCacheFile();
	bool SaveCacheFile();
protected:
	std::map<std::string,PluginInfo> dllInfo;
	std::string cachefile;

};

UNIVERSALIS__COMPILER__NAMESPACE__END
UNIVERSALIS__COMPILER__NAMESPACE__END