// Key Config

// InputHandler.cpp: implementation of the InputHandler class.
//
//////////////////////////////////////////////////////////////////////

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#include "stdafx.h"
#include "mainfrm.h"
#include "InputHandler.h"
#include "machine.h"
#include "Player.h"
#include "childview.h"

const char * sDefaultCfgName = "PsycleKeys.INI";

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

InputHandler::InputHandler()
{
	TRACE("Initializing InputHandler()\n");
	
	// init bools
	bCtrlPlay = true;
	bMultiKey = true;
	bFT2HomeBehaviour = true;
	bFT2DelBehaviour = true;
	bShiftArrowsDoSelect = false;
	bDoingSelection = false;

	// set up multi-channel playback
	for(UINT i=0;i<256;i++)
		notetrack[i]=-1;
	outtrack=0;
	bMultiKey=true;

	if(!ConfigRestore())
	{
		TRACE("Building default keys");
		BuildCmdLUT();
	}
}

InputHandler::~InputHandler()
{
	ConfigSave();
}

// SetCmd
// in: command def, key, modifiers
// out: true if we had to remove another definition
// TODO: more warnings if we are changing existing defs
bool InputHandler::SetCmd(CmdDef cmd, UINT key, UINT modifiers)
{	
	// clear note?
	if(!cmd.IsValid())
	{
		cmdLUT[modifiers][key].ID = cdefNull;
		return false;
	}

	// normal
/*	TRACE("Imm. Command: '%s' (%d) has been set to key %d, modifiers = %d%d%d\n",
		cmd.GetName(),
		int(cmd.ID),
		key,
		modifiers&MOD_S,
		modifiers&MOD_C,
		modifiers&MOD_E);
*/
	// remove last key def, if it exists
	UINT i,j;
	for(j=0;j<MOD_MAX;j++)
	{
		for(i=0;i<256;i++)
		{
			if(cmdLUT[j][i]==cmd)
			{
				TRACE("--> removing from [%d][%d]\n",j,i);
				cmdLUT[j][i]=cdefNull;					
			}
		}
	}

	// add new
	bool bCmdDeleted;
	bCmdDeleted = cmdLUT[modifiers][key].IsValid();
	cmdLUT[modifiers][key] = cmd;

	return bCmdDeleted;
}


// KeyToCmd
// IN: key + modifiers from OnKeyDown
// OUT: command mapped to key
CmdDef InputHandler::KeyToCmd(UINT nChar, UINT nFlags)
{
	bDoingSelection=false;
	// invalid?	
	if(nChar>255)
	{
		CmdDef cmdNull;
		return cmdNull;
	}

	// special: right control mapped to PLAY
	if(bCtrlPlay && GetKeyState(VK_RCONTROL)<0)
	{
		CmdDef cmdPlay;
		cmdPlay.ID = cdefPlaySong;
		return cmdPlay;
	}
	else
	{
		if (bShiftArrowsDoSelect && GetKeyState(VK_SHIFT)<0 && !(Global::pPlayer->_playing&&Global::pConfig->_followSong))
		{
			CmdDef cmdSel;
			switch (nChar)
			{
			case VK_UP:
				cmdSel.ID = cdefNavUp; 
				bDoingSelection=true; 
				return cmdSel; 
				break;
			case VK_LEFT:
				cmdSel.ID = cdefNavLeft; 
				bDoingSelection=true; 
				return cmdSel; 
				break;
			case VK_DOWN:
				cmdSel.ID = cdefNavDn; 
				bDoingSelection=true; 
				return cmdSel; 
				break;
			case VK_RIGHT:
				cmdSel.ID = cdefNavRight; 
				bDoingSelection=true; 
				return cmdSel; 
				break;
			case VK_HOME:
				cmdSel.ID = cdefNavTop; 
				bDoingSelection=true; 
				return cmdSel; 
				break;
			case VK_END:
				cmdSel.ID = cdefNavBottom; 
				bDoingSelection=true; 
				return cmdSel; 
				break;
			case VK_PRIOR:
				cmdSel.ID = cdefNavPageUp; 
				bDoingSelection=true; 
				return cmdSel; 
				break;
			case VK_NEXT:
				cmdSel.ID = cdefNavPageDn; 
				bDoingSelection=true; 
				return cmdSel; 
				break;
			}
		}

		// This comparison is to allow the "Shift+Note" (chord mode) to work.
		CmdDef thisCmd = cmdLUT[(GetModifierIdx(nFlags) & ~MOD_S)][nChar];
		if ( thisCmd.GetType() == CT_Note )
			return thisCmd;
		else
			return cmdLUT[GetModifierIdx(nFlags)][nChar];
	}
}	

