#pragma once
#include <psycle/engine/machine.hpp>
#include <string>

namespace psycle
{
	namespace host
	{

class DllFinder
{
public:
	DllFinder();
	virtual ~DllFinder();

	///< Adds the search path, and initializes any needed variable/process.
	virtual void AddPath(const std::string &path,Machine::class_type subclass);

	///< Resets the Finder to the original (clean) state.
	virtual void ResetFinder();

	///< fills in the path for the specified name so that name becomes a fullpath.
	///< mtype
	virtual bool LookupDllPath(std::string& name,Machine::class_type subclass);

	///< returns the file extracted from the fullpath
	std::string FileFromFullpath(const std::string& fullpath);
protected:
	bool SearchFileInDir(std::string& name,std::string& path);

	std::map<Machine::class_type,std::string> base_paths;
};

	}
}