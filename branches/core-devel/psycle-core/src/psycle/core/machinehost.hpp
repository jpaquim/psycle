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

#ifndef MACHINEHOST_HPP
#define MACHINEHOST_HPP

#include "machinekey.hpp"
#include "machine.h"

namespace psy{namespace core{

class MachineCallbacks;
class Machine;
class PluginFinder;

class MachineHost
{
protected:
	MachineHost(){}
	MachineHost(MachineCallbacks*);
public:
	virtual const Hosts::type hostCode() const = 0;
	virtual const std::string hostName() const = 0;

	virtual void FillFinderData(PluginFinder*, bool clearfirst=false) const = 0;

	virtual Machine* CreateMachine(PluginFinder*,MachineKey,Machine::id_type) const = 0;
	virtual void DeleteMachine(Machine*) const = 0;
protected:
	MachineCallbacks* mcallback_;
};

}}
#endif // MACHINEHOST_HPP
