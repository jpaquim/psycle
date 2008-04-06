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

#include "pluginFinderKey.hpp"

namespace psy
{
	namespace core
	{

		PluginFinderKey::PluginFinderKey( ) : index_(0) {

		}

		PluginFinderKey::PluginFinderKey( const std::string & name, const std::string & dllPath, int index ) :
			name_( name ),
			dllPath_( dllPath ),
			index_( index )
		{
		}

		PluginFinderKey::~PluginFinderKey() {
		}

		PluginFinderKey PluginFinderKey::internalSampler() {
			return PluginFinderKey("Psycle Internal Sampler", "none", 0 );
		}
		PluginFinderKey PluginFinderKey::internalMixer() { 
			return PluginFinderKey("Psycle Internal Mixer", "none", 0 );
		}

		bool PluginFinderKey::operator<(const PluginFinderKey & key) const {
			if ( dllPath() != key.dllPath() )
				return dllPath() < key.dllPath();
			if ( name() != key.name() ) 
				return name() < key.name();
			return index() < key.index();
		}

		bool PluginFinderKey::operator ==( const PluginFinderKey & rhs ) const {
			return dllPath() == rhs.dllPath() && name() == rhs.name() && index() == rhs.index();
		}

		const std::string & PluginFinderKey::name() const {
			return name_;
		}
		
		const std::string & PluginFinderKey::dllPath() const {
			return dllPath_;
		}

		int PluginFinderKey::index() const {
			return index_;
		}

	}
}
