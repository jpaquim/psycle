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

#ifndef INTERNALHOST_HPP
#define INTERNALHOST_HPP

#include "machinehost.hpp"

namespace psy{ namespace core{


class InternalHost : public MachineHost
{
protected:
	InternalHost(MachineCallbacks*);
public:
	~InternalHost();
	static InternalHost& getInstance(MachineCallbacks*);

	virtual Machine* CreateMachine(PluginFinder&, MachineKey, Machine::id_type);
	virtual void FillFinderData(PluginFinder&, bool clearfirst=false);

	virtual const Hosts::type hostCode() const { return Hosts::INTERNAL; }
	virtual const std::string hostName() const { return "Internal"; }
protected:
	virtual void FillPluginInfo(const std::string&, const std::string&, PluginFinder& ) {}
};

}}
#endif // INTERNALHOST_HPP