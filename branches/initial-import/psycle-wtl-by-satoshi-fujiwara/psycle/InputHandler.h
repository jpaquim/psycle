/** @file
 *  @brief header file
 *  $Date: 2005/01/15 22:44:20 $
 *  $Revision: 1.4 $
 */
// InputHandler.h: interface for the CInputHandler class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_InputHandler_H__BE3B4B0C_C330_46B6_A52E_75F094503556__INCLUDED_)
#define AFX_InputHandler_H__BE3B4B0C_C330_46B6_A52E_75F094503556__INCLUDED_


#include "constants.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


// key modifiers
const int MOD_S = 1<<0;
const int MOD_C = 1<<1;
const int MOD_E = 1<<2;
const int MOD_MAX = 1<<3;

const int max_cmds = 1024;

class Machine;
class CPsycleWTLView;
class CMainFrame;

// command types
enum CmdType
{
	CT_Null,
    CT_Note,
    CT_Editor,
    CT_Immediate
};



// command definitions
// IMPORTANT: the enum indexes determine what kind of
// command it is, and for keys determine the note value

const int CS_KEY_START = 0;
const int CS_IMM_START = 256;
const int CS_EDT_START = 512;
const int CS_LAST = 1024;

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
	cdefKeyA_1,
	cdefKeyAS1,
	cdefKeyB_1,
	cdefKeyC_1, // 12
	cdefKeyCS1,
	cdefKeyD_1,
	cdefKeyDS1,
	cdefKeyE_1,
	cdefKeyF_1,
	cdefKeyFS1,
	cdefKeyG_1,
	cdefKeyGS1,
	cdefKeyA_2,
	cdefKeyAS2,
	cdefKeyB_2,
	cdefKeyC_2, // 24
	cdefKeyCS2,
	cdefKeyD_2,
	cdefKeyDS2,
	cdefKeyE_2,
	cdefKeyF_2,
	cdefKeyFS2,
	cdefKeyG_2,
	cdefKeyGS2,
	cdefKeyA_3,	
	
	cdefKeyStop = 120,	// NOTE STOP
	cdefTweakM = 121,	// tweak
	cdefTweakE = 122,	// tweak effect. Old!
	cdefMIDICC = 123,	// Mcm Command (MIDI CC)
	cdefTweakS = 124,
	cdefEffect = 125,	// 他トラックのエフェクトをかける



	// immediate commands	
	cdefEditToggle = CS_IMM_START,

	cdefOctaveUp,
	cdefOctaveDn,

	cdefMachineDec,
	cdefMachineInc,

	cdefInstrDec,
	cdefInstrInc,

	cdefPlayRowTrack,
	cdefPlayRowPattern,
	cdefPlayStart,
	cdefPlaySong,
	cdefPlayFromPos,
	cdefPlayBlock,
	cdefPlayStop,

	cdefInfoPattern,
	cdefInfoMachine,

	cdefEditMachine,
	cdefEditPattern,
	cdefEditInstr,
	cdefAddMachine,

	cdefPatternInc,
	cdefPatternDec,
	
	cdefSongPosInc,
	cdefSongPosDec,

	cdefColumnPrev,		// tab
	cdefColumnNext,		// s-tab

	cdefNavUp,
	cdefNavDn,
	cdefNavLeft,
	cdefNavRight,
	cdefNavPageUp,	// pgup
	cdefNavPageDn,	// pgdn
	cdefNavTop,		// home
	cdefNavBottom,	// end

	cdefSelectMachine,	// Enter
	cdefUndo,
	cdefRedo,

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
	cdefBlockInterpolate,
	cdefBlockSetMachine,
	cdefBlockSetInstr,
	
	cdefSelectAll,
	cdefSelectCol,

	cdefEditQuantizeDec,
	cdefEditQuantizeInc,

	// new ones have to go at bottom of each section or else bad registry reads
	cdefPatternMixPaste,
	cdefPatternTrackMute,
	cdefKeyStopAny,	// NOTE STOP
	cdefPatternDelete,
	cdefBlockDelete,
	cdefPatternTrackSolo,
	cdefPatternTrackRecord,

};



// CmdDef struct
// command definitions
struct CmdDef
{
    CmdSet ID;			// unique identifier
    //int data;			// cmd specific data - e.g. note reference		
	//CString desc;		// string description     
	//void * callback;	// function callback, not currently used
	
	CmdDef(CmdSet _ID=cdefNull)
	{
		ID=_ID;
	}

	CmdType GetType()
	{
		if(ID<CS_KEY_START)
			return CT_Null;

		if(ID<CS_IMM_START)
			return CT_Note;

		if(ID<CS_EDT_START)
			return CT_Immediate;

		return CT_Editor;
	}

	bool operator==(CmdDef other)
	{
		return (ID==other.ID);	
	}

	bool IsValid()
	{
		return (ID!=cdefNull);
	}

	int GetNote()
	{
		if(GetType()==CT_Note)
			return ID;
		else
			return -1;
	}

