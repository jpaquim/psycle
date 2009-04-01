// This program is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
// You should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
// copyright 2007-2009 members of the psycle project http://psycle.sourceforge.net

#ifndef PSYCLE__CORE__INTERNAL_HOST__INCLUDED
#define PSYCLE__CORE__INTERNAL_HOST__INCLUDED
#pragma once

#include "machinehost.hpp"

namespace psy { namespace core {

// type InternalMacs::type
// Allows to differentiate between internal machines.
// They are not intended to be used from outside. Use MachineKey for that.
namespace InternalMacs {
	typedef enum type_t {
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

		virtual Machine* CreateMachine(PluginFinder&, MachineKey, Machine::id_type);
		virtual void FillFinderData(PluginFinder&, bool clearfirst=false);

		virtual const Hosts::type hostCode() const { return Hosts::INTERNAL; }
		virtual const std::string hostName() const { return "Internal"; }
	protected:
		virtual void FillPluginInfo(const std::string&, const std::string&, PluginFinder& ) {}
};

}}
#endif
