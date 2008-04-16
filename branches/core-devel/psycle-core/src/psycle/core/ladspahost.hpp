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

#ifndef LADSPAHOST_HPP
#define LADSPAHOST_HPP

#include "machinehost.hpp"
#include "ladspa.h"
#include <string>
#include <map>

namespace psy{ namespace core{

class LadspaHost : public MachineHost
{
protected:
	LadspaHost(MachineCallbacks*);
public:
	~LadspaHost();
	static LadspaHost& getInstance(MachineCallbacks*);

	virtual Machine* CreateMachine(PluginFinder*, MachineKey, Machine::id_type);

	virtual const Hosts::type hostCode() const { return Hosts::LADSPA; }
	virtual const std::string hostName() const { return "Ladspa"; }
	virtual std::string const & getPluginPath(int) const { return plugin_path_; }
	virtual int getNumPluginPaths() const { return 1; }
	virtual void setPluginPath(std::string path);

protected:
	virtual void FillPluginInfo(const std::string&, const std::string&, std::map<MachineKey,PluginInfo>&);
	void* LoadDll( const std::string &  ) const;
	LADSPA_Descriptor_Function LoadDescriptorFunction(void*) const;
	LADSPA_Handle Instantiate(const LADSPA_Descriptor*) const;
	void UnloadDll( void* ) const;
	void *dlopenLADSPA(const char * pcFilename, int iFlag) const;
	std::string  plugin_path_;
};

}}
#endif // LADSPAHOST_HPP
