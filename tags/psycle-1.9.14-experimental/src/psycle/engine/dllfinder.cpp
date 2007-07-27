#include <packageneric/pre-compiled.private.hpp>
#include <packageneric/module.private.hpp>
#include <psycle/engine/dllfinder.hpp>
#include <algorithm> // std::transform
#include <cctype>	   // std::tolower
#include "afxwin.h"	// For CFileFind. If an alternative method is found, this can be removed.

namespace psycle
{
	namespace host
	{

DllFinder::DllFinder()
{
}
DllFinder::~DllFinder()
{
	base_paths.clear();
}

///< Adds the search path, and initializes any needed variable/process.
void DllFinder::AddPath(const std::string &path,Machine::class_type subclass)
{
	//This implementation only allows one path of each subclass
	base_paths[subclass] = path;
}

///< Resets the Finder to the original (clean) state.
void DllFinder::ResetFinder()
{
	base_paths.clear();
}

///< searches the full path for a specified dll name
bool DllFinder::LookupDllPath(std::string& name,Machine::class_type subclass)
{
	std::transform(name.begin(),name.end(),name.begin(),std::tolower);
	std::map<Machine::class_type,std::string>::iterator iterator = base_paths.find(subclass);

	if(iterator != base_paths.end())
	{
		if (SearchFileInDir(name,iterator->second))
		{
			return true;
		}
	}
	return false;
}

std::string DllFinder::FileFromFullpath(const std::string& fullpath)
{
	std::string str=fullpath;
	// strip off path
	std::string::size_type pos=str.rfind('\\');
	if(pos != std::string::npos) str=str.substr(pos+1);
	// transform string to lower case
	std::transform(str.begin(),str.end(),str.begin(),std::tolower);
	return str;
}

bool DllFinder::SearchFileInDir(std::string& name, std::string& path)
{
	CFileFind finder;
	//Browse through the directories.
	int loop = finder.FindFile(::CString((path + "\\*").c_str()));
	while(loop)
	{						
		loop = finder.FindNextFile();
		if(finder.IsDirectory() && !finder.IsDots())
		{
			std::string filepath = finder.GetFilePath();
			if (SearchFileInDir(name,filepath))
			{
				return true;
			}
		}
	}
	finder.Close();
	//not found in subdirectories, lets see if it's here
	if (finder.FindFile(::CString((path + "\\" + name).c_str())))
	{
		finder.Close();
		name = (path + "\\" + name);
		return true;
	}
	finder.Close();
	return false;
}


	}
}