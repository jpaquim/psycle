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

namespace InternalMacs {
	static const char names[]={
		"Master",
		"Dummy Machine",
		"Sampler",

		"Sampulse",
		"Note Duplicator",
		"Send/Return Mixer",

		"AudioInput",
		"LFO Machine"
	};
	static const char author[]={
		"Psycledelics",
		"Psycledelics",
		"Psycledelics",

		"JosepMa [JAZ]",
		"JosepMa [JAZ]",
		"JosepMa [JAZ]",

		"JosepMa [JAZ]",
		"dw"
	};
	static const char description[]={
		"Outputs Audio to soundcard",
		"lets audio pass through",
		"Plays .wav audio samples",
		
		"Tracker oriented sampler",
		"forwards events to other machines",
		"Mixes audio allowing send/returns",
		
		"Receives audio from the soundcard",
		"Controls another machine's parameter",
	};
	static const char version[]={
		"1.1",
		"1.1",
		"1.3",

		"0.9",
		"1.0",
		"1.0",

		"0.7",
		"0.5"
	};
	static const MachineMode::type mode[] {
		MachineMode::MASTER,
		MachineMode::EFFECT,
		MachineMode::GENERATOR,
		
		MachineMode::GENERATOR,
		MachineMode::CONTROLLER,
		MachineMode::EFFECT,

		MachineMode::GENERATOR,
		MachineMode::CONTROLLER
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

Machine* InternalHost::CreateMachine(MachineKey key,Machine::id_type id) const
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

void InternalHost::getMachineInformation(std::vector<PluginInfo>& infoList) const
{
	//InternalHost always regenerates his pluginInfo.
	infoList.clear();
	
	for(InternalMacs::type i=0; i < InternalMacs::NUM_MACS; ++i) {
		PluginInfo info;
		info.setKey(MachineKey(Hosts::INTERNAL,"",i));
		info.setName(InternalMacs::names[i]);
		info.setAuthor(InternalMacs::author[i]);
		info.setDescription(InternalMacs::description[i]);
		info.setVersion(InternalMacs::version[i]);
		info.setMode(InternalMacs::mode[i]);
		info.setLibName("");
		infoList.push_back(info);
	}
}

}}


