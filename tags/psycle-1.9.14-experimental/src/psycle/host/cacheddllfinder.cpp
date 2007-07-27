#include <packageneric/pre-compiled.private.hpp>
#include <packageneric/module.private.hpp>
#include "cacheddllfinder.hpp"
#include "afxwin.h"	// For CFileFind. If an alternative method is found, this can be removed.
#include <psycle/engine/plugin.hpp>
#include <psycle/engine/VSTHost.hpp>
#include <psycle/engine/FileIO.hpp>

namespace psycle
{
	namespace host
	{

MappedDllFinder::MappedDllFinder()
{
}
MappedDllFinder::~MappedDllFinder()
{
	dllNames.clear();
}

///< Adds the search path, and initializes any needed variable/process.
void MappedDllFinder::AddPath(const std::string &path,Machine::class_type subclass)
{
	DllFinder::AddPath(path,subclass);
	populate_dll_map(path);
}
///< Resets the Finder to the original state.
void MappedDllFinder::ResetFinder()
{
	dllNames.clear();
}
///< searches in the map the full path for a specified dll name
bool MappedDllFinder::LookupDllPath(std::string& name,Machine::class_type subclass)
{
	std::transform(name.begin(),name.end(),name.begin(),std::tolower);
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
	std::string str=FileFromFullpath(fullpath);
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
	bforcedsaving=false;
	LoadCacheFile();
}
CachedDllFinder::~CachedDllFinder()
{
	if (bforcedsaving) SaveCacheFile();
	dllInfo.clear();
}

///< Adds the search path, and initializes any needed variable/process.
void CachedDllFinder::AddPath(const std::string &path,Machine::class_type subclass)
{
	//\todo: check if the directory already exists? (this could be done
	// inside the base class and return a value)
	DllFinder::AddPath(path,subclass);
	populate_plugin_map(path,subclass);
}
///< Resets the Finder to the original state.
void CachedDllFinder::ResetFinder()
{
	DllFinder::ResetFinder();
	//\todo: verify that we don't need to manually delete the PluginInfo's.
	dllInfo.clear();
	DeleteFile(cachefile.c_str());
}
///< searches in the map the full path for a specified dll name
bool CachedDllFinder::LookupDllPath(std::string& name,Machine::class_type subclass)
{
	std::transform(name.begin(),name.end(),name.begin(),std::tolower);
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

void CachedDllFinder::MoveFirst()
{
	infoit = dllInfo.begin();
}
void CachedDllFinder::MoveNext()
{
	infoit++;
}
const PluginInfo* CachedDllFinder::GetInfoAtPos()
{
	if ( infoit != dllInfo.end())
	{
		return &infoit->second;
	}
	else return 0;
}
bool CachedDllFinder::end() const
{
	return (infoit == dllInfo.end());
}
std::uint32_t CachedDllFinder::size() const
{
	return dllInfo.size();

}
void CachedDllFinder::MoveFirstOf(Machine::class_type subclass)
{
	MoveFirst();
	while (infoit != dllInfo.end() && infoit->second.subclass != subclass)
	{
		infoit++;
	}
}
void CachedDllFinder::MoveNextOf(Machine::class_type subclass)
{
	MoveNext();
	while (infoit != dllInfo.end() && infoit->second.subclass != subclass)
	{
		infoit++;
	}
}



///< Adds the new Plugin data to the map.
void CachedDllFinder::LearnPlugin(PluginInfo &plugininfo)
{
	std::string str=FileFromFullpath(plugininfo.dllname);
	dllInfo[str]=plugininfo;
}

///< fills dllNames with all the dlls that exist in the specified directory.
///< and its subdirectories. This is done in populate_plugin_map in order to
///< know the dll's in advance.
void CachedDllFinder::populate_dll_list(std::vector<DllFileInfo>& dllNames, std::string directory)
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
				populate_dll_list(dllNames,sfilePath);
			}
		}
		else
		{
			CString filePath=finder.GetFilePath();
			filePath.MakeLower();
			if(filePath.Right(4) == ".dll")
			{
				DllFileInfo finfo;
				finder.GetLastWriteTime(&(finfo._modtime));
				finfo._name = filePath;
				dllNames.push_back(finfo);
			}
		}
	}
	finder.Close();
}

