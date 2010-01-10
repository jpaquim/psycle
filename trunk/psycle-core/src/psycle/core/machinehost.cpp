// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2009 members of the psycle project http://psycle.sourceforge.net

#include <psycle/core/config.private.hpp>
#include "machinehost.hpp"

#include "pluginfinder.h"
#include "file.h"

#include <iostream>

namespace psycle { namespace core {

MachineHost::MachineHost(MachineCallbacks*calls)
: mcallback_(calls)
{}

void MachineHost::FillFinderData(PluginFinder& finder, bool clearfirst) {
	class populate_plugin_list
	{
	public:
		populate_plugin_list(std::vector<std::string> & result, std::string directory)
		{
			std::vector<std::string> intermediate;
			intermediate = File::fileList(directory, File::list_modes::dirs);
			for ( std::vector<std::string>::iterator it = intermediate.begin()
				; it < intermediate.end(); ++it )
			{
				if (*it != "." && *it != "..")
				{
					populate_plugin_list(result, directory + File::slash() + *it);
				}
			}

			intermediate = File::fileList(directory, File::list_modes::files);
			for ( std::vector<std::string>::iterator it = intermediate.begin();
				it < intermediate.end(); ++it )
			{
				result.push_back(directory + File::slash() + *it);
			}
		}
	};

	if (clearfirst) {
		finder.ClearMap(hostCode());
	}
	for (int i=0; i < getNumPluginPaths();i++) 
	{
		if (getPluginPath(i) == "") continue;

		//std::string currentPath = getPluginPath(i);
		std::vector<std::string> fileList;
		try {
			populate_plugin_list(fileList, getPluginPath(i));
			//fileList = File::fileList(currentPath, File::list_modes::files);
		} catch(std::exception & e) {
			std::cerr
				<< "psycle: host: warning: Unable to scan your " << hostName() << " plugin directory with path: " << getPluginPath(i)
				<< ".\nPlease make sure the directory exists.\nException: " << e.what();
			return;
		}
		//currentPath = currentPath + File::slash();
		std::vector<std::string>::iterator it = fileList.begin();
		for ( ; it < fileList.end(); ++it ) {
			std::string plainName = File::extractFileNameFromPath(*it);
			MachineKey thekey(hostCode(), plainName, 0);
			if ( !finder.hasKey(thekey) ) {
				FillPluginInfo(*it, plainName,finder);
			}
		}
	}
}

}}
