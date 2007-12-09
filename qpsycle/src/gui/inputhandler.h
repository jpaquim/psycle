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
#ifndef INPUTHANDLER_H
#define INPUTHANDLER_H
#include <map>
/**
@author  Psycledelics
*/
///\name key modifiers
///\{
	/// shift
	const int MOD_S = 1<<0;
	// control
	const int MOD_C = 1<<1;
	/// extended (???)
	const int MOD_E = 1<<2;
	/// ???
	const int MOD_MAX = 1<<3;
///\}

/// ???
const int max_cmds = 1024;

class Machine;
class CChildView;
class CMainFrame;

/// command types
enum CmdType
{
	CT_Null,
	CT_Note,
	CT_Editor,
	CT_Immediate
};

///\name command definitions
	/// IMPORTANT: the enum indexes determine what kind of
	/// command it is, and for keys determine the note value
	///\{
	/// .
	const int CS_KEY_START = 0;
	/// .
	const int CS_IMM_START = 256;
	/// .
	const int CS_EDT_START = 512;
	/// .
	const int CS_LAST = max_cmds;
///\}

/// command set.
namespace commands {
	enum command
	{
		null = -1,
		// keys
		key_C_0 = CS_KEY_START,
		key_CS0,
		key_D_0,
		key_DS0,
		key_E_0,
		key_F_0,
		key_FS0,
		key_G_0,
		key_GS0,
		key_A_0,
		key_AS0,
		key_B_0,
		key_C_1, ///< 12
		key_CS1,
		key_D_1,
		key_DS1,
		key_E_1,
		key_F_1,
		key_FS1,
		key_G_1,
		key_GS1,
		key_A_1,
		key_AS1,
		key_B_1,
		key_C_2, ///< 24
		key_CS2,
		key_D_2,
		key_DS2,
		key_E_2,
		key_F_2,
		key_FS2,
		key_G_2,
		key_GS2,
		key_A_2,

		key_stop = 120, ///< NOTE STOP
		tweak = 121, ///< tweak
		tweak_effect = 122, ///< tweak effect. Old!
		midi_cc = 123, ///< Mcm Command (MIDI CC)
		tweak_slide = 124, ///< tweak slide command

		// immediate commands

		edit_toggle = CS_IMM_START,

		octave_up,
		octave_down,

		machine_dec,
		machine_inc,
		pattern_step_dec,
		pattern_step_inc,

		instrument_dec,
		instrument_inc,

		play_row_track,
		play_row_pattern,
		play_start,
		play_song,
		play_from_position,
		play_block,
		play_stop,
		play_loop_entry,

		info_pattern,
		info_machine,

		show_pattern_box,
		show_machine_view,
		show_pattern_view,
		show_wave_editor,
		show_sequencer_view,

		add_machine,

		pattern_inc,
		pattern_dec,
		
		song_position_inc,
		song_position_dec,

		track_next, ///< s-tab
		track_prev, ///< tab

		navigate_up,
		navigate_down,
		navigate_left,
		navigate_right,
		navigate_page_up, ///< pgup
		navigate_page_down, ///< pgdn
		navigate_top, ///< home
		navigate_bottom, ///< end
		navigate_first_track,
		navigate_last_track,

		select_machine, ///< Enter
		undo,
		redo,
		follow_sequence,
		max_pattern,
		error_log,

		transpose_channel_inc = CS_EDT_START,
		transpose_channel_dec,
		transpose_channel_inc12,
		transpose_channel_dec12,
		transpose_block_inc,
		transpose_block_dec,
		transpose_block_inc12,
		transpose_block_dec12,

		pattern_cut,
		pattern_copy,
		pattern_paste,

		row_insert,
		row_delete,
		row_clear,
		
		block_start,
		block_end,
		block_unmark,
		block_double,
		block_halve,
		block_cut,
		block_copy,
		block_paste,
		block_mix,
		block_switch,
		block_interpolate,
		block_set_machine,
		block_set_instrument,
		
		select_all,
		select_track,
		select_up,
		select_down,
		select_left,
		select_right,
		select_top,
		select_bottom,

		edit_quantitize_dec,
		edit_quantitize_inc,

		// new ones have to go at bottom of each section or else bad registry reads
		pattern_mix_paste,
		pattern_track_mute,
		key_stop_any, ///< NOTE STOP
		pattern_delete,
		block_delete,
		pattern_track_solo,
		pattern_track_record,
		select_bar,
		mute_machine,
		solo_machine,
		bypass_machine
	};
}

/// command definitions.
class CmdDef
{
	public:
		/// unique identifier
		commands::command ID;
		/// cmd specific data - e.g. note reference
		//int data;
		/// string description
		//CString desc;
		/// function callback, not currently used
		//void * callback;
		
		CmdDef(commands::command ID = commands::null) : ID(ID) {}

		CmdType GetType() const;

		bool operator==(CmdDef const & other) const { return ID == other.ID; }

		inline bool IsValid() const { return ID != commands::null; }

		int GetNote() const;
		const char * GetName() const;
};

class Key
{
	public:
		Key() : shift_(), vkey_() {}
		Key( int shift, int key ) : shift_(shift), vkey_(key) {}

		bool operator<(const Key & key) const;

		int vkey() const { return vkey_; }
		int shift() const { return shift_; }

	private:
		int shift_;
		int vkey_;
};

class InputHandler
{
	public:
		void changeKeyCode(int keyEnumCode, const Key & key);
		int getEnumCodeByKey(const Key & key) const;

	private:
		std::map<Key,int> keyMap;
};
#endif
