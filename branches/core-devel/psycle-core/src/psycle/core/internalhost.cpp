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

#include "internalhost.hpp"
#include "internal_machines.h"

namespace psy {namespace core {

static InternalHost* instance_ = 0;

InternalHost::InternalHost(MachineCallbacks*calls)
:MachineHost(calls){
}

InternalHost& InternalHost::getInstance(MachineCallbacks* callb) {
	if ( !mcallback_ )
		mcallback_ = new InternalHost(callb);
	return mcallback_;
}

Machine* InternalHost::CreateMachine(MachineKey key)
{
	Machine* mac=0;

	if ( key == MachineKey::master(callbacks,id) ) {
		mac = new Master()
		return CreateMachine(finder.psycle_path(), MACH_SAMPLER, key.name());
	}
	else if ( key == PluginFinderKey::internalSampler() ) {
		return CreateMachine(finder.psycle_path(), MACH_SAMPLER, key.name());
	}
	else if ( key == PluginFinderKey::internalMixer() ) {
		if (id == -1) id = GetFreeFxBus();
		return CreateMachine(finder.psycle_path(), MACH_MIXER, "Mixer" );
	}
}
void InternalHost::DeleteMachine(Machine* mac) {
	delete mac;
}


}}