// StringToCmd
// IN: command name (string)
// OUT: command
CmdDef InputHandler::StringToCmd(LPCTSTR str)
{
	CmdDef ret;
	for(UINT i=0;i<CS_LAST;i++)
	{
		ret.ID = CmdSet(i);
		if(!strcmp(ret.GetName(),str))
			return ret;
	}
	ret.ID = cdefNull;
	return ret;
}

// StringToCmd
// IN: command def
// OUT: key/mod command is defined for, 0/0 if not defined
void InputHandler::CmdToKey(CmdDef cmd,WORD & key,WORD &mods)
{
	key = 0;
	mods = 0;
	
	// hunt for key with cmd
	UINT i,j;
	for(j=0;j<MOD_MAX;j++)
	{
		for(i=0;i<256;i++)
		{
			if(cmdLUT[j][i]==cmd)
			{
				// found
				
				if(j&MOD_S)
					mods|=HOTKEYF_SHIFT;
				if(j&MOD_C)
					mods|=HOTKEYF_CONTROL;				
				if(j&MOD_E)
					mods|=HOTKEYF_EXT;

				key = i;
				return;
			}
		}
	}

	// not found
}

//ConfigSave
// save key config data
// returns true on success, false on fail
bool InputHandler::ConfigSave()
{

	CString sect;
	CString key;
	CString data;

	sect = "Info";
	key = "AppVersion";
	data = "N/A" ;
	WritePrivateProfileString(sect,key,data,sDefaultCfgName);
	key = "Description";
	data = "N/A";	
	WritePrivateProfileString(sect,key,data,sDefaultCfgName);	

	UINT i,j;
	
	// note keys
	sect = "Keys";
	WritePrivateProfileString(sect,NULL,NULL,sDefaultCfgName); 	// clear
	for(j=0;j<MOD_MAX;j++)
	{
		for(i=0;i<256;i++)
		{
			if(cmdLUT[j][i].IsValid())
			{
				key.Format("Key[%d]%03d",j,i);
				data.Format("%03d     ; cmd = '%s'",cmdLUT[j][i].ID,cmdLUT[j][i].GetName());
				WritePrivateProfileString(sect,key,data,sDefaultCfgName);
			}
		}
	}

	// option data
	sect = "Options";
	key = "bNewHomeBehaviour"; // Variable renamed to bFT2HomeBehaviour.
	data.Format("%d",bFT2HomeBehaviour);
	WritePrivateProfileString(sect,key,data,sDefaultCfgName);

	key = "bCtrlPlay";
	data.Format("%d",bCtrlPlay);
	WritePrivateProfileString(sect,key,data,sDefaultCfgName);

	key = "bMultiKey";
	data.Format("%d",bMultiKey);
	WritePrivateProfileString(sect,key,data,sDefaultCfgName);

	key = "bFt2DelBehaviour";
	data.Format("%d",bFT2DelBehaviour);
	WritePrivateProfileString(sect,key,data,sDefaultCfgName);

	key = "bShiftArrowsDoSelect";
	data.Format("%d",bShiftArrowsDoSelect);
	WritePrivateProfileString(sect,key,data,sDefaultCfgName);

	return true;
}

// ConfigRestore
// restore key config data
// returns true on success, false on fail 
bool InputHandler::ConfigRestore()
{
	CString sect;
	CString key;
	CString data;

	// check file
	sect = "Info";
	key = "AppVersion";
	data = "";	
	GetPrivateProfileString(sect,key,"",data.GetBufferSetLength(64),64,sDefaultCfgName);
	if(data=="")
		return false;

	
	// save key data
	UINT i,j;
	
	CmdSet ID;
	int cmddata;
	sect = "Keys";
	for(j=0;j<MOD_MAX;j++)
	{
		for(i=0;i<256;i++)
		{
			key.Format("Key[%d]%03d",j,i);
			cmddata = GetPrivateProfileInt(sect,key,-1,sDefaultCfgName);			
			ID = CmdSet(cmddata);
			if ( ID == cdefTweakE) ID = cdefMIDICC;// Old twf Command
			SetCmd(ID,i,j);
		}
	}
	WORD tmpkey, tmpmods;
	CmdToKey(cdefSelectMachine,tmpkey,tmpmods);
	if ( !tmpkey ) SetCmd(cdefSelectMachine,VK_RETURN,0);

	// option data
	sect = "Options";
	key = "bNewHomeBehaviour"; // Variable renamed to bFT2HomeBehaviour.
	bFT2HomeBehaviour = GetPrivateProfileInt(sect,key,1,sDefaultCfgName)?true:false;

	key = "bCtrlPlay";
	bCtrlPlay = GetPrivateProfileInt(sect,key,1,sDefaultCfgName)?true:false;

	key = "bMultiKey";
	bMultiKey = GetPrivateProfileInt(sect,key,1,sDefaultCfgName)?true:false;

	key = "bFT2DelBehaviour";
	bFT2DelBehaviour = GetPrivateProfileInt(sect,key,1,sDefaultCfgName)?true:false;

	key = "bShiftArrowsDoSelect";
	bShiftArrowsDoSelect = GetPrivateProfileInt(sect,key,1,sDefaultCfgName)?true:false;

	return true;
}


















