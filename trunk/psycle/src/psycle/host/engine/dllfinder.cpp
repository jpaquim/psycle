#include <packageneric/pre-compiled.private.hpp>
#include PACKAGENERIC
#include "dllfinder.hpp"
#include "afxwin.h"	// For CFileFind. If an alternative method is found, this can be removed.

UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(psycle)
UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(host)

DllFinder::DllFinder()
{
}
DllFinder::~DllFinder()
{
	base_paths.clear();
}

///< Adds the search path, and initializes any needed variable/process.
void DllFinder::AddPath(std::string &path)
{
	base_paths.push_back(path);
}

///< Resets the Finder to the original (clean) state.
void DllFinder::ResetFinder()
{
	base_paths.clear();
}

///< searches the full path for a specified dll name
bool DllFinder::LookupDllPath(std::string& name)
{
	std::vector<std::string>::iterator iterator = base_paths.begin();
	for (;iterator != base_paths.end();iterator++)
	{
		if (SearchFileInDir(name,*iterator))
		{
			return true;
		}
	}
	return false;
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


UNIVERSALIS__COMPILER__NAMESPACE__END
UNIVERSALIS__COMPILER__NAMESPACE__END