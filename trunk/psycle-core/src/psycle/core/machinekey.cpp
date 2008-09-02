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


#include "machinekey.hpp"

namespace psy
{
	namespace core
	{
	
		struct ToLower
		{
			char operator() (char c) const  { return std::tolower(c); }
		};


		MachineKey::MachineKey( )
			:host_(Hosts::INTERNAL)
			,dllName_(),
			index_(0) {
		}
		MachineKey::MachineKey( const MachineKey & key)
			:host_(key.host())
			,dllName_(key.dllName())
			,index_(key.index()) {
		}

		MachineKey::MachineKey(const Hosts::type host, const std::string & dllName, int index )
			:host_( host )
			,index_( index )
		{
			if (!dllName.empty()) {
				dllName_ = preprocessName(dllName);
			}
		}

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
		const MachineKey MachineKey::audioinput() {
			return MachineKey(Hosts::INTERNAL,"",InternalMacs::AUDIOINPUT );
		}
		const MachineKey MachineKey::lfo() {
			return MachineKey(Hosts::INTERNAL,"",InternalMacs::LFO );
		}
		const MachineKey MachineKey::failednative() {
			return MachineKey(Hosts::NATIVE,"",0);
		}


		const std::string MachineKey::preprocessName(std::string dllName) {
			{ // 1) remove extension
				std::string::size_type const pos(dllName.find(
					#if defined __unix__ || defined __APPLE__
						".so"
					#else
						".dll"
					#endif
				));
				if(pos != std::string::npos) dllName = dllName.substr(0, pos);
			}

			// 2) ensure lower case
			std::transform(dllName.begin(),dllName.end(),dllName.begin(),ToLower());
			
			// 3) replace spaces with underscores
			std::replace(dllName.begin(),dllName.end(),' ','_');

			{ // 4) remove prefix
				std::string const prefix("lib-xpsycle.plugin.");
				std::string::size_type const pos(dllName.find(prefix));
				if(pos == 0) dllName.erase(pos, prefix.length());
			}

			return dllName;
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
		bool MachineKey::operator !=( const MachineKey & rhs ) const {
			return host() != rhs.host() || dllName() != rhs.dllName() || index() != rhs.index();
		}
		MachineKey& MachineKey::operator=( const MachineKey & key ) {
			host_ = key.host();
			dllName_ = key.dllName();
			index_ = key.index();
			return *this;
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
