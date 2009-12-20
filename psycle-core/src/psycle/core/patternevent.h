// This program is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
// You should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
// copyright 2007-2009 members of the psycle project http://psycle.sourceforge.net

#ifndef PSYCLE__CORE__PATTERN_EVENT__INCLUDED
#define PSYCLE__CORE__PATTERN_EVENT__INCLUDED
#pragma once

#include <psycle/core/config.hpp>
#include <universalis/stdlib/cstdint.hpp>
#include <vector>
#include <string>

namespace psycle { namespace core {

class PSYCLE__CORE__DECL PatternEvent {
	public:
		typedef std::pair<uint8_t, uint8_t> PcmType;
		typedef std::vector<PcmType> PcmListType;

		PatternEvent();

		void setNote(uint8_t value) { note_ = value; }
		uint8_t note() const { return note_; }

		void setInstrument(uint8_t instrument) { inst_ = instrument; }
		uint8_t instrument() const { return inst_; }

		void setMachine(uint8_t machine) { mach_ = machine; }
		uint8_t machine() const { return mach_; }

		void setCommand(uint8_t command) { cmd_ = command; }
		uint8_t command() const { return cmd_; }

		void setParameter(uint8_t parameter) { param_ = parameter; }
		uint8_t parameter() const { return param_; }

		void setVolume(uint8_t volume) { volume_ = volume; }
		uint8_t volume() const { return volume_; }

		bool empty() const { return note_ == 255 && inst_ == 255 && mach_ == 255 && cmd_ == 0 && param_ == 0; }

		std::string toXml(int track) const;

		PcmListType & paraCmdList() { return paraCmdList_; }

		void set_track(int track) {
			track_ = track;
		}

		void set_sequence(int seq_track) {
			seq_track_ = seq_track;
		}

		int track() const { return track_; }
		int sequence_track() const { return seq_track_; }

		void set_time_offset(double offset) {
			offset_ = offset;
		}
		double time_offset() const { return offset_; }


	private: ///\todo the compiler/stdlib implementation has a reserved namespace consisting of all names prefixed with an underscore, so we should postfix private data rather than prefix them.
		uint8_t note_;
		uint8_t inst_;
		uint8_t mach_;
		uint8_t cmd_;
		uint8_t param_;
		uint8_t volume_;
		PcmListType paraCmdList_;
		int track_;
		int seq_track_;
		double offset_;
};

}}
#endif