// operations

// perform command
// TODO: move to a callback system... this is disgustingly messy
void InputHandler::PerformCmd(CmdDef cmd, BOOL brepeat)
{
	switch(cmd.ID)
	{
	case cdefNull:
		break;

	case cdefPatternCut:
		pChildView->patCut();
		break;

	case cdefPatternCopy:
		pChildView->patCopy();
		break;

	case cdefPatternPaste:
		pChildView->patPaste();
		break;

	case cdefPatternMixPaste:
		pChildView->patPaste();
		break;

	case cdefPatternDelete:
		pChildView->patDelete();
		break;

	case cdefPatternTrackMute:
		pChildView->patTrackMute();
		break;

	case cdefKeyStopAny:
		pChildView->EnterNoteoffAny();
		break;

	case cdefColumnNext:
		pChildView->bScrollDetatch=false;
		pChildView->AdvanceTrack(1,Global::pConfig->_wrapAround);
		break;

	case cdefColumnPrev:
		pChildView->bScrollDetatch=false;
		pChildView->PrevTrack(1,Global::pConfig->_wrapAround);
		break;

	case cdefNavLeft:
		pChildView->bScrollDetatch=false;
		if ( !bDoingSelection )
		{
			pChildView->PrevCol(Global::pConfig->_wrapAround);
			if ( bShiftArrowsDoSelect ) pChildView->BlockUnmark();
		}
		else
		{
			 if ( !pChildView->blockSelected )
			 {
				pChildView->StartBlock(pChildView->editcur.track,pChildView->editcur.line,pChildView->editcur.col);
			 }
			pChildView->PrevTrack(1,Global::pConfig->_wrapAround);
			pChildView->ChangeBlock(pChildView->editcur.track,pChildView->editcur.line,pChildView->editcur.col);
		}

		bDoingSelection = false;
		break;
	case cdefNavRight:
		pChildView->bScrollDetatch=false;
		if ( !bDoingSelection )
		{
			pChildView->NextCol(Global::pConfig->_wrapAround);
			if ( bShiftArrowsDoSelect ) pChildView->BlockUnmark();
		}
		else
		{
			if ( !pChildView->blockSelected)
			{
				pChildView->StartBlock(pChildView->editcur.track,pChildView->editcur.line,pChildView->editcur.col);
			}
			pChildView->AdvanceTrack(1,Global::pConfig->_wrapAround);
			pChildView->ChangeBlock(pChildView->editcur.track,pChildView->editcur.line,pChildView->editcur.col);
		}

		bDoingSelection = false;
		break;
	case cdefNavUp:
		pChildView->bScrollDetatch=false;
		if ( bDoingSelection && !pChildView->blockSelected)
		{
			pChildView->StartBlock(pChildView->editcur.track,pChildView->editcur.line,pChildView->editcur.col);
		}
		pChildView->PrevLine(1,Global::pConfig->_wrapAround);
		if ( bDoingSelection )
		{
			pChildView->ChangeBlock(pChildView->editcur.track,pChildView->editcur.line,pChildView->editcur.col);
		}
		else if ( bShiftArrowsDoSelect ) pChildView->BlockUnmark();
		bDoingSelection = false;
		break;
	case cdefNavDn:
		pChildView->bScrollDetatch=false;
		if ( bDoingSelection && !pChildView->blockSelected)
		{
			pChildView->StartBlock(pChildView->editcur.track,pChildView->editcur.line,pChildView->editcur.col);
		}
		pChildView->AdvanceLine(1,Global::pConfig->_wrapAround);
		if ( bDoingSelection )
		{
			pChildView->ChangeBlock(pChildView->editcur.track,pChildView->editcur.line,pChildView->editcur.col);
		}
		else if ( bShiftArrowsDoSelect ) pChildView->BlockUnmark();
		bDoingSelection = false;
		break;
	case cdefNavPageUp:
		pChildView->bScrollDetatch=false;
		if ( bDoingSelection && !pChildView->blockSelected)
		{
			pChildView->StartBlock(pChildView->editcur.track,pChildView->editcur.line,pChildView->editcur.col);
		}
		pChildView->PrevLine(16,false);
		if ( bDoingSelection )
		{
			pChildView->ChangeBlock(pChildView->editcur.track,pChildView->editcur.line,pChildView->editcur.col);
		}
		else if ( bShiftArrowsDoSelect ) pChildView->BlockUnmark();
		break;

	case cdefNavPageDn:
		pChildView->bScrollDetatch=false;
		if ( bDoingSelection && !pChildView->blockSelected)
		{
			pChildView->StartBlock(pChildView->editcur.track,pChildView->editcur.line,pChildView->editcur.col);
		}
		pChildView->AdvanceLine(16,false);
		if ( bDoingSelection )
		{
			pChildView->ChangeBlock(pChildView->editcur.track,pChildView->editcur.line,pChildView->editcur.col);
		}
		else if ( bShiftArrowsDoSelect ) pChildView->BlockUnmark();
		break;
	
	case cdefNavTop:				
		pChildView->bScrollDetatch=false;
		if ( bDoingSelection && !pChildView->blockSelected)
		{
			pChildView->StartBlock(pChildView->editcur.track,pChildView->editcur.line,pChildView->editcur.col);
		}
		if(bFT2HomeBehaviour)
		{
			pChildView->editcur.line=0;
		}
		else
		{
			if (pChildView->editcur.col != 0) 
				pChildView->editcur.col = 0;
			else 
				if ( pChildView->editcur.track != 0 ) 
					pChildView->editcur.track = 0;
				else 
					pChildView->editcur.line = 0;
		}
		if ( bDoingSelection )
		{
			pChildView->ChangeBlock(pChildView->editcur.track,pChildView->editcur.line,pChildView->editcur.col);
		}
		else if ( bShiftArrowsDoSelect ) pChildView->BlockUnmark();

		pChildView->Repaint(DMCursor);
		break;
	
	case cdefNavBottom:
		pChildView->bScrollDetatch=false;
		if ( bDoingSelection && !pChildView->blockSelected)
		{
			pChildView->StartBlock(pChildView->editcur.track,pChildView->editcur.line,pChildView->editcur.col);
		}
		if(bFT2HomeBehaviour)
		{
			pChildView->editcur.line=Global::_pSong->patternLines[Global::_pSong->playOrder[pChildView->editPosition]]-1;
		}
		else
		{		
			if (pChildView->editcur.col != 8) 
				pChildView->editcur.col = 8;
			else if ( pChildView->editcur.track != Global::_pSong->SONGTRACKS-1 ) 
				pChildView->editcur.track = Global::_pSong->SONGTRACKS-1;
			else 
				pChildView->editcur.line = Global::_pSong->patternLines[Global::_pSong->playOrder[pChildView->editPosition]]-1;
		}
		if ( bDoingSelection )
		{
			pChildView->ChangeBlock(pChildView->editcur.track,pChildView->editcur.line,pChildView->editcur.col);
		}
		else if ( bShiftArrowsDoSelect ) pChildView->BlockUnmark();


		pChildView->Repaint(DMCursor);
		break;
	
	case cdefRowInsert:
		pChildView->InsertCurr();
		break;

	case cdefRowDelete:
		pChildView->DeleteCurr();
		break;

	case cdefRowClear:
		pChildView->ClearCurr();		
		break;

	case cdefBlockStart:
		pChildView->StartBlock(pChildView->editcur.track,pChildView->editcur.line,pChildView->editcur.col);
		break;

	case cdefBlockEnd:
		pChildView->EndBlock(pChildView->editcur.track,pChildView->editcur.line,pChildView->editcur.col);
		break;

	case cdefSelectAll:
		{
			const int nl = Global::_pSong->patternLines[Global::_pSong->playOrder[pChildView->editPosition]];
			pChildView->StartBlock(0,0,0);
			pChildView->EndBlock(Global::_pSong->SONGTRACKS-1,nl-1,8);
		}
		break;
		
	case cdefSelectCol:
		{
			const int nl = Global::_pSong->patternLines[Global::_pSong->playOrder[pChildView->editPosition]];
			pChildView->StartBlock(pChildView->editcur.track,0,0);
			pChildView->EndBlock(pChildView->editcur.track,nl-1,8);
		}
		break;

	case cdefEditQuantizeDec:
		pMainFrame->EditQuantizeChange(-1);
		break;

	case cdefEditQuantizeInc:
		pMainFrame->EditQuantizeChange(1);
		break;

	case cdefTransposeChannelInc:
		pChildView->patTranspose(1);
		break;
	case cdefTransposeChannelDec:
		pChildView->patTranspose(-1);
		break;
	case cdefTransposeChannelInc12:
		pChildView->patTranspose(12);
		break;
	case cdefTransposeChannelDec12:
		pChildView->patTranspose(-12);
		break;

	case cdefTransposeBlockInc:
		pChildView->BlockTranspose(1);
		break;
	case cdefTransposeBlockDec:
		pChildView->BlockTranspose(-1);
		break;
	case cdefTransposeBlockInc12:
		pChildView->BlockTranspose(12);
		break;
	case cdefTransposeBlockDec12:
		pChildView->BlockTranspose(-12);
		break;


	case cdefBlockUnMark:
		pChildView->BlockUnmark();
		break;

	case cdefBlockDouble:
		pChildView->DoubleLength();
		break;

	case cdefBlockHalve:
		pChildView->HalveLength();
		break;

	case cdefBlockCut:
		pChildView->CopyBlock(true);
		break;

	case cdefBlockCopy:
		pChildView->CopyBlock(false);
		break;

	case cdefBlockPaste:
		pChildView->PasteBlock(pChildView->editcur.track,pChildView->editcur.line,false);
		break;

	case cdefBlockMix:
		pChildView->PasteBlock(pChildView->editcur.track,pChildView->editcur.line,true);
		break;

	case cdefBlockDelete:
		pChildView->DeleteBlock();
		break;

	case cdefBlockInterpolate:
		pChildView->BlockParamInterpolate();
		break;

	case cdefBlockSetMachine:
		pChildView->BlockGenChange(Global::_pSong->seqBus);
		break;

	case cdefBlockSetInstr:
		pChildView->BlockInsChange(Global::_pSong->auxcolSelected);
		break;

	case cdefOctaveUp:
		pMainFrame->ShiftOctave(1);
		break;

	case cdefOctaveDn:
		pMainFrame->ShiftOctave(-1);
		break;

	case cdefPlaySong:
		PlaySong();
		break;

	case cdefPlayFromPos:
		PlayFromCur();
		break;

	case cdefPlayRepeat:
		// TODO
		TRACE("TODO\n");
		PlaySong(); //FIXME
		break;

	case cdefPlayRowTrack:
		pChildView->PlayCurrentNote();
		pChildView->AdvanceLine(1,Global::pConfig->_wrapAround);
		break;

	case cdefPlayRowPattern:
		pChildView->PlayCurrentRow();
		pChildView->AdvanceLine(1,Global::pConfig->_wrapAround);
		break;

	case cdefPlayBlock:
		pChildView->OnButtonplayseqblock();
		break;

	case cdefEditToggle:
		pChildView->bEditMode = !pChildView->bEditMode;
		
		if(bCtrlPlay && GetKeyState(VK_RCONTROL)<0) Stop();
		
//		pChildView->Repaint(DMPatternHeader);
		break;

	case cdefPlayStop:
		Stop();
		break;
	
	case cdefSelectMachine:
		pChildView->SelectMachineUnderCursor();
		break;
	case cdefMachineInc:
		pMainFrame->OnBIncgen();
		break;

	case cdefMachineDec:
		pMainFrame->OnBDecgen();
		break;

	case cdefInstrInc:
		pMainFrame->OnBIncwav();
		break;

	case cdefInstrDec:
		pMainFrame->OnBDecwav();
		break;

	case cdefInfoPattern:
		if ( pChildView->viewMode == VMPattern )
		{
			pChildView->OnPopPattenproperties();
		}
		break;

	case cdefInfoMachine:
		int nmac;
		if (Global::_pSong->seqBus<MAX_BUSES) // Generators
			nmac = Global::_pSong->busMachine[Global::_pSong->seqBus];
		else
			nmac = Global::_pSong->busEffect[(Global::_pSong->seqBus & (MAX_BUSES-1))];

		if (nmac < MAX_MACHINES && Global::_pSong->_machineActive[nmac])
		{	
			CPoint point;
			point.x = Global::_pSong->_pMachines[nmac]->_x;
			point.y = Global::_pSong->_pMachines[nmac]->_y;
			pMainFrame->ShowMachineGui(nmac, point);//, Global::_pSong->seqBus);
		}
		break;

	case cdefEditMachine:
		pChildView->OnMachineview();
		break;

	case cdefEditPattern:
		pChildView->OnPatternView();
		break;

	case cdefEditInstr:
		pMainFrame->ShowInstrumentEditor();
		break;

	case cdefAddMachine:
		pChildView->OnNewmachine();
		break;


	case cdefPatternInc:
		pChildView->IncCurPattern();
		break;

	case cdefPatternDec:
		pChildView->DecCurPattern();
		break;

	case cdefSongPosInc:
		pChildView->IncPosition(brepeat?true:false);
		break;

	case cdefSongPosDec:
		pChildView->DecPosition();
		break;

	case cdefUndo:
		pChildView->bScrollDetatch=false;
		 pChildView->OnEditUndo();
		 break;

	case cdefRedo:
		pChildView->bScrollDetatch=false;
		 pChildView->OnEditRedo();
		 break;
	}
}

