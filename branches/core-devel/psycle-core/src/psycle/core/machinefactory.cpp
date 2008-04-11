// -*- mode:c++; indent-tabs-mode:t -*-
// /******************************************************************************
// *  copyright 2007 members of the psycle project http://psycle.sourceforge.net *
// *                                                                             *
// *  This program is free software; you can redistribute it and/or modify       *
// *  it under the terms of the GNU General Public License as published by       *
// *  the Free Software Foundation; either version 2 of the License, or          *
// *  (at your option) any later version.                                        *
// *                                                                             *
// *  This program is distributed in the hope that it will be useful,            *
// *  but WITHOUT ANY WARRANTY; without even the implied warranty of             *
// *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the              *
// *  GNU General Public License for more details.                               *
// *                                                                             *
// *  You should have received a copy of the GNU General Public License          *
// *  along with this program; if not, write to the                              *
// *  Free Software Foundation, Inc.,                                            *
// *  59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.                  *
// ******************************************************************************/
//
#include <psycle/core/psycleCorePch.hpp>

#include "machinefactory.h"
#include "pluginfinder.h"
#include "internalhost.hpp"
#include "nativehost.hpp"
//#include "vsthost24.hpp"
//#include "ladspahost.hpp"
namespace psy{ namespace core {

MachineFactory::MachineFactory(MachineCallbacks* callbacks,PluginFinder* finder)
:callbacks_(callbacks)
,finder_(finder)
{
	//Please, keep the same order than with the Hosts::type enum. (machinekey.hpp)
	hosts_.push_back( &InternalHost::getInstance(callbacks) );
	InternalHost::getInstance(callbacks).FillFinderData(finder_);

	hosts_.push_back( &NativeHost::getInstance(callbacks) );
	//Skipped until path is set.
	//NativeHost::getInstance(callbacks).FillFinderData(finder_);

	//hosts_.push_back( &VstHost::getInstance(callbacks) );
	//VstHost::getInstance(callbacks).FillFinderData(finder_);

	//hosts_.push_back( &LadspaHost::getInstance(callbacks) );
	//LadspaHost::getInstance(callbacks).FillFinderData(finder_);

}

Machine* MachineFactory::CreateMachine(MachineKey key,Machine::id_type id)
{
	assert(key.host() < Hosts::NUM_HOSTS);
	return hosts_[key.host()].CreateMachine(finder_,key,id);
#if 0
	for (int i=0; i< hosts_.size(); ++i)
	{
		if ( hosts_[i]->hostCode() == key.host() ) {
			return hosts_[i]->CreateMachine(key,id);
			break;
		}
	}
	return 0;
#endif	
///FIXME: Move this code to the appropiate host.
#if 0
	if ( finder.info( key ).type() == MACH_PLUGIN )
	{

	}
	else if ( finder.info( key ).type() == MACH_LADSPA )
	{
		if (id == -1 ) id = GetFreeFxBus();
		LADSPAMachine* plugin = new LADSPAMachine( machinecallbacks, id, this );
		if(plugin->loadDll( key.dllPath(), key.index())) {
			plugin->SetPosX( x );
			plugin->SetPosY( y );
			plugin->Init();
			if( machine_[fb] ) DestroyMachine( fb );
				machine_[ fb ] = plugin;
		} else {
			delete plugin;
		}
	}
#endif	
}

void MachineFactory::DeleteMachine(Machine* mac)
{
	MachineKey key = mac->getMachineKey();
	assert(key.host() < Hosts::NUM_HOSTS);
	hosts_[key.host()].DeleteMachine(mac);
#if 0	
	for (int i=0; i < hosts_.size(); ++i )
	{
		if ( hosts_[i]->hostCode() == key.host() ) {
			return hosts_[i].DeleteMachine(mac);
			break;
		}
	}
#endif
}

std::string const & getPsyclePath() const { return hosts_[Hosts::NATIVE].getPsyclePath(); }
void setPsyclePath(std::string path)
{
	hosts_[Hosts::NATIVE].setPsyclePath(path);
	hosts_[Hosts::NATIVE].FillFinderData(finder_,true);
}


void MachineFactory::RegenerateFinderData() 
{
	for (int i=0; i < hosts_.size(); ++i )
	{
		hosts_[i].FillFinderData(finder_,true);
	}
}

}}
