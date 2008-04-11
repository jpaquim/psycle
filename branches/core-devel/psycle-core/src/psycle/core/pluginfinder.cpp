// -*- mode:c++; indent-tabs-mode:t -*-
/**************************************************************************
*   Copyright 2007 Psycledelics http://psycle.sourceforge.net             *
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
#include "pluginfinder.h"
#include "file.h"
#include "ladspamachine.h"
#include "plugin.h"
#include <iostream> // only for debug output
#include <sstream>

namespace psy { namespace core {

PluginFinder::PluginFinder()
{
}
PluginFinder::~PluginFinder()
{
}
PluginFinder& PluginFinder::getInstance() 
{
	static PluginFinder finder;
	return finder;
}

void PluginFinder::addHost(Hosts::type type)
{
	if (type >= maps_.size()) {
		maps_.resize(type+1);
	}
}
bool PluginFinder::hasHost(Hosts::type type)
{
	return (type < maps_.size());
}

std::map< MachineKey, PluginInfo >::const_iterator PluginFinder::begin(Hosts::type) const {
	return map_.begin();
}
std::map< MachineKey, PluginInfo >::const_iterator PluginFinder::end(Hosts::type) const {
	return map_.end();
}
std::map< MachineKey, PluginInfo >& PluginInfo::getMap(Hosts::type type) {
	assert(type < maps_size());
	return maps_[type];
}

PluginInfo PluginFinder::info( const MachineKey & key ) const {
	if (!hasHost(key.host())) {
		return PluginInfo();
	}
	
	std::map< MachineKey, PluginInfo >::const_iterator it = maps_[key.host()].find( key );
	if ( it != map_.end() ) {
		return it->second;
	} else {
		return PluginInfo();
	}
}
std::string lookupDllName( const MachineKey & key ) const {
	return info(key).libName();
}




void PluginFinder::scanLadspa() {
	std::string ladspa_path = ladspa_path_;
	//FIXME: this just uses the first path in getenv. Do this for each path.
	// The best way would be pre-process in the constructor, and store a vector of strings.
	#if defined __unix__ || defined __APPLE__
		std::string::size_type dotpos = ladspa_path.find(':',0);
		if ( dotpos != ladspa_path.npos ) ladspa_path = ladspa_path.substr( 0, dotpos );
	#else
	#endif

	std::vector<std::string> fileList;
	fileList = File::fileList(ladspa_path, File::list_modes::files);

	std::vector<std::string>::iterator it = fileList.begin();
	for ( ; it < fileList.end(); ++it ) {
		LoadLadspaInfo(*it);
	}
}


//FIXME:Probably is needed to pass the path too, when we support more than one path.
void PluginFinder::LoadLadspaInfo(std::string fileName)
{
	std::string ladspa_path = ladspa_path_;
	const LADSPA_Descriptor * psDescriptor;
	LADSPA_Descriptor_Function pfDescriptorFunction;
	unsigned long lPluginIndex;

		#if defined __unix__ || defined __APPLE__
			// problem of so.0.0.x .. .so all three times todo
		#else
			if ( fileName.find( ".dll" ) == std::string::npos ) return;
		#endif

		class DummyCallbacks : public MachineCallbacks {
			private:
				PlayerTimeInfo ti;
			public:
				PlayerTimeInfo& timeInfo()  { return ti; }
				bool autoStopMachines() const { return false; }
		} dummycallbacks;

		LADSPAMachine plugin(&dummycallbacks, 0, 0 );
		pfDescriptorFunction = plugin.loadDescriptorFunction( (ladspa_path + File::slash()) + fileName );

		if (pfDescriptorFunction) {
			for (lPluginIndex = 0;; lPluginIndex++) {
				psDescriptor = pfDescriptorFunction(lPluginIndex);
				if (psDescriptor == NULL) {
					break;
				}
				PluginInfo info;
				info.setType( MACH_LADSPA );
				info.setName( psDescriptor->Name );
				info.setLibName( fileName );
				MachineKey key(Hosts::LADSPA, fileName, lPluginIndex );
				map_[key] = info;
			}
		}
	}


}}
