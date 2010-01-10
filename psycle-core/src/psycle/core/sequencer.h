// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2009 members of the psycle project http://psycle.sourceforge.net

#ifndef PSYCLE__CORE__SEQUENCER__INCLUDED
#define PSYCLE__CORE__SEQUENCER__INCLUDED
#pragma once

#include "machine.h"

namespace psycle { namespace core {

class Sequencer {
	public:
		Sequencer();
		~Sequencer();
			
		CoreSong* song() { return song_; }
		void set_song(class CoreSong* song) { song_ = song; }

		void Work(unsigned int nframes);

		PlayerTimeInfo* time_info() { return time_info_; }
		void set_time_info(class PlayerTimeInfo* info) { time_info_ = info; }

	private:
		void execute_notes(double beat_offset, class PatternEvent& line);
		PlayerTimeInfo* time_info_;
		CoreSong* song_;
		/// stores which machine played last in each track. this allows you to not specify the machine number everytime in the pattern.
		Machine::id_type prev_machines_[MAX_TRACKS];
};

}}
#endif
