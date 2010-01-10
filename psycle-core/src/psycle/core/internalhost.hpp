// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2009 members of the psycle project http://psycle.sourceforge.net

#ifndef PSYCLE__CORE__INTERNAL_HOST__INCLUDED
#define PSYCLE__CORE__INTERNAL_HOST__INCLUDED
#pragma once

#include "machinehost.hpp"

namespace psycle { namespace core {

// type InternalMacs::type
// Allows to differentiate between internal machines.
// They are not intended to be used from outside. Use MachineKey for that.
namespace InternalMacs {
	enum type_t {
		MASTER = 0,
		DUMMY,
		SAMPLER,
		XMSAMPLER,
		DUPLICATOR,
		MIXER,
		AUDIOINPUT,
		LFO,
		//Keep at Last position.
		NUM_MACS
	};
}

class PSYCLE__CORE__DECL InternalHost : public MachineHost {
	protected:
		InternalHost(MachineCallbacks*);
	public:
		virtual ~InternalHost();
		static InternalHost& getInstance(MachineCallbacks*);

		virtual Machine* CreateMachine(PluginFinder&, const MachineKey &, Machine::id_type);
		virtual void FillFinderData(PluginFinder&, bool clearfirst=false);

		virtual const Hosts::type hostCode() const { return Hosts::INTERNAL; }
		virtual const std::string hostName() const { return "Internal"; }
	protected:
		virtual void FillPluginInfo(const std::string&, const std::string&, PluginFinder& ) {}
};

}}
#endif
