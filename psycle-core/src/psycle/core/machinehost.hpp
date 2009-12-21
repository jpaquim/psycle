// This program is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
// You should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
// copyright 2007-2009 members of the psycle project http://psycle.sourceforge.net

#ifndef PSYCLE__CORE__MACHINE_HOST__INCLUDED
#define PSYCLE__CORE__MACHINE_HOST__INCLUDED
#pragma once

#include "machinekey.hpp"
#include "plugininfo.h"
#include "machine.h"

#include <string>

namespace psycle { namespace core {

class MachineCallbacks;
class Machine;
class PluginFinder;

class PSYCLE__CORE__DECL MachineHost {
protected:
	MachineHost(MachineCallbacks*);
public:
	virtual ~MachineHost() {}
	virtual Machine* CreateMachine(PluginFinder&, const MachineKey &, Machine::id_type) = 0;
	virtual void FillFinderData(PluginFinder&, bool clearfirst=false);

	virtual const Hosts::type hostCode() const = 0;
	virtual const std::string hostName() const = 0;

	virtual std::string const & getPluginPath(int) const { static std::string ret = ""; return ret; };
	virtual int getNumPluginPaths() const { return 0; }
	virtual void setPluginPath(std::string ) {};
protected:
	virtual void FillPluginInfo(const std::string&, const std::string& , PluginFinder& ) = 0;

	MachineCallbacks* mcallback_;
};

}}
#endif