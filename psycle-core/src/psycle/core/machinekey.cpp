// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2009 members of the psycle project http://psycle.sourceforge.net

#include <psycle/core/config.private.hpp>
#include "machinekey.hpp"
#include "internalhost.hpp"

#include <algorithm>

namespace psycle { namespace core {
	
		struct ToLower {
			char operator() (char c) const  { return std::tolower(c); }
		};

		const MachineKey MachineKey::invalid(Hosts::INTERNAL,"<invalid>", -1);
		const MachineKey MachineKey::master(Hosts::INTERNAL,"<master>", InternalMacs::MASTER);
		const MachineKey MachineKey::dummy(Hosts::INTERNAL,"<dummy>", InternalMacs::DUMMY);
		const MachineKey MachineKey::sampler(Hosts::INTERNAL,"<sampler>", InternalMacs::SAMPLER );
		const MachineKey MachineKey::sampulse(Hosts::INTERNAL,"<xm-sampler>", InternalMacs::XMSAMPLER );
		const MachineKey MachineKey::duplicator(Hosts::INTERNAL,"<duplicator>", InternalMacs::DUPLICATOR);
		const MachineKey MachineKey::mixer(Hosts::INTERNAL,"<mixer>", InternalMacs::MIXER );
		const MachineKey MachineKey::audioinput(Hosts::INTERNAL,"<audio-input>", InternalMacs::AUDIOINPUT );
		const MachineKey MachineKey::lfo(Hosts::INTERNAL,"<lfo>", InternalMacs::LFO );
		const MachineKey MachineKey::failednative(Hosts::NATIVE,"<failed-native>", 0);
		const MachineKey MachineKey::wrapperVst(Hosts::VST,"<vst-wrapper>", 0);


		MachineKey::MachineKey( )
		:
			dllName_(),
			host_(Hosts::INTERNAL),
			index_(-1)
		{}
		MachineKey::MachineKey( const MachineKey & key)
		:
			dllName_(key.dllName()),
			host_(key.host()),
			index_(key.index())
		{}

		MachineKey::MachineKey(const Hosts::type host, const std::string & dllName, std::uint32_t index )
		:
			host_(host),
			index_(index)
		{
			if(!dllName.empty()) dllName_ = preprocessName(dllName);
		}

		MachineKey::~MachineKey() {
		}


		const std::string MachineKey::preprocessName(std::string dllName) {
			#if 0
			std::cout << "preprocess in: " << dllName << std::endl;
			#endif
			{ // 1) remove extension
				std::string::size_type pos(dllName.find(".so"));
				if(pos != std::string::npos) dllName = dllName.substr(0, pos);

				pos = dllName.find(".dll");
				if(pos != std::string::npos) dllName = dllName.substr(0, pos);
			}

			// 2) ensure lower case
			std::transform(dllName.begin(),dllName.end(),dllName.begin(),ToLower());
			
			// 3) replace spaces and underscores with dash.
			std::replace(dllName.begin(),dllName.end(),' ','-');
			std::replace(dllName.begin(),dllName.end(),'_','-');

			{ // 4) remove prefix
				std::string const prefix("libpsycle-plugin-");
				std::string::size_type const pos(dllName.find(prefix));
				if(pos == 0) dllName.erase(pos, prefix.length());
			}
			#if 0
			std::cout << "preprocess out: " << dllName << std::endl;
			#endif
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
		std::uint32_t MachineKey::index() const {
			return index_;
		}
}}
