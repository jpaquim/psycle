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

#ifndef MACHINEKEY_HPP
#define MACHINEKEY_HPP

#include <string>

namespace psy
{
	namespace core
	{
		// type Hosts::type
		// Allows to differentiate between machines of different hosts.
		namespace Hosts {
			typedef enum 
			{
				INTERNAL=0,
				NATIVE,
				LADSPA,
				VST,
				//Keep at last position
				NUM_HOSTS
			} type;
		}
		// type InternalMacs::type
		// Allows to differentiate between internal machines.
		// Note: This should be known only to InternalHost, but it is here
		// so that MachineKey can return the keys of internal machines.
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
				LFO,
				//Keep at Last position.
				NUM_MACS
			} type;
		}


		class MachineKey
		{
		public:
			MachineKey();
			MachineKey(const Hosts::type host, const std::string & dllName, const int index = 0 );
			MachineKey(const MachineKey& key);
			~MachineKey();

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
			static const std::string preprocessName(std::string dllName);

			const std::string & dllName() const;
			const Hosts::type host() const;
			int index() const;

			bool operator<( const MachineKey & key) const;
			bool operator==( const MachineKey & rhs ) const;
			bool operator!=( const MachineKey & rhs ) const;
			MachineKey& operator=( const MachineKey & key );
		private:
			std::string dllName_;
			Hosts::type host_;
			int index_;
		};
	}
}

#endif
