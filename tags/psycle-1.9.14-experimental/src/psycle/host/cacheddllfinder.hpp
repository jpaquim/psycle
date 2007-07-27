#pragma once
#include <iostream>
#include <typeinfo>
#include <map>
#include <psycle/engine/dllfinder.hpp>
#include <psycle/engine/machine.hpp>
#include <sigslot/sigslot.h>

namespace psycle
{
	namespace host
	{

//std::uint32_t const CURRENT_CACHE_MAP_VERSION = 1;

class PluginInfo
{
public:
	PluginInfo()
		: subclass(MACH_UNDEFINED)
		, mode(MACHMODE_UNDEFINED)
		, allow(true)
	{
		std::memset(&FileTime, 0, sizeof FileTime);
	}

	~PluginInfo() {}

	Machine::class_type subclass;
	Machine::mode_type mode;
	std::string name;
	std::string desc;
	std::string version;
	std::string dllname;
	FILETIME FileTime;
	std::string error;
	bool allow;
	std::string category;
#if 0
	void operator=(PluginInfo& newinfo)
	{
		mode=newinfo.mode;
		subclass=newinfo.subclass;
		strcpy(version,newinfo.version);
		strcpy(name,newinfo.name);
		strcpy(desc,newinfo.desc);
		zapArray(dllname,new char[sizeof(newinfo.dllname)+1]);
		strcpy(dllname,newinfo.dllname);
	}
	friend bool operator!=(PluginInfo& info1,PluginInfo& info2)
	{
		if ((info1.subclass != info2.subclass) ||
			(info1.mode != info2.mode) ||
			(strcmp(info1.version,info2.version) != 0 ) ||
			(strcmp(info1.desc,info2.desc) != 0 ) ||
			(strcmp(info1.name,info2.name) != 0 ) ||
			(strcmp(info1.dllname,info2.dllname) != 0)) return true;
		else return false;
	}
#endif
};

class DllFileInfo
{
public:
	DllFileInfo()
	{
		memset(&_modtime,0,sizeof(FILETIME));
	}

	std::string _name;
	FILETIME _modtime;
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
	virtual void AddPath(const std::string &path,Machine::class_type subclass);
	///< Resets the Finder to the original state.
	virtual void ResetFinder();
	///< fills in the path for the specified name so that name becomes a fullpath.
	virtual bool LookupDllPath(std::string& name,Machine::class_type subclass);
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
	virtual void AddPath(const std::string &path,Machine::class_type subclass);
	///< Resets the Finder to the original state.
	virtual void ResetFinder();
	///< fills in the path for the specified name so that name becomes a fullpath.
	virtual bool LookupDllPath(std::string& name,Machine::class_type subclass);

	PluginInfo& GetPluginInfo(std::string dllname) { return dllInfo.find(dllname)->second; }
	void MoveFirst();
	void MoveNext();
	const PluginInfo* GetInfoAtPos();
	bool end() const ;
	std::uint32_t size() const ;
	void MoveFirstOf(Machine::class_type subclass);
	void MoveNextOf(Machine::class_type subclass);

	void ForceSaveOnDestroy() { bforcedsaving =true; }
public:
	sigslot::signal2<const std::string &, const std::string &> report;
	sigslot::signal2<const std::string &, const std::string &> progress;
	sigslot::signal1<const std::string &> logger;

protected:
	///< Adds the new Plugin data to the map.
	void LearnPlugin(PluginInfo &plugininfo);
	///< fills dllNames with all the dlls that exist in the specified directory.
	///< and its subdirectories. This is done in populate_plugin_map in order to
	///< know the dll's in advance.
	void populate_dll_list(std::vector<DllFileInfo>& dllNames, std::string directory);
	///< fills the dllInfo with the information of all plugins found in the directory specified 
	///< and its subdirectories. If a Cache exists it will only load/fill those that are new
	///< or modified since the cache creation.
	void populate_plugin_map(std::string directory,Machine::class_type subclass);
	void GeneratePluginInfo(PluginInfo& pinfo);
	bool LoadCacheFile();
	bool SaveCacheFile();
protected:
	std::map<std::string,PluginInfo> dllInfo;
	std::map<std::string,PluginInfo>::iterator infoit;
	std::string cachefile;
	bool bforcedsaving;

};

}
}