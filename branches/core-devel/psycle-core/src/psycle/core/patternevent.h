/******************************************************************************
*  copyright 2007 members of the psycle project http://psycle.sourceforge.net *
*                                                                             *
*  This program is free software; you can redistribute it and/or modify       *
*  it under the terms of the GNU General Public License as published by       *
*  the Free Software Foundation; either version 2 of the License, or          *
*  (at your option) any later version.                                        *
*                                                                             *
*  This program is distributed in the hope that it will be useful,            *
*  but WITHOUT ANY WARRANTY; without even the implied warranty of             *
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the              *
*  GNU General Public License for more details.                               *
*                                                                             *
*  You should have received a copy of the GNU General Public License          *
*  along with this program; if not, write to the                              *
*  Free Software Foundation, Inc.,                                            *
*  59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.                  *
******************************************************************************/
#ifndef PSYCLE__CORE__PATTERN_EVENT__INCLUDED
#define PSYCLE__CORE__PATTERN_EVENT__INCLUDED
#pragma once

#include <cstdint>
#include <vector>

namespace psy { namespace core {

/**
@author  Psycledelics  
*/
class PatternEvent
{
	public:
		typedef std::pair<std::uint8_t,std::uint8_t> PcmType;
		typedef std::vector<PcmType> PcmListType;

		PatternEvent();

		void setNote(std::uint8_t value) { note_ = value; }
		std::uint8_t note() const { return note_; }

		void setInstrument(std::uint8_t instrument) { inst_ = instrument; }
		std::uint8_t instrument() const { return inst_; }

		void setMachine(std::uint8_t machine) { mach_ = machine; }
		std::uint8_t machine() const { return mach_; }

		void setCommand(std::uint8_t command) { cmd_ = command; }
		std::uint8_t command() const { return cmd_; }

		void setParameter(std::uint8_t parameter) { param_ = parameter; }
		std::uint8_t parameter() const { return param_; }

		void setVolume(std::uint8_t volume) { volume_ = volume; }
		std::uint8_t volume() const { return volume_; }

		bool empty() const { return note_ == 255 && inst_ == 255 && mach_ == 255 && cmd_ == 0 && param_ == 0; }

		std::string toXml(int track) const;

		PcmListType & paraCmdList() { return paraCmdList_; }

		void setSharp( bool b ) { sharp_ = b; }
		bool isSharp() const { return sharp_; }

	private: ///\todo the compiler/stdlib implementation has a reserved namespace consisting of all names prefixed with an underscore, so we should postfix private data rather than prefix them.
		std::uint8_t note_;
		std::uint8_t inst_;
		std::uint8_t mach_;
		std::uint8_t cmd_;
		std::uint8_t param_;
		std::uint8_t volume_;
		bool sharp_;
		PcmListType paraCmdList_;
};

}}
#endif
