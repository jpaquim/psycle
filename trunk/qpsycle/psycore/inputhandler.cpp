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
#include "inputhandler.h"

namespace psy {
namespace core {

InputHandler::InputHandler()
{
}


InputHandler::~InputHandler()
{
}

int InputHandler::getEnumCodeByKey( const Key & key ) const
{
	std::map<Key,int>::const_iterator itr;
	if ( (itr = keyMap.find(key)) != keyMap.end() )
			return itr->second;
		else
			return -1;
}

CmdType CmdDef::GetType() const
{
  if(ID<CS_KEY_START)
    return CT_Null;
  
  if(ID<CS_IMM_START)
    return CT_Note;
  
  if(ID<CS_EDT_START)
    return CT_Immediate;
  
  return CT_Editor;
}

  int CmdDef::GetNote() const
{
  if(GetType()==CT_Note) {
    assert(ID >= 0 && ID < 128);
    return ID;
  }
  else
    return -1;
}

///\todo move to lookup table
const char * CmdDef::GetName() const
{
  switch(ID) {
    case cdefKeyC_0: return "Key (Oct.0) C";
    case cdefKeyCS0: return "Key (Oct.0) C#";
    case cdefKeyD_0: return "Key (Oct.0) D";
    case cdefKeyDS0: return "Key (Oct.0) D#";
    case cdefKeyE_0: return "Key (Oct.0) E";
    case cdefKeyF_0: return "Key (Oct.0) F";
    case cdefKeyFS0: return "Key (Oct.0) F#";
    case cdefKeyG_0: return "Key (Oct.0) G";
    case cdefKeyGS0: return "Key (Oct.0) G#";
    case cdefKeyA_0: return "Key (Oct.0) A";
    case cdefKeyAS0: return "Key (Oct.0) A#";
    case cdefKeyB_0: return "Key (Oct.0) B";
    case cdefKeyC_1: return "Key (Oct.1) C";
    case cdefKeyCS1: return "Key (Oct.1) C#";
    case cdefKeyD_1: return "Key (Oct.1) D";
    case cdefKeyDS1: return "Key (Oct.1) D#";
    case cdefKeyE_1: return "Key (Oct.1) E";
    case cdefKeyF_1: return "Key (Oct.1) F";
    case cdefKeyFS1: return "Key (Oct.1) F#";
    case cdefKeyG_1: return "Key (Oct.1) G";
    case cdefKeyGS1: return "Key (Oct.1) G#";
    case cdefKeyA_1: return "Key (Oct.1) A";
    case cdefKeyAS1: return "Key (Oct.1) A#";
    case cdefKeyB_1: return "Key (Oct.1) B";
    case cdefKeyC_2: return "Key (Oct.2) C";
    case cdefKeyCS2: return "Key (Oct.2) C#";
    case cdefKeyD_2: return "Key (Oct.2) D";
    case cdefKeyDS2: return "Key (Oct.2) D#";
    case cdefKeyE_2: return "Key (Oct.2) E";
    case cdefKeyF_2: return "Key (Oct.2) F";
    case cdefKeyFS2: return "Key (Oct.2) F#";
    case cdefKeyG_2: return "Key (Oct.2) G";
    case cdefKeyGS2: return "Key (Oct.2) G#";
    case cdefKeyA_2: return "Key (Oct.2) A";

    case cdefKeyStop: return "Key Stop";
    case cdefKeyStopAny: return "Key Stop Current";
    case cdefTweakM:  return "Tweak (Parameter)";
    case cdefTweakS:  return "Tweak Smooth (Parameter)";
    case cdefMIDICC:  return "Mcm (MIDI CC)";

    case cdefTrackPrev:	return "Prev track";
    case cdefTrackNext:	return "Next track";
    case cdefNavUp:			return "Nav. Up";
    case cdefNavDown:			return "Nav. Down";
    case cdefNavLeft:		return "Nav. Left";
    case cdefNavRight:		return "Nav. Right";
    case cdefNavPageDn:		return "Nav. Down 16";
    case cdefNavPageUp:		return "Nav. Up 16";
    case cdefNavTop:		return "Nav. Top";
    case cdefNavBottom:		return "Nav. Bottom";

    case cdefSelectMachine:	return "Select Mac/Ins in Cursor Pos";
				
    case cdefRowInsert:		return "Insert Row";
    case cdefRowDelete:		return "Delete Row";
    case cdefRowClear:		return "Clear Row";

    case cdefPatternCut:	return "Pattern Cut";
    case cdefPatternCopy:	return "Pattern Copy";
    case cdefPatternPaste:	return "Pattern Paste";
    case cdefPatternMixPaste:	return "Pattern Mix Paste";
    case cdefPatternTrackMute:	return "Pattern Track Mute";
    case cdefPatternTrackSolo:	return "Pattern Track Solo";
    case cdefPatternTrackRecord:	return "Pattern Track Record";
    case cdefFollowSong:	return "Toggle Follow Song";
    case cdefPatternDelete:	return "Pattern Delete";
						
    case cdefBlockCut:		return "Block Cut";
    case cdefBlockCopy:		return "Block Copy";
    case cdefBlockPaste:	return "Block Paste";
    case cdefBlockDelete:	return "Block Delete";
				
    case cdefBlockStart:	return "Block Start";
    case cdefBlockEnd:		return "Block End";
    case cdefBlockUnMark:	return "Block Unmark";
    case cdefBlockDouble:	return "Block Double";
    case cdefBlockHalve:	return "Block Halve";
    case cdefBlockMix:		return "Block Mix";
    case cdefBlockSwitch:	return "Block Switch";
    case cdefBlockInterpolate:	return "Block Interpolate";
    case cdefBlockSetMachine:	return "Block Set Machine";
    case cdefBlockSetInstr:		return "Block Set Instrument";

    case cdefSelectAll:		return "Block Select All";
    case cdefSelectTrack:		return "Block Select Track";
    case cdefSelectBar:		return "Block Select Bar";

    case cdefEditQuantizeDec:	return "Row-skip -1";
    case cdefEditQuantizeInc:	return "Row-skip +1";

    case cdefTransposeChannelInc:	return "Transpose Channel +1";
    case cdefTransposeChannelDec:	return "Transpose Channel -1";
    case cdefTransposeChannelInc12:	return "Transpose Channel +12";	
    case cdefTransposeChannelDec12:	return "Transpose Channel -12";

    case cdefTransposeBlockInc:		return "Transpose Block +1";
    case cdefTransposeBlockDec:		return "Transpose Block -1";
    case cdefTransposeBlockInc12:	return "Transpose Block +12";
    case cdefTransposeBlockDec12:	return "Transpose Block -12";

    case cdefEditToggle:	return "Toggle Edit Mode";

    case cdefOctaveDn:	return "Current Octave -1";
    case cdefOctaveUp:	return "Current Octave +1";	

    case cdefMachineDec:	return "Current Machine -1";
    case cdefMachineInc:	return "Current Machine +1";

    case cdefInstrDec:	return "Current Instrument -1";
    case cdefInstrInc:	return "Current Instrument +1";

    case cdefPlayRowTrack:	return "Play Current Note";
    case cdefPlayRowPattern:return "Play Current Row";
    case cdefPlayStart:		return "Play Song (from start)";
    case cdefPlaySong:		return "Play Song (normal)";
    case cdefPlayFromPos:	return "Play Song (from current row)";
    case cdefPlayBlock:		return "Play Sel Pattern(s) Looped";
    case cdefPlayStop:		return "Stop Playback";

    case cdefInfoPattern:	return "Pattern Info";
    case cdefInfoMachine:	return "Machine Info";

    case cdefShowPatternBox:	return "Show pattern box";
    case cdefShowMachineView:	return "Screen of Machines";
    case cdefShowPatternView:	return "Screen of Patterns";
    case cdefShowWaveEditor:	return "Screen of Waves";
    case cdefShowSequencerView:	return "Screen of Sequencing";
    case cdefAddMachine:		return "Add New Machine";
    case cdefMaxPattern:		return "Maximise Pattern View";

    case cdefPatternInc:		return "Current Pattern +1";
    case cdefPatternDec:		return "Current Pattern -1";
    case cdefSongPosInc:		return "Position +1";
    case cdefSongPosDec:		return "Position -1";

    case cdefUndo:		return "Edit Undo";
    case cdefRedo:		return "Edit Redo";

    case cdefErrorLog:	return "Show Error Log";

    case cdefMuteMachine: return "Mute Machine";
    case cdefSoloMachine: return "Solo Machine";

    case cdefNull:
    default:
      // This is a valid point. It is used when doing searches for name.
      return "Invalid" ;
    }
}

void InputHandler::changeKeyCode( int keyEnumCode, const Key & key )
{
	keyMap[key] = keyEnumCode;
}

Key::Key( int shift, int vkey )
{
	vkey_  = vkey;
	shift_ = shift;
}

bool Key::operator<(const Key & key) const {
  long key1 = shift()     | vkey() << 8;
  long key2 = key.shift() | key.vkey() <<8;
  return key1 < key2;
};

}
}
