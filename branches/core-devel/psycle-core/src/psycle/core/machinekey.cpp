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
#include <psycle/core/psycleCorePch.hpp>

#include "machinekey.hpp"

namespace psy
{
	namespace core
	{
		namespace Hosts 
		{
                       static const char *names[]={"Internal","Psycle","vST","LADSPA"};
		}
		namespace InternalMacs
		{
			typedef enum 
			{
				MASTER = 0,
				DUMMY,
				SAMPLER,
				XMSAMPLER,
				DUPLICATOR,
				MIXER,
				AUDIOINPUT,
				LFO
			} type;
		}

		MachineKey::MachineKey( ) : index_(0) {
		}

		MachineKey::MachineKey(const Hosts::type host, const std::string & dllName, int index ) :
			dllName_( dllName ),
			host_( host ),
			index_( index )
		{}

		MachineKey::~MachineKey() {
		}

		const MachineKey MachineKey::master() {
			return MachineKey(Hosts::INTERNAL,"",InternalMacs::MASTER);
		}
		const MachineKey MachineKey::dummy() {
			return MachineKey(Hosts::INTERNAL,"",InternalMacs::DUMMY);
		}
		const MachineKey MachineKey::sampler() {
			return MachineKey(Hosts::INTERNAL,"",InternalMacs::SAMPLER );
		}
		const MachineKey MachineKey::sampulse() {
			return MachineKey(Hosts::INTERNAL,"",InternalMacs::XMSAMPLER );
		}
		const MachineKey MachineKey::duplicator() {
			return MachineKey(Hosts::INTERNAL,"",InternalMacs::DUPLICATOR);
		}
		const MachineKey MachineKey::mixer() {
			return MachineKey(Hosts::INTERNAL,"",InternalMacs::MIXER );
		}
		const MachineKey MachineKey::audioInput() {
			return MachineKey(Hosts::INTERNAL,"",InternalMacs::AUDIOINPUT );
		}
		const MachineKey MachineKey::LFO() {
			return MachineKey(Hosts::INTERNAL,"",InternalMacs::LFO );
		}

		static const std::string HostName(Hosts::type type) {
			if ( type < Hosts::NUM_HOSTS ) return Hosts::names[type];
		}

		bool MachineKey::operator<(const MachineKey & key) const {
			if ( host() != key.host() ) 
				return host() < key.host();
			else if ( dllName() != key.dllName() )
				return dllName() < key.dllName();
			return index() < key.index();
		}
		bool MachineKey::operator ==( const MachineKey & rhs ) const {
			return host() == rhs.host() && dllName() == rhs.dllName() && index() == rhs.index();
		}

		const std::string & MachineKey::dllName() const {
			return dllName_;
		}
		const Hosts::type MachineKey::host() const { 
			return host_;
		}
		int MachineKey::index() const {
			return index_;
		}
	}
}
