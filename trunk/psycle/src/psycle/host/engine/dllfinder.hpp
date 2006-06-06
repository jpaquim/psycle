#pragma once

UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(psycle)
	UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(host)

class DllFinder
{
public:
	DllFinder();
	virtual ~DllFinder();

	///< Adds the search path, and initializes any needed variable/process.
	virtual void AddPath(std::string &path);

	///< Resets the Finder to the original (clean) state.
	virtual void ResetFinder();

	///< fills in the path for the specified name so that name becomes a fullpath.
	virtual bool LookupDllPath(std::string& name);

protected:
	bool SearchFileInDir(std::string& name,std::string& path);

	std::vector<std::string> base_paths;
};

UNIVERSALIS__COMPILER__NAMESPACE__END
UNIVERSALIS__COMPILER__NAMESPACE__END