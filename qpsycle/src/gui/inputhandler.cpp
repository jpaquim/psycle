/* -*- mode:c++, indent-tabs-mode:t -*- */
/***************************************************************************
*   Copyright (C) 2007 Psycledelics   *
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
#include <qpsyclePch.hpp>

#include "inputhandler.hpp"

int InputHandler::getEnumCodeByKey( const Key & key ) const
{
	std::map<Key,int>::const_iterator itr;
	if ( (itr = keyMap.find(key)) != keyMap.end() ) return itr->second;
	return -1;
}

CmdType CmdDef::GetType() const
{
	if(ID<CS_KEY_START) return CT_Null;
	if(ID<CS_IMM_START) return CT_Note;
	if(ID<CS_EDT_START) return CT_Immediate;
	return CT_Editor;
}

int CmdDef::GetNote() const
{
	if(GetType()==CT_Note) {
		assert(ID >= 0 && ID < 128);
		return ID;
	}
	return -1;
}

///\todo move to lookup table
const char * CmdDef::GetName() const
{
	switch(ID) {
		case commands::key_C_0: return "Key (Oct.0) C";
		case commands::key_CS0: return "Key (Oct.0) C#";
		case commands::key_D_0: return "Key (Oct.0) D";
		case commands::key_DS0: return "Key (Oct.0) D#";
		case commands::key_E_0: return "Key (Oct.0) E";
		case commands::key_F_0: return "Key (Oct.0) F";
		case commands::key_FS0: return "Key (Oct.0) F#";
		case commands::key_G_0: return "Key (Oct.0) G";
		case commands::key_GS0: return "Key (Oct.0) G#";
		case commands::key_A_0: return "Key (Oct.0) A";
		case commands::key_AS0: return "Key (Oct.0) A#";
		case commands::key_B_0: return "Key (Oct.0) B";
		case commands::key_C_1: return "Key (Oct.1) C";
		case commands::key_CS1: return "Key (Oct.1) C#";
		case commands::key_D_1: return "Key (Oct.1) D";
		case commands::key_DS1: return "Key (Oct.1) D#";
		case commands::key_E_1: return "Key (Oct.1) E";
		case commands::key_F_1: return "Key (Oct.1) F";
		case commands::key_FS1: return "Key (Oct.1) F#";
		case commands::key_G_1: return "Key (Oct.1) G";
		case commands::key_GS1: return "Key (Oct.1) G#";
		case commands::key_A_1: return "Key (Oct.1) A";
		case commands::key_AS1: return "Key (Oct.1) A#";
		case commands::key_B_1: return "Key (Oct.1) B";
		case commands::key_C_2: return "Key (Oct.2) C";
		case commands::key_CS2: return "Key (Oct.2) C#";
		case commands::key_D_2: return "Key (Oct.2) D";
		case commands::key_DS2: return "Key (Oct.2) D#";
		case commands::key_E_2: return "Key (Oct.2) E";
		case commands::key_F_2: return "Key (Oct.2) F";
		case commands::key_FS2: return "Key (Oct.2) F#";
		case commands::key_G_2: return "Key (Oct.2) G";
		case commands::key_GS2: return "Key (Oct.2) G#";
		case commands::key_A_2: return "Key (Oct.2) A";

		case commands::key_stop: return "Key Stop";
		case commands::key_stop_any: return "Key Stop Current";
		
		case commands::tweak: return "Tweak (Parameter)";
		case commands::tweak_slide: return "Tweak Smooth (Parameter)";
		
		case commands::midi_cc: return "Mcm (MIDI CC)";

		case commands::track_prev: return "Prev track";
		case commands::track_next: return "Next track";
		case commands::navigate_up: return "Nav. Up";
		case commands::navigate_down: return "Nav. Down";
		case commands::navigate_left: return "Nav. Left";
		case commands::navigate_right: return "Nav. Right";
		case commands::navigate_page_down: return "Nav. Down 16";
		case commands::navigate_page_up: return "Nav. Up 16";
		case commands::navigate_top: return "Nav. Top";
		case commands::navigate_bottom: return "Nav. Bottom";

		case commands::select_machine: return "Select Mac/Ins in Cursor Pos";
					
		case commands::row_insert: return "Insert Row";
		case commands::row_delete: return "Delete Row";
		case commands::row_clear: return "Clear Row";

		case commands::pattern_cut: return "Pattern Cut";
		case commands::pattern_copy: return "Pattern Copy";
		case commands::pattern_paste: return "Pattern Paste";
		case commands::pattern_mix_paste: return "Pattern Mix Paste";
		case commands::pattern_track_mute: return "Pattern Track Mute";
		case commands::pattern_track_solo: return "Pattern Track Solo";
		case commands::pattern_track_record: return "Pattern Track Record";
		case commands::pattern_delete: return "Pattern Delete";

		case commands::follow_sequence: return "Toggle Follow Song";
							
		case commands::block_cut: return "Block Cut";
		case commands::block_copy: return "Block Copy";
		case commands::block_paste: return "Block Paste";
		case commands::block_delete: return "Block Delete";
		case commands::block_start: return "Block Start";
		case commands::block_end: return "Block End";
		case commands::block_unmark: return "Block Unmark";
		case commands::block_double: return "Block Double";
		case commands::block_halve: return "Block Halve";
		case commands::block_mix: return "Block Mix";
		case commands::block_switch: return "Block Switch";
		case commands::block_interpolate: return "Block Interpolate";
		case commands::block_set_machine: return "Block Set Machine";
		case commands::block_set_instrument: return "Block Set Instrument";

		case commands::select_all: return "Block Select All";
		case commands::select_track: return "Block Select Track";
		case commands::select_bar: return "Block Select Bar";

		case commands::edit_quantitize_dec: return "Row-skip -1";
		case commands::edit_quantitize_inc: return "Row-skip +1";

		case commands::transpose_channel_inc: return "Transpose Channel +1";
		case commands::transpose_channel_dec: return "Transpose Channel -1";
		case commands::transpose_channel_inc12: return "Transpose Channel +12";
		case commands::transpose_channel_dec12: return "Transpose Channel -12";

		case commands::transpose_block_inc: return "Transpose Block +1";
		case commands::transpose_block_dec: return "Transpose Block -1";
		case commands::transpose_block_inc12: return "Transpose Block +12";
		case commands::transpose_block_dec12: return "Transpose Block -12";

		case commands::edit_toggle: return "Toggle Edit Mode";

		case commands::octave_down: return "Current Octave -1";
		case commands::octave_up: return "Current Octave +1";

		case commands::machine_dec: return "Current Machine -1";
		case commands::machine_inc: return "Current Machine +1";

		case commands::instrument_dec: return "Current Instrument -1";
		case commands::instrument_inc: return "Current Instrument +1";

		case commands::play_row_track: return "Play Current Note";
		case commands::play_row_pattern: return "Play Current Row";
		case commands::play_start: return "Play Song (from start)";
		case commands::play_song: return "Play Song (normal)";
		case commands::play_from_position: return "Play Song (from current row)";
		case commands::play_block: return "Play Sel Pattern(s) Looped";
		case commands::play_stop: return "Stop Playback";

		case commands::info_pattern: return "Pattern Info";
		case commands::info_machine: return "Machine Info";

		case commands::show_pattern_box: return "Show pattern box";
		case commands::show_machine_view: return "Screen of Machines";
		case commands::show_pattern_view: return "Screen of Patterns";
		case commands::show_wave_editor: return "Screen of Waves";
		case commands::show_sequencer_view: return "Screen of Sequencing";
		case commands::add_machine: return "Add New Machine";
		case commands::max_pattern: return "Maximise Pattern View";

		case commands::pattern_inc: return "Current Pattern +1";
		case commands::pattern_dec: return "Current Pattern -1";
		case commands::song_position_inc: return "Position +1";
		case commands::song_position_dec: return "Position -1";

		case commands::undo: return "Edit Undo";
		case commands::redo: return "Edit Redo";

		case commands::error_log: return "Show Error Log";

		case commands::mute_machine: return "Mute Machine";
		case commands::solo_machine: return "Solo Machine";

		case commands::null:
		default:
			// This is a valid point. It is used when searching for name.
			return "Invalid" ;
	}
}

bool commands::isNote(int command) {
	return command >= key_C_0 && command <= key_A_2;
}

int commands::noteFromCommand(int command) {
	if (!isNote(command)) {
		return -1;
	}
	else {
		return command - key_C_0;
	}
}

void InputHandler::changeKeyCode( int keyEnumCode, const Key & key )
{
	keyMap[key] = keyEnumCode;
}

bool Key::operator<(const Key & key) const {
	if( vkey() != key.vkey() )
		return vkey() < key.vkey();
	else
		return shift() < key.shift();
}
