// This program is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
// You should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
// copyright 2007-2009 members of the psycle project http://psycle.sourceforge.net

#ifndef PSYCLE__CORE__PRESET__INCLUDED
#define PSYCLE__CORE__PRESET__INCLUDED
#pragma once

#include <psycle/core/config.hpp>

#include <vector>
#include <string>

namespace psycle { namespace helpers {
	class BinRead;
}}

namespace psy { namespace core {

		class Machine;

		class PSYCLE__CORE__DECL Preset {
		public:

			Preset();

			Preset( int numpars, int dataSize );

			bool read( psycle::helpers::BinRead& prsIn );

			const std::string& name() const;
			const std::vector<int>& parameter() const;

			void tweakMachine( Machine & mac );

		private:

			std::string name_;

			std::vector<int> params_;
			std::vector<char> data_;
		};

}}
#endif
