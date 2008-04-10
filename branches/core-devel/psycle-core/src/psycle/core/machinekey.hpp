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
				VST,
				LADSPA,
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
		protected:
			MachineKey( );
		public:
			MachineKey( const Hosts::type host, const std::string & dllName, int index = 0 );
			~MachineKey();

			static const MachineKey MachineKey::master() const;
			static const MachineKey MachineKey::dummy() const;
			static const MachineKey MachineKey::sampler) const;
			static const MachineKey MachineKey::sampulse() const;
			static const MachineKey MachineKey::duplicator() const;
			static const MachineKey MachineKey::mixer() const;
			static const MachineKey MachineKey::audioinput() const;
			static const MachineKey MachineKey::lfo() const;

			static const std::string MachineKey::preprocessName(std::string dllName);

			const std::string & dllName() const;
			const Hosts::type host() const;
			int index() const;

			bool operator<(const MachineKey & key) const;
			bool operator==( const MachineKey & rhs ) const;
		private:
			std::string dllName_;
			Hosts::type host_;
			int index_;
		};
	}
}

#endif