	// TODO: move to lookup table
	const TCHAR * GetName()
	{
		switch(ID)
		{
		case cdefNull: return _T("-");

		case cdefKeyC_0: return _T("Key (Oct.0) C");
		case cdefKeyCS0: return _T("Key (Oct.0) C#");
		case cdefKeyD_0: return _T("Key (Oct.0) D");
		case cdefKeyDS0: return _T("Key (Oct.0) D#");
		case cdefKeyE_0: return _T("Key (Oct.0) E");
		case cdefKeyF_0: return _T("Key (Oct.0) F");
		case cdefKeyFS0: return _T("Key (Oct.0) F#");
		case cdefKeyG_0: return _T("Key (Oct.0) G");
		case cdefKeyGS0: return _T("Key (Oct.0) G#");
		case cdefKeyA_1: return _T("Key (Oct.0) A");
		case cdefKeyAS1: return _T("Key (Oct.0) A#");
		case cdefKeyB_1: return _T("Key (Oct.0) B");
		case cdefKeyC_1: return _T("Key (Oct.1) C");
		case cdefKeyCS1: return _T("Key (Oct.1) C#");
		case cdefKeyD_1: return _T("Key (Oct.1) D");
		case cdefKeyDS1: return _T("Key (Oct.1) D#");
		case cdefKeyE_1: return _T("Key (Oct.1) E");
		case cdefKeyF_1: return _T("Key (Oct.1) F");
		case cdefKeyFS1: return _T("Key (Oct.1) F#");
		case cdefKeyG_1: return _T("Key (Oct.1) G");
		case cdefKeyGS1: return _T("Key (Oct.1) G#");
		case cdefKeyA_2: return _T("Key (Oct.1) A");
		case cdefKeyAS2: return _T("Key (Oct.1) A#");
		case cdefKeyB_2: return _T("Key (Oct.1) B");
		case cdefKeyC_2: return _T("Key (Oct.2) C");
		case cdefKeyCS2: return _T("Key (Oct.2) C#");
		case cdefKeyD_2: return _T("Key (Oct.2) D");
		case cdefKeyDS2: return _T("Key (Oct.2) D#");
		case cdefKeyE_2: return _T("Key (Oct.2) E");
		case cdefKeyF_2: return _T("Key (Oct.2) F");
		case cdefKeyFS2: return _T("Key (Oct.2) F#");
		case cdefKeyG_2: return _T("Key (Oct.2) G");
		case cdefKeyGS2: return _T("Key (Oct.2) G#");
		case cdefKeyA_3: return _T("Key (Oct.2) A");

		case cdefKeyStop: return _T("Key Stop");
		case cdefKeyStopAny: return _T("Key Stop Current");
		case cdefTweakM:  return _T("Tweak (Machine)");
		case cdefTweakS:  return _T("Tweak Smooth (Machine)");
		case cdefMIDICC:  return _T("Mcm (MIDI CC)");

		case cdefColumnPrev:	return _T("Prev column");
		case cdefColumnNext:	return _T("Next column");
		case cdefNavUp:			return _T("Nav. Up");
		case cdefNavDn:			return _T("Nav. Down");
		case cdefNavLeft:		return _T("Nav. Left");
		case cdefNavRight:		return _T("Nav. Right");
		case cdefNavPageDn:		return _T("Nav. Down 16");
		case cdefNavPageUp:		return _T("Nav. Up 16");
		case cdefNavTop:		return _T("Nav. Top");
		case cdefNavBottom:		return _T("Nav. Bottom");

		case cdefSelectMachine:	return _T("Select Mac/Ins in Cursor Pos");
		
		case cdefRowInsert:		return _T("Insert Row");
		case cdefRowDelete:		return _T("Delete Row");
		case cdefRowClear:		return _T("Clear Row");

		case cdefPatternCut:	return _T("Pattern Cut");
		case cdefPatternCopy:	return _T("Pattern Copy");
		case cdefPatternPaste:	return _T("Pattern Paste");
		case cdefPatternMixPaste:	return _T("Pattern Mix Paste");
		case cdefPatternTrackMute:	return _T("Pattern Track Mute");
		case cdefPatternTrackSolo:	return _T("Pattern Track Solo");
		case cdefPatternTrackRecord:	return _T("Pattern Track Record");
		case cdefPatternDelete:	return _T("Pattern Delete");
				
		case cdefBlockCut:		return _T("Block Cut");
		case cdefBlockCopy:		return _T("Block Copy");
		case cdefBlockPaste:	return _T("Block Paste");
		case cdefBlockDelete:	return _T("Block Delete");
		
		case cdefBlockStart:	return _T("Block Start");
		case cdefBlockEnd:		return _T("Block End");
		case cdefBlockUnMark:	return _T("Block Unmark");
		case cdefBlockDouble:	return _T("Block Double");
		case cdefBlockHalve:	return _T("Block Halve");
		case cdefBlockMix:		return _T("Block Mix");
		case cdefBlockInterpolate:	return _T("Block Interpolate");
		case cdefBlockSetMachine:	return _T("Block Set Machine");
		case cdefBlockSetInstr:		return _T("Block Set Instrument");

		case cdefSelectAll:		return _T("Block Select All");
		case cdefSelectCol:		return _T("Block Select Column");

		case cdefEditQuantizeDec:	return _T("Row-skip -1");
		case cdefEditQuantizeInc:	return _T("Row-skip +1");

		case cdefTransposeChannelInc:	return _T("Transpose Channel +1");
		case cdefTransposeChannelDec:	return _T("Transpose Channel -1");
		case cdefTransposeChannelInc12:	return _T("Transpose Channel +12");	
		case cdefTransposeChannelDec12:	return _T("Transpose Channel -12");

		case cdefTransposeBlockInc:		return _T("Transpose Block +1");
		case cdefTransposeBlockDec:		return _T("Transpose Block -1");
		case cdefTransposeBlockInc12:	return _T("Transpose Block +12");
		case cdefTransposeBlockDec12:	return _T("Transpose Block -12");

		case cdefEditToggle:	return _T("Toggle Edit Mode");

		case cdefOctaveDn:	return _T("Current Octave -1");
		case cdefOctaveUp:	return _T("Current Octave +1");	

		case cdefMachineDec:	return _T("Current Machine -1");
		case cdefMachineInc:	return _T("Current Machine +1");

		case cdefInstrDec:	return _T("Current Instrument -1");
		case cdefInstrInc:	return _T("Current Instrument +1");

		case cdefPlayRowTrack:	return _T("Play Current Note");
		case cdefPlayRowPattern:return _T("Play Current Row");
		case cdefPlayStart:		return _T("Play Song (from start)");
		case cdefPlaySong:		return _T("Play Song (normal)");
		case cdefPlayFromPos:	return _T("Play Song (from current row)");
		case cdefPlayBlock:		return _T("Play Sel Pattern(s) Looped");
		case cdefPlayStop:		return _T("Stop Playback");

		case cdefInfoPattern:	return _T("Pattern Info");
		case cdefInfoMachine:	return _T("Machine Info");

		case cdefEditMachine:	return _T("Screen of Machines");
		case cdefEditPattern:	return _T("Screen of Patterns");
		case cdefEditInstr:		return _T("Edit Instrument");
		case cdefAddMachine:		return _T("Add New Machine");

		case cdefPatternInc:		return _T("Current Pattern +1");
		case cdefPatternDec:		return _T("Current Pattern -1");
		case cdefSongPosInc:		return _T("Position +1");
		case cdefSongPosDec:		return _T("Position -1");

		case cdefUndo:		return _T("Edit Undo");
		case cdefRedo:		return _T("Edit Redo");

		default: return _T("Invalid");
		}
	}
};

