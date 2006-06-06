#include <packageneric/pre-compiled.private.hpp>
#include PACKAGENERIC
#include "cacheddllfinder.hpp"

UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(psycle)
UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(host)

MappedDllFinder::MappedDllFinder()
{
}
MappedDllFinder::~MappedDllFinder()
{
}

///< Adds the search path, and initializes any needed variable/process.
void MappedDllFinder::AddPath(std::string &path)
{
}
///< Resets the Finder to the original state.
void MappedDllFinder::ResetFinder()
{
}
///< searches in the map the full path for a specified dll name
bool MappedDllFinder::LookupDllPath(std::string& name, std::string& fullpath)
{
	return false;
}

///< Adds the dll name -> full path mapping to the map.
void MappedDllFinder::learnDllName(const std::string & fullpath)
{
}




CachedDllFinder::CachedDllFinder()
{
}
CachedDllFinder::~CachedDllFinder()
{
}

///< Adds the search path, and initializes any needed variable/process.
void CachedDllFinder::AddPath(std::string &path)
{
}
///< Resets the Finder to the original state.
void CachedDllFinder::ResetFinder()
{
}
///< searches in the map the full path for a specified dll name
bool CachedDllFinder::LookupDllPath(std::string& name, std::string& fullpath)
{
	return false;
}

///< Adds the new Plugin data to the map.
void CachedDllFinder::learnPlugin(PluginInfo &plugininfo)
{
}
void CachedDllFinder::CollectPlugins()
{
}
///< Checks if the plugin that is going to be loaded is allowed to be loaded.
bool CachedDllFinder::TestFilename(const std::string & name)
{
	return false;
}

bool CachedDllFinder::LoadCacheFile(int & currentPlugsCount, int & currentBadPlugsCount)
{
	return false;
}
bool CachedDllFinder::SaveCacheFile()
{
	return false;
}

UNIVERSALIS__COMPILER__NAMESPACE__END
UNIVERSALIS__COMPILER__NAMESPACE__END