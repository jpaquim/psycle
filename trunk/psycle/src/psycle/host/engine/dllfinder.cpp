#include <packageneric/pre-compiled.private.hpp>
#include PACKAGENERIC
#include "dllfinder.hpp"
UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(psycle)
UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(host)

DllFinder::DllFinder()
{
}
DllFinder::~DllFinder()
{
}

///< Adds the search path, and initializes any needed variable/process.
void DllFinder::AddPath(std::string &path)
{
}

///< Resets the Finder to the original (clean) state.
void DllFinder::ResetFinder()
{
}

///< searches the full path for a specified dll name
bool DllFinder::LookupDllPath(std::string& name, std::string& fullpath)
{
	return false;
}

UNIVERSALIS__COMPILER__NAMESPACE__END
UNIVERSALIS__COMPILER__NAMESPACE__END