void InputHandler::Stop()
{
	Global::pPlayer->Stop();
	pMainFrame->SetAppSongBpm(0);
	pMainFrame->SetAppSongTpb(0);
	pMainFrame->UpdatePlayOrder(false); // <- This restores the selected block
	pChildView->Repaint(DMCursor);  
}

void InputHandler::PlaySong() 
{
	((Master*)(Global::_pSong->_pMachines[0]))->_clip = false;
	Global::pPlayer->Start(pChildView->editPosition,0);
	pMainFrame->StatusBarIdle();
}

void InputHandler::PlayFromCur() 
{
	((Master*)(Global::_pSong->_pMachines[0]))->_clip = false;
	Global::pPlayer->Start(pChildView->editPosition,pChildView->editcur.line);
	pMainFrame->StatusBarIdle();
}


bool InputHandler::EnterData(UINT nChar,UINT nFlags)
{
	if ( pChildView->editcur.col == 0 )
	{
		// get command
		CmdDef cmd = Global::pInputHandler->KeyToCmd(nChar,nFlags);

		BOOL bRepeat = nFlags&0x4000;
		if ( cmd.GetType() == CT_Note )
		{
			if (!bRepeat)
			{
				pChildView->EnterNote(cmd.GetNote());
				return true;
			}
		}
		return false;
	}
	else if ( GetKeyState(VK_CONTROL)>=0 && GetKeyState(VK_SHIFT)>=0 )
	{
		return pChildView->MSBPut(nChar);
	}
	return false;
}