///< fills the dllInfo with the information of all plugins found in the directory specified 
///< and its subdirectories. If a Cache exists it will only load/fill those that are new
///< or modified since the cache creation.
void CachedDllFinder::populate_plugin_map(std::string directory,Machine::class_type subclass)
{
	std::vector<DllFileInfo> dllList;

	populate_dll_list(dllList,directory);
	//\todo: add the progress signal.
	for(int i(0) ; i < (int)dllList.size(); i++)
	{
		bool found(false);
		logger.emit(dllList[i]._name);
		std::map<std::string,PluginInfo>::iterator iterator
			= dllInfo.find(FileFromFullpath(dllList[i]._name));
		if(iterator != dllInfo.end())
		{
			if ( iterator->second.FileTime.dwHighDateTime ==  dllList[i]._modtime.dwHighDateTime &&
				iterator->second.FileTime.dwLowDateTime  ==  dllList[i]._modtime.dwLowDateTime )
			{
				found = true;
				const std::string error(iterator->second.error);
				std::stringstream s;
				if(error.empty())
					s << "found in cache.";
				else
					s << "cache says it has previously been disabled because:" << std::endl << error << std::endl;
				logger.emit(s.str());
				break;
			}
			//else, we try to load it again, so found=false;
		}
		if (!found) 
		{
			logger.emit("new plugin added to cache ; ");
			PluginInfo pinfo;
			pinfo.dllname = dllList[i]._name;
			pinfo.FileTime = dllList[i]._modtime;
			pinfo.subclass = subclass;
			GeneratePluginInfo(pinfo);
			LearnPlugin(pinfo);
			bforcedsaving=true;
		}
	}
	if (bforcedsaving) { SaveCacheFile(); bforcedsaving=false; }
}

