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
#include <cassert>
#include <map>
/**
@author  Psycledelics
*/
namespace psy {
	namespace core {

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
		enum CmdSet
		{
			cdefNull = -1,
			// keys
			cdefKeyC_0 = CS_KEY_START,
			cdefKeyCS0,
			cdefKeyD_0,
			cdefKeyDS0,
			cdefKeyE_0,
			cdefKeyF_0,
			cdefKeyFS0,
			cdefKeyG_0,
			cdefKeyGS0,
			cdefKeyA_0,
			cdefKeyAS0,
			cdefKeyB_0,
			cdefKeyC_1, ///< 12
			cdefKeyCS1,
			cdefKeyD_1,
			cdefKeyDS1,
			cdefKeyE_1,
			cdefKeyF_1,
			cdefKeyFS1,
			cdefKeyG_1,
			cdefKeyGS1,
			cdefKeyA_1,
			cdefKeyAS1,
			cdefKeyB_1,
			cdefKeyC_2, ///< 24
			cdefKeyCS2,
			cdefKeyD_2,
			cdefKeyDS2,
			cdefKeyE_2,
			cdefKeyF_2,
			cdefKeyFS2,
			cdefKeyG_2,
			cdefKeyGS2,
			cdefKeyA_2,	

			cdefKeyStop = 120,	///< NOTE STOP
			cdefTweakM = 121,	///< tweak
			cdefTweakE = 122,	///< tweak effect. Old!
			cdefMIDICC = 123,	///< Mcm Command (MIDI CC)
			cdefTweakS = 124,	///< tweak slide command

			// immediate commands

			cdefEditToggle = CS_IMM_START,

			cdefOctaveUp,
			cdefOctaveDn,

			cdefMachineDec,
			cdefMachineInc,
			cdefPatternstepDec,
			cdefPatternstepInc,

			cdefInstrDec,
			cdefInstrInc,

			cdefPlayRowTrack,
			cdefPlayRowPattern,
			cdefPlayStart,
			cdefPlaySong,
			cdefPlayFromPos,
			cdefPlayBlock,
			cdefPlayStop,
			cdefLoopEntry,

			cdefInfoPattern,
			cdefInfoMachine,

			cdefShowPatternBox,
			cdefShowMachineView,
			cdefShowPatternView,
			cdefShowWaveEditor,
			cdefShowSequencerView,

			cdefAddMachine,

			cdefPatternInc,
			cdefPatternDec,
			
			cdefSongPosInc,
			cdefSongPosDec,

			cdefTrackNext,		///< s-tab
			cdefTrackPrev,		///< tab

			cdefNavUp,
			cdefNavDown,
			cdefNavLeft,
			cdefNavRight,
			cdefNavPageUp,	///< pgup
			cdefNavPageDn,	///< pgdn
			cdefNavTop,		///< home
			cdefNavBottom,	///< end
			cdefNavFirstTrack,
			cdefNavLastTrack,

			cdefSelectMachine,	///< Enter
			cdefUndo,
			cdefRedo,
			cdefFollowSong,
			cdefMaxPattern,
			cdefErrorLog,

			cdefTransposeChannelInc = CS_EDT_START,	
			cdefTransposeChannelDec,
			cdefTransposeChannelInc12,
			cdefTransposeChannelDec12,
			cdefTransposeBlockInc,
			cdefTransposeBlockDec,
			cdefTransposeBlockInc12,
			cdefTransposeBlockDec12,

			cdefPatternCut,
			cdefPatternCopy,
			cdefPatternPaste,

			cdefRowInsert,
			cdefRowDelete,
			cdefRowClear,
			
			cdefBlockStart,
			cdefBlockEnd,
			cdefBlockUnMark,
			cdefBlockDouble,
			cdefBlockHalve,
			cdefBlockCut,
			cdefBlockCopy,
			cdefBlockPaste,
			cdefBlockMix,
			cdefBlockSwitch,
			cdefBlockInterpolate,
			cdefBlockSetMachine,
			cdefBlockSetInstr,
			
			cdefSelectAll,
			cdefSelectTrack,
			cdefSelectUp,
			cdefSelectDn,
			cdefSelectLeft,
			cdefSelectRight,
			cdefSelectTop,
			cdefSelectBottom,

			cdefEditQuantizeDec,
			cdefEditQuantizeInc,

			// new ones have to go at bottom of each section or else bad registry reads
			cdefPatternMixPaste,
			cdefPatternTrackMute,
			cdefKeyStopAny,	///< NOTE STOP
			cdefPatternDelete,
			cdefBlockDelete,
			cdefPatternTrackSolo,
			cdefPatternTrackRecord,
			cdefSelectBar,
			cdefMuteMachine,
			cdefSoloMachine,
			cdefBypassMachine

		};

		/// command definitions.
		class CmdDef
		{
			public:
				/// unique identifier
				CmdSet ID;			
				//int data;			// cmd specific data - e.g. note reference		
				//CString desc;		// string description     
				//void * callback;	// function callback, not currently used
				
				CmdDef(CmdSet ID=cdefNull) : ID(ID) {}

				CmdType GetType() const;

				bool operator==(CmdDef const & other) const { return ID == other.ID; }

				inline bool IsValid() const { return ID != cdefNull; }

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
	}
}
#endif