void InputHandler::StopNote(int note, bool bTranspose,Machine*pMachine)
{
	if(note<0)
		return;

	// octave offset 
	if(note<120)
	{
		if(bTranspose)
			note+=Global::_pSong->currentOctave*12;

		if (note > 119) 
			note = 119;
	}

	for(int i=0;i<Global::_pSong->SONGTRACKS;i++)
	{
		if(notetrack[i]==note)
		{
			// build entry
			PatternEntry entry;
			entry._note = 120;
			entry._inst = Global::_pSong->auxcolSelected;
			entry._mach = Global::_pSong->seqBus;
			entry._cmd = 0;
			entry._parameter = 0;	

			// play it
			if(pMachine==NULL)
			{
				int mgn;
				if ( Global::_pSong->seqBus < MAX_BUSES )
					mgn = Global::_pSong->busMachine[Global::_pSong->seqBus];
				else
					mgn = Global::_pSong->busEffect[(Global::_pSong->seqBus & (MAX_BUSES-1))];

				if (mgn < MAX_MACHINES && Global::_pSong->_machineActive[mgn])
					pMachine = Global::_pSong->_pMachines[mgn];
				else return;
			}

			notetrack[i]=-1;
			pMachine->Tick(i,&entry);
			return;
		}
	}
}

