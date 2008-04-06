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
#include "machine.h"
#include "pluginfinder.h"
#include "internalhost.hpp"
//#include "nativehost11.hpp"
//#include "vsthost24.hpp"
//#include "ladspahost.hpp"
namespace psy{ namespace core {



MachineFactory::MachineFactory(MachineCallbacks* callbacks,PluginFinder* finder, Song* song)
:callbacks_(callbacks)
,song_(song)
,finder_(finder)
{
	hosts_.push_back( &InternalHost::getInstance(callbacks) );
//	hosts_.push_back( &NativeHost::getInstance(callbacks) );
//	hosts_.push_back( &VstHost::getInstance(callbacks) );
//	hosts_.push_back( &LadspaHost::getInstance(callbacks) );

}

Machine* MachineFactory::CreateMachine(MachineKey key,Machine::id_type id)
{
	for (std::vector<MachineHost*>::iterator it = hosts_.begin(); it < hosts_.end(); ++it)
	{
		MachineHost* host = *it;
		if ( host->hostCode() == key.host() ) {
			return host->CreateMachine(key,id);
			break;
		}
	}
///FIXME: Move this code to the appropiate host.
#if 0
	if ( finder.info( key ).type() == MACH_PLUGIN )
	{
		if ( finder.info( key ).mode() == MACHMODE_FX ) {
			if (id == -1) id = GetFreeFxBus();
		} else if (id == -1) id = GetFreeBus();
		return CreateMachine(finder.psycle_path(), MACH_PLUGIN, x, y, key.dllPath(), id );
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
	return 0;
}

void MachineFactory::DeleteMachine(Machine* mac)
{
	MachineKey key = mac->getMachineKey();
	for (std::vector<MachineHost*>::iterator it = hosts_.begin(); it < hosts_.end(); ++it)
	{
		MachineHost* host = *it;
		if ( host->hostCode() == key.host() ) {
			return host->DeleteMachine(mac);
			break;
		}
	}
}

}}
