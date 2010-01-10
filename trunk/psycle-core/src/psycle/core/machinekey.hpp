// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2009 members of the psycle project http://psycle.sourceforge.net

#ifndef PSYCLE__CORE__MACHINE_KEY__INCLUDED
#define PSYCLE__CORE__MACHINE_KEY__INCLUDED
#pragma once

#include <psycle/core/config.hpp>

#include <universalis/stdlib/cstdint.hpp>
#include <string>

namespace psycle { namespace core {

// type Hosts::type
// Allows to differentiate between machines of different hosts.
namespace Hosts {
	enum type
	{
		INTERNAL=0,
		NATIVE,
		VST,
		LADSPA,
		//Keep at last position
		NUM_HOSTS
	};
}

class PSYCLE__CORE__DECL MachineKey {
	public:
		MachineKey();
		MachineKey(const Hosts::type host, const std::string & dllName, const uint32_t index = 0 );
		MachineKey(const MachineKey& key);
		~MachineKey();

		static const MachineKey invalid();
		static const MachineKey master();
		static const MachineKey dummy();
		static const MachineKey sampler();
		static const MachineKey sampulse();
		static const MachineKey duplicator();
		static const MachineKey mixer();
		static const MachineKey audioinput();
		static const MachineKey lfo();
		//Used by the psy2loader to parse the plugin part of the loader for a plugin that couldn't be loaded.
		static const MachineKey failednative();
		static const MachineKey wrapperVst();
		static const std::string preprocessName(std::string dllName);

		const std::string & dllName() const;
		const Hosts::type host() const;
		uint32_t index() const;

		bool operator<( const MachineKey & key) const;
		bool operator==( const MachineKey & rhs ) const;
		bool operator!=( const MachineKey & rhs ) const;
		MachineKey& operator=( const MachineKey & key );
	private:
		std::string dllName_;
		Hosts::type host_;
		uint32_t index_;
};

}}
#endif
