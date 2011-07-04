/**************************************************************************
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

#ifndef MACHINEFACTORY_H
#define MACHINEFACTORY_H

#include "machinekey.hpp"
#include "machine.h"

namespace psy { namespace core { 

class PluginFinder;
class MachineHost;

// MachineFactory
// Generates Machines and maintains the finder information.
// Note: An usual factory would have functions for each type of machine.
// Here it's been choosen to use MachineKeys instead.
class MachineFactory
{
private:
	MachineFactory();
public:
	// To create a MachineFactory do a getInstance().Initialize() with either
	// of the Initialize Functions.
	// If you use the one wihout PluginFinder, one will be created automatically.
	void Initialize(MachineCallbacks* callbacks);
	void Initialize(MachineCallbacks* callbacks, PluginFinder* finder);
	static MachineFactory& getInstance();

	Machine* CreateMachine(MachineKey key,Machine::id_type id=-1);
	Machine* CloneMachine(Machine& mac);
	
	const std::vector<MachineHost*> getHosts() const { return hosts_; }
	MachineCallbacks* getCallbacks() const { return callbacks_; }

	std::string const & getPsyclePath() const;
	void setPsyclePath(std::string path, bool cleardata=false);

	std::string const & getLadspaPath() const;
	void setLadspaPath(std::string path,bool cleardata=false);

	void RegenerateFinderData();

protected:
	void FillHosts();

	MachineCallbacks* callbacks_;
	PluginFinder* finder_;
	std::vector<MachineHost*> hosts_;
};

}}
#endif //MACHINEFACTORY_H
