#pragma once

UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(psycle)
	UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(host)

class DllFinder
{
public:
	DllFinder();
	virtual ~DllFinder();

	///< Adds the search path, and initializes any needed variable/process.
	void AddPath(std::string &path);

	///< Resets the Finder to the original (clean) state.
	void ResetFinder();

	///< searches the full path for a specified dll name
	bool LookupDllPath(std::string& name, std::string& fullpath);

protected:
	std::vector<std::string> base_paths;
};

UNIVERSALIS__COMPILER__NAMESPACE__END
UNIVERSALIS__COMPILER__NAMESPACE__END