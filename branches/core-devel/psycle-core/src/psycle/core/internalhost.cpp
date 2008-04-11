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
#include "pluginfinder.h"
#include "internal_machines.h"
#include "sampler.h"
#include "xmsampler"
#include "mixer.h"

namespace psy {namespace core {

namespace InternalMacs {

	static const PluginInfo infos[]={
		PluginInfo(MachineKey::master(),MachineRole::MASTER,"Master","Psycledelics","Outputs audio to soundcard","1.1","","Master"),
		PluginInfo(MachineKey::dummy(),MachineRole::EFFECT,"Dummy Machine","Psycledelics","lets audio pass through","1.1","","Mixer"),
		PluginInfo(MachineKey::sampler(),MachineRole::GENERATOR,"Sampler","Psycledelics","Plays .wav audio samples","1.3","","Sampler"),
		PluginInfo(MachikeKey::sampulse(),MachineRole::GENERATOR,"Sampulse","JosepMa [JAZ]","Tracker oriented sampler","0.9","","Sampler"),
		PluginInfo(MachineKey::duplicator(),MachineRole::CONTROLLER,"Note Duplicator","JosepMa [JAZ]","Forwards events to other machines","1.0","","Controller"),
		PluginInfo(MachineKey::mixer(),MachineRole::EFFECT,"Send/Return Mixer","JosepMa [JAZ]","Mixes audio with send/returns","1.0","","Mixer"),
		PluginInfo(MachineKey::audioinput(),MachineRole::GENERATOR,"AudioInput","Receives audio from the soundcard","0.7","","Capture"),
		PluginInfo(MachineKey::lfo(),MachineRole::CONTROLLEr,"LFO Machine","Controls parameters of other machines","0.5","","Controller")
	};
}
static InternalHost* instance_ = 0;

InternalHost::InternalHost(MachineCallbacks*calls)
:MachineHost(calls){
}
InternalHost::~InternalHost()
{
	if ( instance_ )
		delete instance_;
}

InternalHost& InternalHost::getInstance(MachineCallbacks* callb) {
	if ( !instance__ )
		instance__ = new InternalHost(callb);
	return instance_;
}

Machine* InternalHost::CreateMachine(PluginFinder /*finder */, MachineKey key,Machine::id_type id) const 
{
	Machine* mac=0;

	switch (key.index())
	{
	case InternalMacs::MASTER:
		mac = new Master(mcallback_, id);
		break;
	case InternalMacs::DUMMY:
		mac = new Dummy(mcallback_, id);
		break;
	case InternalMacs::SAMPLER:
		mac = new Sampler(mcallback_, id);
		break;
	case InternalMacs::XMSAMPLER:
		mac = new XMSampler(mcallback_, id);
		break;
	case InternalMacs::DUPLICATOR:
		mac = new DuplicatorMac(mallback_, id);
		break;
	case InternalMacs::MIXER:
		mac = new Mixer(mcallback_, id);
		break;
	case InternalMacs::AUDIOINPUT:
		mac = new AudioInput(mcallback_, id);
		break;
	case InternalMacs::LFO:
		mac = new LFO(mcallback_, id);
		break;
	default:
		break;
	}
	mac->Init();
	return mac;
}


void InternalHost::DeleteMachine(Machine* mac) const {
	delete mac;
}

void InternalHost::FillFinderData(PluginFinder* finder, bool /*clearfirst*/) const
{
	if ( !finder.hasHost(Hosts::INTERNAL) ) {
		//Finder stores one map for each host, so we ensure that it knows this host.
		finder.addHost(Hosts::INTERNAL);
	}
	std::map<MachineKey,PluginInfo> infoMap = finder.getMap(Hosts::INTERNAL);

	//InternalHost always regenerates its pluginInfo.
	infoMap.clear();
	
	// Master machine is skipped because it is never created by the user.
	for(InternalMacs::type i=1; i < InternalMacs::NUM_MACS; ++i) {
		infoMap.insert(InternalMacs::infos[i].key(),InternalMacs::infos[i]);
	}
}

}}