class InputHandler  
{
public:
	// initialization
	InputHandler();
	virtual ~InputHandler();
	
	void BuildCmdLUT();

private:
	// get key modifier index
	UINT GetModifierIdx(UINT nFlags)
	{
		UINT idx=0;
		if(GetKeyState(VK_SHIFT)<0) idx|=MOD_S;		// shift?
		if(GetKeyState(VK_CONTROL)<0) idx|=MOD_C;	// ctrl?
		if(nFlags&(1<<8)) idx|=MOD_E;				// extended?
		return idx;
	}

public:
	void SetChildView(CPsycleWTLView*p){pChildView=p;}
	void SetMainFrame(CMainFrame*p){pMainFrame=p;}

private:
	CPsycleWTLView * pChildView;
	CMainFrame * pMainFrame;

public:	
	// translation	
	void CmdToKey(CmdDef cse,WORD & key,WORD & mods);	
	CmdDef KeyToCmd(UINT nChar, UINT nFlags);
	CmdDef StringToCmd(LPCTSTR str);

public:
	// control 	
	void PerformCmd(CmdDef cmd,BOOL brepeat);
	
	// commands
	bool EnterData(UINT nChar,UINT nFlags);
	void PlayNote(int note,int velocity=127,bool bTranspose=true,Machine*pMachine=NULL);
	void StopNote(int note,bool bTranspose=true,Machine*pMachine=NULL);

	void Stop();
	void PlaySong();
	void PlayFromCur();
	
public:
	// store/load
	bool SetCmd(CmdDef cmd, UINT key, UINT modifiers);
	bool ConfigSave();
	bool ConfigRestore();

	
public:	
	// behaviour
	bool bCtrlPlay;		// right ctrl mapped to PLAY?
	bool bMultiKey;		// multikey playback?
	bool bFT2HomeBehaviour;	// FT2 style Home/End?
	bool bFT2DelBehaviour;	// FT2 style Delete line?
	bool bShiftArrowsDoSelect; // Shift+Arrows do act as selection?
	bool bDoingSelection;		// Indicates that Shift+Arrow is Selection.

	// multi-key playback state stuff
	int notetrack[MAX_TRACKS]; 
	int outtrack;		// last track output to	

	// LUT for command data
	CmdDef cmdLUT[MOD_MAX][256];		
private:
};




























#endif // !defined(AFX_InputHandler_H__BE3B4B0C_C330_46B6_A52E_75F094503556__INCLUDED_)
