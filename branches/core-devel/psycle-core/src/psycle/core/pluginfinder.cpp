// -*- mode:c++; indent-tabs-mode:t -*-
/**************************************************************************
*   Copyright 2007 Psycledelics http://psycle.sourceforge.net             *
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
#include "pluginfinder.h"
#include "file.h"
#include "ladspamachine.h"
#include "plugin.h"
#include <iostream> // only for debug output
#include <sstream>

namespace psy { namespace core {

PluginFinder::PluginFinder()
{
}
PluginFinder::~PluginFinder()
{
}
PluginFinder& PluginFinder::getInstance() 
{
	static PluginFinder finder;
	return finder;
}

void PluginFinder::addHost(Hosts::type type)
{
	if (type >= maps_.size()) {
		maps_.resize(type+1);
	}
}
bool PluginFinder::hasHost(Hosts::type type)
{
	return (type < maps_.size());
}

std::map< MachineKey, PluginInfo >::const_iterator PluginFinder::begin(Hosts::type) const {
	return map_.begin();
}
std::map< MachineKey, PluginInfo >::const_iterator PluginFinder::end(Hosts::type) const {
	return map_.end();
}
std::map< MachineKey, PluginInfo >& PluginFinder:getMap(Hosts::type type) {
	assert(type < maps_size());
	return maps_[type];
}

PluginInfo PluginFinder::info( const MachineKey & key ) const {
	if (!hasHost(key.host())) {
		return PluginInfo();
	}
	
	std::map< MachineKey, PluginInfo >::const_iterator it = maps_[key.host()].find( key );
	if ( it != map_.end() ) {
		return it->second;
	} else {
		return PluginInfo();
	}
}
std::string PluginFinder::lookupDllName( const MachineKey & key ) const {
	return info(key).libName();
}
bool PluginFinder::hasKey( const MachineKey& key ) const {
	if (!hasHost(key.host())) {
		return false;
	}
	std::map< MachineKey, PluginInfo >::const_iterator it = maps_[key.host()].find( key );
	if ( it != map_.end() ) {
		return true;
	} else {
		return false;
	}
	
}}
