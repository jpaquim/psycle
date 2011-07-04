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

#include "pluginfinder.h"

#include <cassert>

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
bool PluginFinder::hasHost(Hosts::type type) const
{
	return (type < maps_.size());
}

std::map< MachineKey, PluginInfo >::const_iterator PluginFinder::begin(Hosts::type host) const {
	assert(host < maps_.size());
	return maps_[host].begin();
}
std::map< MachineKey, PluginInfo >::const_iterator PluginFinder::end(Hosts::type host) const {
	assert(host < maps_.size());
	return maps_[host].end();
}

void PluginFinder::AddInfo(const MachineKey & key, const PluginInfo& info) {
	if (hasHost(key.host())){
		maps_[key.host()][key]= info;
	}
}

const PluginInfo & PluginFinder::info ( const MachineKey & key ) const {
	if (!hasHost(key.host())) {
		return empty_;
	}

	std::map< MachineKey, PluginInfo >::const_iterator it = maps_[key.host()].find( key );
	if ( it != maps_[key.host()].end() ) {
		return it->second;
	} else {
		return empty_;
	}
}
PluginInfo PluginFinder::info( const MachineKey & key ) {
	if (!hasHost(key.host())) {
		return empty_;
	}

	std::map< MachineKey, PluginInfo >::const_iterator it = maps_[key.host()].find( key );
	if ( it != maps_[key.host()].end() ) {
		return it->second;
	} else {
		return empty_;
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
	if ( it != maps_[key.host()].end() ) {
		return true;
	} else {
		return false;
	}
}
void PluginFinder::ClearMap(Hosts::type host) {
	if ( hasHost(host)) {
		maps_[host].clear();
	}
}

}}