// velocity range 0 -> 127
void InputHandler::PlayNote(int note,int velocity,bool bTranspose,Machine*pMachine)
{
	// stop any notes with the same value
	StopNote(note,bTranspose,pMachine);

	if(note<0)
		return;

	// octave offset
	if(note<120)
	{
		if(bTranspose)
			note+=Global::_pSong->currentOctave*12;

		if (note > 119) 
			note = 119;
	}

	// build entry
	PatternEntry entry;
	entry._note = note;
	entry._inst = Global::_pSong->auxcolSelected;
	entry._mach = Global::_pSong->seqBus;	// Not really needed.

	if ((velocity != 127) &&  (Global::pConfig->_midiRecordVel))
	{
		entry._cmd = Global::pConfig->_midiCommandVel;
		int par = Global::pConfig->_midiFromVel + 
			(((Global::pConfig->_midiToVel - Global::pConfig->_midiFromVel) * velocity)/127);
		if (par > 255) 
		{
			par = 255;
		}
		else if (par < 0) 
		{
			par = 0;
		}
		entry._parameter = par;
	}
	else
	{
		entry._cmd=0;
		entry._parameter=0;
	}

	// play it
	if(pMachine==NULL)
	{
		int mgn;
		if ( Global::_pSong->seqBus < MAX_BUSES )
			mgn = Global::_pSong->busMachine[Global::_pSong->seqBus];
		else
			mgn = Global::_pSong->busEffect[(Global::_pSong->seqBus & (MAX_BUSES-1))];

		if (mgn < MAX_MACHINES && Global::_pSong->_machineActive[mgn])
			pMachine = Global::_pSong->_pMachines[mgn];
		else return;
	}	


	// pick a track to play it on	
	if(bMultiKey)
		outtrack++;
	if(outtrack>=Global::_pSong->SONGTRACKS)
		outtrack=0;

	// play
	notetrack[outtrack]=note;
	pMachine->Tick(outtrack,&entry);
}