void CachedDllFinder::GeneratePluginInfo(PluginInfo& pinfo)
{
	try
	{
		if(pinfo.subclass == MACH_PLUGIN)
		{
			//\todo: BIG TODO! the try{} catch doesn't work right now because the exceptions are already catched in LoadDll!
			//We need a way to work around this.
			Plugin plug(0);
			try
			{
				if( plug.LoadDll(pinfo.dllname) )
				{
					plug.Init(); // [bohan] hmm, we should get rid of two-stepped constructions.
				}
				else pinfo.error = "Loading Error";
			}
			catch(const std::exception & e)
			{
				std::ostringstream s; s << typeid(e).name() << std::endl;
				if(e.what()) s << e.what(); else s << "no message"; s << std::endl;
				pinfo.error = s.str();
			}
			catch(...)
			{
				std::ostringstream s;
				s << "Type of exception is unknown, cannot display any further information." << std::endl;
				pinfo.error = s.str();
			}
			if(!pinfo.error.empty())
			{
				std::ostringstream s; s
					<< "### ERRONEOUS ###" << std::endl
					<< pinfo.error;
				logger.emit(s.str());
				pinfo.allow = false;
				pinfo.name = pinfo.dllname;
				pinfo.desc = "???";
				pinfo.version = "???";
			}
			else
			{
				pinfo.allow = true;
				pinfo.name = plug.GetBrand();
				{
					std::ostringstream s; s << (plug.IsSynth() ? "Psycle instrument" : "Psycle effect") << " by " << plug.GetVendorName();
					pinfo.desc = s.str();
				}
				{
					std::ostringstream s; s << plug.GetInfo()->Version; // API VERSION
					pinfo.version = s.str();
				}
				if(plug.IsSynth()) pinfo.mode = MACHMODE_GENERATOR;
				else pinfo.mode = MACHMODE_FX;
			}
			// [bohan] plug is a stack object, so its destructor is called
			// [bohan] at the end of its scope (this scope actually).
			// [bohan] The problem with destructors of any object of any class is that
			// [bohan] they are never allowed to throw any exception.
			// [bohan] So, we catch exceptions here by calling plug.Free(); explicitly.
			try
			{
				plug.Free();
			}
			catch(const std::exception & e)
			{
				std::stringstream s;
				s
					<< std::endl
					<< "### ERRONEOUS ###" << std::endl
					<< "Exception occured while trying to free the temporary instance of the plugin." << std::endl
					<< "This plugin will not be disabled, but you might consider it unstable." << std::endl
					<< typeid(e).name() << std::endl;
				if(e.what()) s << e.what(); else s << "no message"; s << std::endl;
				logger.emit(s.str());
			}
			catch(...)
			{
				std::stringstream s;
				s
					<< std::endl
					<< "### ERRONEOUS ###" << std::endl
					<< "Exception occured while trying to free the temporary instance of the plugin." << std::endl
					<< "This plugin will not be disabled, but you might consider it unstable." << std::endl
					<< "Type of exception is unknown, no further information available.";
				logger.emit(s.str());
			}
		}
		else if(pinfo.subclass == MACH_VST)
		{
			vst::plugin vstPlug(MACH_VST, MACHMODE_UNDEFINED, Machine::id_type());
			try
			{
				vstPlug.Instance(pinfo.dllname);
			}
			catch(const std::exception & e)
			{
				std::ostringstream s; s << typeid(e).name() << std::endl;
				if(e.what()) s << e.what(); else s << "no message"; s << std::endl;
				pinfo.error = s.str();
			}
			catch(...)
			{
				std::ostringstream s; s << "Type of exception is unknown, cannot display any further information." << std::endl;
				pinfo.error = s.str();
			}
			if(!pinfo.error.empty())
			{
				std::ostringstream s; s
					<< "### ERRONEOUS ###" << std::endl
					<< pinfo.error;
				logger.emit(s.str());
				pinfo.allow = false;
				pinfo.name = pinfo.dllname;
				pinfo.desc = "???";
				pinfo.version = "???";
			}
			else
			{
				pinfo.allow = true;
				pinfo.name = vstPlug.GetBrand();
				{
					std::ostringstream s;
					s << (vstPlug.IsSynth() ? "VST2 instrument" : "VST2 effect") << " by " << vstPlug.GetVendorName();
					pinfo.desc = s.str();
				}
				{
					std::ostringstream s;
					s << vstPlug.GetVersion();
					pinfo.version = s.str();
				}

				if(vstPlug.IsSynth()) pinfo.mode = MACHMODE_GENERATOR;
				else pinfo.mode = MACHMODE_FX;

				logger.emit(vstPlug.GetBrand()); logger.emit(" - successfully instanciated");
			}
			// [bohan] vstPlug is a stack object, so its destructor is called
			// [bohan] at the end of its scope (this cope actually).
			// [bohan] The problem with destructors of any object of any class is that
			// [bohan] they are never allowed to throw any exception.
			// [bohan] So, we catch exceptions here by calling vstPlug.Free(); explicitly.
			try
			{
				vstPlug.Free();
				// <bohan> phatmatik crashes here...
				// <magnus> so does PSP Easyverb, in FreeLibrary
			}
			catch(const std::exception & e)
			{
				std::stringstream s; s
					<< std::endl
					<< "### ERRONEOUS ###" << std::endl
					<< "Exception occured while trying to free the temporary instance of the plugin." << std::endl
					<< "This plugin will not be disabled, but you might consider it unstable." << std::endl
					<< typeid(e).name() << std::endl;
				if(e.what()) s << e.what(); else s << "no message"; s << std::endl;
				logger.emit(s.str());
			}
			catch(...)
			{
				std::stringstream s; s
					<< std::endl
					<< "### ERRONEOUS ###" << std::endl
					<< "Exception occured while trying to free the temporary instance of the plugin." << std::endl
					<< "This plugin will not be disabled, but you might consider it unstable." << std::endl
					<< "Type of exception is unknown, no further information available.";
				logger.emit(s.str());
			}
		}
	}
	catch(const std::exception & e)
	{
		{
			std::stringstream s;
			s << std::endl << "################ SCANNER CRASHED ; PLEASE REPORT THIS BUG! ################" << std::endl;
			logger.emit(s.str());
		}
		{
			std::stringstream s;
			s << typeid(e).name() << std::endl;
			if(e.what()) s << e.what(); else s << "no message"; s << std::endl;
			logger.emit(s.str());
		}
	}
	catch(...)
	{
		std::stringstream s; s
			<< std::endl
			<< "################ SCANNER CRASHED ; PLEASE REPORT THIS BUG! ################" << std::endl
			<< "Type of exception is unknown, no further information available.";
		logger.emit(s.str());
	}
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
		file.Read(p.subclass);
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
		file.Write(iterator->second.subclass);
		file.WriteChunk(iterator->second.name.c_str(),iterator->second.name.length()+1);
		file.WriteChunk(iterator->second.desc.c_str(),iterator->second.desc.length()+1);
		file.WriteChunk(iterator->second.version.c_str(),iterator->second.version.length()+1);
	}
	file.Close();
	return true;
}

}
}