#include "../../songstructs.hpp"
#include <iostream>
#include <typeinfo>
#include <map>
#include <cstdint>
namespace psycle
{
	namespace host
	{

		const int MAX_BROWSER_PLUGINS = 2048;
		class CProgressDialog;

		class PluginInfo
		{
		public:
			PluginInfo()
				: mode(MACHMODE_UNDEFINED)
				, type(MACH_UNDEFINED)
				, allow(true)
			{
				std::memset(&FileTime, 0, sizeof FileTime);
			}
			virtual ~PluginInfo() throw()
			{
			}
			std::string dllname;
			long identifier;
			std::string error;
			MachineMode mode;
			MachineType type;
			std::string name;
			std::string vendor;
			std::string desc;
			std::string version;
			std::uint32_t APIversion;
			FILETIME FileTime;
			bool allow;
			/*
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
			*/
		};

		class CNewMachine
		{
		public:
			static void learnDllName(const std::string & fullpath, MachineType type);
			static bool lookupDllName(const std::string, std::string & result, MachineType tye,int& shellIdx);
			static void DestroyPluginInfo();
			static void LoadPluginInfo(bool verify=true);
			static bool TestFilename(const std::string & name,int shellIdx);
			static void Regenerate();
			static bool IsLoaded(){ return _numPlugins>0; }
		protected:
			static std::map<std::string,std::string> NativeNames;
			static std::map<std::string,std::string> VstNames;

		private:
			static int _numPlugins;
			static PluginInfo* _pPlugsInfo[MAX_BROWSER_PLUGINS];
			static int _numDirs;
			static void FindPlugins(int & currentPlugsCount, int & currentBadPlugsCount, std::vector<std::string> const & list, MachineType type, std::ostream & out, CProgressDialog * pProgress = 0);
			static bool LoadCacheFile(int & currentPlugsCount, int & currentBadPlugsCount, bool verify);
			static bool SaveCacheFile();
			void UpdateList(bool bInit = false);
		};

	}
}