// configure default keys
// messy, but not really any way around it
void InputHandler::BuildCmdLUT()
{
	// initialize
	CmdDef cmdNull;

	UINT i;
	UINT j;
	for(i=0;i<MOD_MAX;i++)		
		for(j=0;j<256;j++)
			SetCmd(cmdNull,j,i);			

	// immediate commands
	SetCmd(cdefEditMachine,VK_F2,0);
	SetCmd(cdefEditPattern,VK_F3,0);

	SetCmd(cdefInfoMachine,VK_RETURN,MOD_S);
	SetCmd(cdefInfoPattern,VK_RETURN,MOD_C);
	SetCmd(cdefSelectMachine,VK_RETURN,0);		// This is also called in ConfigRestore().
												// Remove it from there when making a new system.
	SetCmd(cdefAddMachine,VK_F9,0);
	SetCmd(cdefEditInstr,VK_F10,0);

	SetCmd(cdefOctaveUp,VK_MULTIPLY,0);
	SetCmd(cdefOctaveDn,VK_DIVIDE,MOD_E);

	SetCmd(cdefMachineDec,VK_LEFT,MOD_C|MOD_E);
	SetCmd(cdefMachineInc,VK_RIGHT,MOD_C|MOD_E);

	SetCmd(cdefInstrDec,VK_DOWN,MOD_C|MOD_E);
	SetCmd(cdefInstrInc,VK_UP,MOD_C|MOD_E);

	SetCmd(cdefPlayRowTrack,'4',0);
	SetCmd(cdefPlayRowPattern,'8',0);
	
	SetCmd(cdefPlayRepeat,17,MOD_S|MOD_E);
	SetCmd(cdefPlaySong,VK_F5,0);
	SetCmd(cdefPlayBlock,VK_F6,0);
	SetCmd(cdefPlayFromPos,VK_F7,0);
	SetCmd(cdefPlayStop,VK_F8,0);

	SetCmd(cdefPatternInc,VK_UP,MOD_S|MOD_E);
	SetCmd(cdefPatternDec,VK_DOWN,MOD_S|MOD_E);
	SetCmd(cdefSongPosInc,VK_RIGHT,MOD_S|MOD_E);
	SetCmd(cdefSongPosDec,VK_LEFT,MOD_S|MOD_E);

	SetCmd(cdefEditToggle,' ',0);

	// editor commands
	SetCmd(cdefColumnNext,VK_TAB,0);
	SetCmd(cdefColumnPrev,VK_TAB,MOD_S);

	SetCmd(cdefNavUp,VK_UP,MOD_E);
	SetCmd(cdefNavDn,VK_DOWN,MOD_E);
	SetCmd(cdefNavLeft,VK_LEFT,MOD_E);
	SetCmd(cdefNavRight,VK_RIGHT,MOD_E);

	SetCmd(cdefNavPageUp,VK_PRIOR,MOD_E);
	SetCmd(cdefNavPageDn,VK_NEXT,MOD_E);
	SetCmd(cdefNavTop,VK_HOME,MOD_E);
	SetCmd(cdefNavBottom,VK_END,MOD_E);

	SetCmd(cdefTransposeChannelDec,VK_F1,MOD_C);	
	SetCmd(cdefTransposeChannelInc,VK_F2,MOD_C);	
	SetCmd(cdefTransposeChannelDec12,VK_F1,MOD_C|MOD_S);
	SetCmd(cdefTransposeChannelInc12,VK_F2,MOD_C|MOD_S);

	SetCmd(cdefTransposeBlockDec,VK_F11,MOD_C);
	SetCmd(cdefTransposeBlockInc,VK_F12,MOD_C);
	SetCmd(cdefTransposeBlockDec12,VK_F11,MOD_C|MOD_S);
	SetCmd(cdefTransposeBlockInc12,VK_F12,MOD_C|MOD_S);	

	SetCmd(cdefPatternCut,VK_F3,MOD_C);
	SetCmd(cdefPatternCopy,VK_F4,MOD_C);
	SetCmd(cdefPatternPaste,VK_F5,MOD_C);
	SetCmd(cdefPatternMixPaste,VK_F5,MOD_C|MOD_S);
	SetCmd(cdefPatternTrackMute,VK_F9,MOD_C);
	SetCmd(cdefPatternDelete,VK_F3,MOD_C|MOD_S);

	SetCmd(cdefRowInsert,VK_INSERT,MOD_E);
	SetCmd(cdefRowDelete,VK_BACK,0);
	SetCmd(cdefRowClear,VK_DELETE,MOD_E);

	SetCmd(cdefBlockStart,'B',MOD_C);
	SetCmd(cdefBlockEnd,'E',MOD_C);
	SetCmd(cdefBlockUnMark,'U',MOD_C);
	SetCmd(cdefBlockDouble,'D',MOD_C);
	SetCmd(cdefBlockHalve,'H',MOD_C);
	SetCmd(cdefBlockCut,'X',MOD_C);
	SetCmd(cdefBlockCopy,'C',MOD_C);
	SetCmd(cdefBlockPaste,'V',MOD_C);
	SetCmd(cdefBlockMix,'M',MOD_C);
	SetCmd(cdefBlockInterpolate,'I',MOD_C);
	SetCmd(cdefBlockSetMachine,'G',MOD_C);
	SetCmd(cdefBlockSetInstr,'T',MOD_C);
	SetCmd(cdefBlockDelete,'X',MOD_C|MOD_S);

	SetCmd(cdefSelectAll,'A',MOD_C);
	SetCmd(cdefSelectCol,'R',MOD_C);

	SetCmd(cdefEditQuantizeInc,221,0);    // lineskip + 1
	SetCmd(cdefEditQuantizeDec,219,0);    // lineskip - 1

	// note keys

	// octave 0
	SetCmd(cdefKeyC_0,'Z',0);
	SetCmd(cdefKeyCS0,'S',0);
	SetCmd(cdefKeyD_0,'X',0);
	SetCmd(cdefKeyDS0,'D',0);
	SetCmd(cdefKeyE_0,'C',0);
	SetCmd(cdefKeyF_0,'V',0);
	SetCmd(cdefKeyFS0,'G',0);
	SetCmd(cdefKeyG_0,'B',0);
	SetCmd(cdefKeyGS0,'H',0);
	SetCmd(cdefKeyA_1,'N',0);
	SetCmd(cdefKeyAS1,'J',0);
	SetCmd(cdefKeyB_1,'M',0);

	// octave 1
	SetCmd(cdefKeyC_1,'Q',0);
	SetCmd(cdefKeyCS1,'2',0);
	SetCmd(cdefKeyD_1,'W',0);
	SetCmd(cdefKeyDS1,'3',0);
	SetCmd(cdefKeyE_1,'E',0);
	SetCmd(cdefKeyF_1,'R',0);
	SetCmd(cdefKeyFS1,'5',0);
	SetCmd(cdefKeyG_1,'T',0);
	SetCmd(cdefKeyGS1,'6',0);
	SetCmd(cdefKeyA_2,'Y',0);
	SetCmd(cdefKeyAS2,'7',0);
	SetCmd(cdefKeyB_2,'U',0);

	// octave 2
	SetCmd(cdefKeyC_2,'I',0);
	SetCmd(cdefKeyCS2,'9',0);
	SetCmd(cdefKeyD_2,'O',0);
	SetCmd(cdefKeyDS2,'0',0);
	SetCmd(cdefKeyE_2,'P',0);

	// special
	SetCmd(cdefKeyStop,'1',0);
	SetCmd(cdefKeyStopAny,'1',MOD_C);
	SetCmd(cdefTweakM,192,0);        // tweak machine (`)
	SetCmd(cdefMIDICC,192,MOD_S);    // Previously Tweak Effect. Now Mcm Command (~)

	/*
	outnoteLUT[188]	= 12;	// , = C 
	outnoteLUT['L']	= 13;	// C#
	outnoteLUT[190]	= 14;	// . = D
	outnoteLUT[186]	= 15;	// / = D#
	outnoteLUT[191]	= 16;	// E


	outnoteLUT['I']	= 24;	// C
	outnoteLUT['9']	= 25;	// C#
	outnoteLUT['O']	= 26;	// D
	outnoteLUT['0']	= 27;	// D#
	outnoteLUT['P']	= 28;	// E
	*/

	// undo/redo
	SetCmd(cdefUndo,'Z',MOD_C);
	SetCmd(cdefRedo,'Z',MOD_C|MOD_S);

}




