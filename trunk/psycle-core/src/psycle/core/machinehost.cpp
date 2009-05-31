// This program is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
// You should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
// copyright 2007-2009 members of the psycle project http://psycle.sourceforge.net

#include <psycle/core/config.private.hpp>
#include "machinehost.hpp"

#include "pluginfinder.h"
#include "file.h"

#include <iostream>

namespace psy { namespace core {

MachineHost::MachineHost(MachineCallbacks*calls)
: mcallback_(calls)
{}

void MachineHost::FillFinderData(PluginFinder& finder, bool clearfirst) {
	if (clearfirst) {
		finder.ClearMap(hostCode());
	}
	for (int i=0; i < getNumPluginPaths();i++) 
	{
		std::string currentPath = getPluginPath(i);
		std::vector<std::string> fileList;
		try {
			fileList = File::fileList(currentPath, File::list_modes::files);
		} catch(std::exception & e) {
			std::cerr
				<< "psycle: host: warning: Unable to scan your " << hostName() << " plugin directory with path: " << currentPath
				<< ".\nPlease make sure the directory exists.\nException: " << e.what();
			return;
		}
		currentPath = currentPath + File::slash();
		std::vector<std::string>::iterator it = fileList.begin();
		for ( ; it < fileList.end(); ++it ) {
			MachineKey thekey(hostCode(),*it,0);
			if ( !finder.hasKey(thekey) ) {
				std::string fullName = currentPath + *it;
				FillPluginInfo(fullName,*it,finder);
			}
		}
	}
}

}}
