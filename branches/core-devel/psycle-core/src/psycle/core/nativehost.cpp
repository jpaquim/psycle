// -*- mode:c++; indent-tabs-mode:t -*-
/***************************************************************************
*   Copyright (C) 2007 Psycledelics     *
*   psycle.sf.net   *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
*   This program is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
*   You should have received a copy of the GNU General Public License     *
*   along with this program; if not, write to the                         *
*   Free Software Foundation, Inc.,                                       *
*   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
***************************************************************************/
#include <psycle/core/psycleCorePch.hpp>

#include "nativehost.hpp"
#include "pluginfinder.h"
#include "plugin.h"

namespace psy {namespace core {

static NativeHost* instance_ = 0;

NativeHost::NativeHost(MachineCallbacks*calls)
:MachineHost(calls){
}
NativeHost::~NativeHost()
{
	if ( instance_ )
		delete instance_;
}

NativeHost& NativeHost::getInstance(MachineCallbacks* callb) {
	if ( !instance__ )
		instance__ = new NativeHost(callb);
	return instance_;
}

Machine* NativeHost::CreateMachine(PluginFinder* finder, MachineKey key,Machine::id_type id) const 
{
	Machine* mac=0;
	Plugin * p;
	mac = p = new Plugin(callbacks, id);

	std::string fullPath = finder->lookupDllName(key);
	if (fullpath.empty()){ 
		return 0;
	}

	if(!p->LoadDll(plugin_path, dllName))
	{
		delete p;
		return 0;
	}
	mac->Init();
	return mac;
}


void NativeHost::DeleteMachine(Machine* mac) const {
	delete mac;
}

void NativeHost::FillFinderData(PluginFinder* finder, bool clearfirst) const
{
	if ( !finder.hasHost(Hosts::NATIVE) ) {
		//Finder stores one map for each host, so we ensure that it knows this host.
		finder.addHost(Hosts::NATIVE);
	}
	std::map<MachineKey,PluginInfo> infoMap = finder.getMap(Hosts::NATIVE);

	if (clearfirst) {
		infoMap.clear();
	}
}



bool NativeHost::LoadDll( Plugin* mac, std::string const & path, std::string const & psFileName_ )
{
// do this more generic. Since we lookup the full path from the pluginFinder, this process is not needed
// *BUT* the dllname in the MachineKey needs some preprocessing previous to be added.
// Concretely: removal of the extension.
// Removal of the "lib-xpsycle.plugin." part of the name, for linux.
// Loaders will need also to remove the extension when creating a key.
//
// MachineKey could do the std::transform to ToLower() as an automatic step.


	std::string prefix = "lib-xpsycle.plugin.";
	std::string psFileName = psFileName_;
	#if defined __unix__ || defined __APPLE__        
		std::transform(psFileName.begin(),psFileName.end(),psFileName.begin(),ToLower());
		if (psFileName.find(".so")== std::string::npos) {
			_psDllName = psFileName;
			int i = psFileName.find(".dll");
			std::string withoutSuffix = psFileName.substr(0,i);
			std::string soName = withoutSuffix + ".so";
			psFileName = prefix + soName;
			psFileName = path + psFileName; 
			unsigned int pos;
			while((pos = psFileName.find(' ')) != std::string::npos) psFileName[pos] = '_';
		} else {
			unsigned int i = psFileName.find(prefix);
			if (i!=std::string::npos) {
				int j = psFileName.find(".so");
				if (j!=0) {
					_psDllName = psFileName.substr(0,j);
					_psDllName.erase(0, prefix.length());
					_psDllName = _psDllName + ".dll";
				} else {
					_psDllName = psFileName;
					_psDllName.erase(0, prefix.length());
					_psDllName = _psDllName + ".dll";
				}
			} else _psDllName = psFileName;

			psFileName = path + psFileName; 
		}
	#else
		_psDllName = psFileName;
		psFileName = path + psFileName;
	#endif   
	return Instance(psFileName);
}

}}


