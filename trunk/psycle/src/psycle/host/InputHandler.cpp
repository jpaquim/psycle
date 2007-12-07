///\file
///\brief implementation file for psycle::host::InputHandler.
#include <psycle/project.private.hpp>
#include "InputHandler.hpp"
#include "version.hpp"
#include "mainfrm.hpp"
#include "machine.hpp"
#include "Player.hpp"
#include "childview.hpp"
#include "Global.hpp"
namespace psycle
{
	namespace host
	{
		const char * sDefaultCfgName = "PsycleKeys.INI";

		InputHandler::InputHandler()
		{
			TRACE("Initializing InputHandler()\n");
			
			// init bools
			bCtrlPlay = true;
			bMultiKey = true;
			bMoveCursorPaste = true;
			bFT2HomeBehaviour = true;
			bFT2DelBehaviour = true;
			bShiftArrowsDoSelect = false;
			bDoingSelection = false;

			// set up multi-channel playback
			for(UINT i=0;i<MAX_TRACKS;i++)
				notetrack[i]=120;
			outtrack=0;

			TRACE("Building default keys");
			// It is important to build the default keys, in order for the commands to exist.
			BuildCmdLUT();
			ConfigRestore();
		}

		InputHandler::~InputHandler()
		{
			ConfigSave();
		}

		// SetCmd
		// in: command def, key, modifiers
		// out: true if we had to remove another definition
		///\todo more warnings if we are changing existing defs
		bool InputHandler::SetCmd(CmdDef &cmd, UINT key, UINT modifiers,bool checkforduplicates)
		{	
			// clear note?
			if(!cmd.IsValid())
			{
				cmdLUT[modifiers][key].SetNull();
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
			bool bCmdReplaced=false;
			if ( checkforduplicates)
			{
				UINT i,j;
				for(j=0;j<MOD_MAX;j++)
				{
					for(i=0;i<256;i++)
					{
						if(cmdLUT[j][i]==cmd && ((j!=modifiers)|| (i!=key)))
						{
							TRACE("--> removing from [%d][%d]\n",j,i);
							cmdLUT[j][i].SetNull();
							bCmdReplaced=true;
						}
					}
				}
			}

			// add new
			cmdLUT[modifiers][key] = cmd;
			return bCmdReplaced;
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
			TRACE("Key nChar : %u pressed. Flags %u\n",nChar,nFlags);

			// special: right control mapped to PLAY
			if(bCtrlPlay && GetKeyState(VK_RCONTROL)<0)
			{
				CmdDef cmdPlay(cdefPlaySong);
				return cmdPlay;
			}
			else
			{
				if (bShiftArrowsDoSelect && GetKeyState(VK_SHIFT)<0 && !(Global::pPlayer->_playing&&Global::pConfig->_followSong))
				{
					switch (nChar)
					{
					case VK_UP:
						{
							CmdDef cmdSel(cdefNavUp);
							bDoingSelection=true; 
							return cmdSel;
						}
						break;
					case VK_LEFT:
						{
							CmdDef cmdSel(cdefNavLeft);
							bDoingSelection=true;
							return cmdSel;
						}
						break;
					case VK_DOWN:
						{
							CmdDef cmdSel(cdefNavDn);
							bDoingSelection=true; 
							return cmdSel; 
						}
						break;
					case VK_RIGHT:
						{
							CmdDef cmdSel(cdefNavRight); 
							bDoingSelection=true; 
							return cmdSel; 
						}
						break;
					case VK_HOME:
						{
							CmdDef cmdSel(cdefNavTop); 
							bDoingSelection=true; 
							return cmdSel; 
						}
						break;
					case VK_END:
						{
							CmdDef cmdSel(cdefNavBottom); 
							bDoingSelection=true; 
							return cmdSel; 
						}
						break;
					case VK_PRIOR:
						{
							CmdDef cmdSel(cdefNavPageUp); 
							bDoingSelection=true; 
							return cmdSel; 
						}
						break;
					case VK_NEXT:
						{
							CmdDef cmdSel(cdefNavPageDn); 
							bDoingSelection=true; 
							return cmdSel; 
						}
						break;
					}
				}

				nFlags= nFlags & ~MK_LBUTTON;
				// This comparison is to allow the "Shift+Note" (chord mode) to work.

				CmdDef thisCmd = cmdLUT[GetModifierIdx(nFlags)][nChar];
				if ( thisCmd.GetType() == CT_Note )
					return thisCmd;

				thisCmd = cmdLUT[(GetModifierIdx(nFlags) & ~MOD_S)][nChar];
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
			int i;
			for(i=0;i<max_cmds;i++)
			{
				CmdDef ret = CmdSet(i);
				if(ret.IsValid())
				{
					if(!strcmp(ret.GetName(),str))
						return ret;
				}
			}
			CmdDef ret(cdefNull);
			return ret;
		}

		// CmdToKey
		// IN: command def
		// OUT: key/mod command is defined for, 0/0 if not defined
		void InputHandler::CmdToKey(CmdDef &cmd,WORD & key,WORD &mods)
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

			// File version
			sect = "Info";
			key = "AppVersion";
			data = PSYCLE__VERSION;
			WritePrivateProfileString(sect,key,data,sDefaultCfgName);
			key = "Description";
			data = "Psycle";
			WritePrivateProfileString(sect,key,data,sDefaultCfgName);	

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

			key = "bMoveCursorPaste";
			data.Format("%d",bMoveCursorPaste);
			WritePrivateProfileString(sect,key,data,sDefaultCfgName);

			key = "bFt2DelBehaviour";
			data.Format("%d",bFT2DelBehaviour);
			WritePrivateProfileString(sect,key,data,sDefaultCfgName);

			key = "bShiftArrowsDoSelect";
			data.Format("%d",bShiftArrowsDoSelect);
			WritePrivateProfileString(sect,key,data,sDefaultCfgName);


			UINT i,j;
			
			// note keys
			sect = "Keys2";
			WritePrivateProfileString(sect,NULL,NULL,sDefaultCfgName); 	// clear

			for(j=0;j<MOD_MAX;j++)
			{
				for(i=0;i<256;i++)
				{
					if(cmdLUT[j][i].IsValid())
					{
						key.Format("n%03d",int(cmdLUT[j][i].GetID()));
						data.Format("%04d   ; cmd = '%s'",(j*256)+i,cmdLUT[j][i].GetName());
						WritePrivateProfileString(sect,key,data,sDefaultCfgName);
					}
				}
			}



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
			{
				return false;
			}
			// option data
			sect = "Options";
			key = "bNewHomeBehaviour"; // Variable renamed to bFT2HomeBehaviour.
			bFT2HomeBehaviour = GetPrivateProfileInt(sect,key,1,sDefaultCfgName)?true:false;

			key = "bCtrlPlay";
			bCtrlPlay = GetPrivateProfileInt(sect,key,1,sDefaultCfgName)?true:false;

			key = "bMultiKey";
			bMultiKey = GetPrivateProfileInt(sect,key,1,sDefaultCfgName)?true:false;

			key = "bMoveCursorPaste";
			bMoveCursorPaste = GetPrivateProfileInt(sect,key,1,sDefaultCfgName)?true:false;

			key = "bFT2DelBehaviour";
			bFT2DelBehaviour = GetPrivateProfileInt(sect,key,1,sDefaultCfgName)?true:false;

			key = "bShiftArrowsDoSelect";
			bShiftArrowsDoSelect = GetPrivateProfileInt(sect,key,1,sDefaultCfgName)?true:false;
			
			if (data== "N/A")
			{
				return ParseOldFileformat(); // 1.7.6 and older
			}
			else
			{
				CString cmdDefn;
				int cmddata, i,modi;
				bool saveconfig(false);
				// restore key data
				sect = "Keys2"; // 1.8 onward
				key.Format("n%03d",0);
				if ( GetPrivateProfileInt(sect,key,-1,sDefaultCfgName) == -1 ) // trying to get key for command 0 (C-0)
				{
					sect = "Keys";  // 1.7 Alpha release.
					saveconfig=true;
				}
				for(i=0;i<max_cmds;i++)
				{
					CmdDef cmd = CmdSet(i);
//					cmdDefn = cmd.GetName();
//					if(cmdDefn!="Invalid")
					if (cmd.IsValid())
					{
						key.Format("n%03d",i);
						cmddata= GetPrivateProfileInt(sect,key,cdefNull,sDefaultCfgName);
						if (cmddata != cdefNull)
						{
							modi=cmddata/256;
							SetCmd(cmd.GetID(),cmddata%256,modi,true);
						}
					}
				}
				if (saveconfig) ConfigSave();
				return true;
			}
		}
		bool InputHandler::ParseOldFileformat()
		{
			// save key data
			UINT i,j;
			CString sect;
			CString key;
			
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
			ConfigSave();
			return true;
		}

















		// operations

		// perform command
		///\todo move to a callback system... this is disgustingly messy
		void InputHandler::PerformCmd(CmdDef &cmd, BOOL brepeat)
		{
			switch(cmd.GetID())
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
				pChildView->bScrollDetatch=false;
				pChildView->ChordModeOffs = 0;
				pChildView->patPaste();
				break;

			case cdefPatternMixPaste:
				pChildView->bScrollDetatch=false;
				pChildView->ChordModeOffs = 0;
				pChildView->patPaste();
				break;

			case cdefPatternDelete:
				pChildView->patDelete();
				break;

			case cdefPatternTrackMute:
				pChildView->patTrackMute();
				break;

			case cdefPatternTrackSolo:
				pChildView->patTrackSolo();
				break;

			case cdefPatternTrackRecord:
				pChildView->patTrackRecord();
				break;

			case cdefFollowSong:	

				//used by song follow toggle
				//CButton*cb=(CButton*)pMainFrame->m_wndSeq.GetDlgItem(IDC_FOLLOW);

				if (((CButton*)pMainFrame->m_wndSeq.GetDlgItem(IDC_FOLLOW))->GetCheck() == 0) 
				{
					((CButton*)pMainFrame->m_wndSeq.GetDlgItem(IDC_FOLLOW))->SetCheck(1);
				} 
				else
				{
					((CButton*)pMainFrame->m_wndSeq.GetDlgItem(IDC_FOLLOW))->SetCheck(0);
				}
				pMainFrame->OnFollowSong();
				break;

			case cdefKeyStopAny:
				pChildView->EnterNoteoffAny();
				break;

			case cdefColumnNext:
				pChildView->bScrollDetatch=false;
				pChildView->ChordModeOffs = 0;
				pChildView->AdvanceTrack(1,Global::pConfig->_wrapAround);
				break;

			case cdefColumnPrev:
				pChildView->bScrollDetatch=false;
				pChildView->ChordModeOffs = 0;
				pChildView->PrevTrack(1,Global::pConfig->_wrapAround);
				break;

			case cdefNavLeft:
				pChildView->bScrollDetatch=false;
				pChildView->ChordModeOffs = 0;
				if ( !bDoingSelection )
				{
					pChildView->PrevCol(Global::pConfig->_wrapAround);
					if ( bShiftArrowsDoSelect && Global::pConfig->_windowsBlocks) pChildView->BlockUnmark();
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
				pChildView->ChordModeOffs = 0;
				if ( !bDoingSelection )
				{
					pChildView->NextCol(Global::pConfig->_wrapAround);
					if ( bShiftArrowsDoSelect && Global::pConfig->_windowsBlocks) pChildView->BlockUnmark();
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
				pChildView->ChordModeOffs = 0;
				if ( bDoingSelection && !pChildView->blockSelected)
				{
					pChildView->StartBlock(pChildView->editcur.track,pChildView->editcur.line,pChildView->editcur.col);
				}
				if (pChildView->patStep == 0)
					pChildView->PrevLine(1,Global::pConfig->_wrapAround);
				else
					//if added by sampler. New option.
					if (!Global::pConfig->_NavigationIgnoresStep)
						pChildView->PrevLine(pChildView->patStep,Global::pConfig->_wrapAround);//before
					else
						pChildView->PrevLine(1,Global::pConfig->_wrapAround);//new option
				if ( bDoingSelection )
				{
					pChildView->ChangeBlock(pChildView->editcur.track,pChildView->editcur.line,pChildView->editcur.col);
				}
				else if ( bShiftArrowsDoSelect && Global::pConfig->_windowsBlocks) pChildView->BlockUnmark();
				bDoingSelection = false;
				break;
			case cdefNavDn:
				pChildView->bScrollDetatch=false;
				pChildView->ChordModeOffs = 0;
				if ( bDoingSelection && !pChildView->blockSelected)
				{
					pChildView->StartBlock(pChildView->editcur.track,pChildView->editcur.line,pChildView->editcur.col);
				}
				if (pChildView->patStep == 0)
					pChildView->AdvanceLine(1,Global::pConfig->_wrapAround);
				else
					//if added by sampler. New option.
					if (!Global::pConfig->_NavigationIgnoresStep)
						pChildView->AdvanceLine(pChildView->patStep,Global::pConfig->_wrapAround); //before
					else
						pChildView->AdvanceLine(1,Global::pConfig->_wrapAround);//new option
				if ( bDoingSelection )
				{
					pChildView->ChangeBlock(pChildView->editcur.track,pChildView->editcur.line,pChildView->editcur.col);
				}
				else if ( bShiftArrowsDoSelect && Global::pConfig->_windowsBlocks) pChildView->BlockUnmark();
				bDoingSelection = false;
				break;
			case cdefNavPageUp:
				{
					int stepsize(0);
					if ( Global::pConfig->_pageUpSteps == 0) stepsize = Global::_pSong->LinesPerBeat();
					else if ( Global::pConfig->_pageUpSteps == 1)stepsize = Global::_pSong->LinesPerBeat()*Global::pConfig->pv_timesig;
					else stepsize = Global::pConfig->_pageUpSteps;

					//if added by sampler to move backward 16 lines when playing
					if (Global::pPlayer->_playing && Global::pConfig->_followSong)
					{
						if (Global::pPlayer->_playBlock )
						{
							if (Global::pPlayer->_lineCounter >= stepsize) Global::pPlayer->_lineCounter -= stepsize;
							else
							{
								Global::pPlayer->_lineCounter = 0;
								Global::pPlayer->ExecuteLine();
							}
						}
						else
						{
							if (Global::pPlayer->_lineCounter >= stepsize) Global::pPlayer->_lineCounter -= stepsize;
							else
							{
								if (Global::pPlayer->_playPosition > 0)
								{
									Global::pPlayer->_playPosition -= 1;
									Global::pPlayer->_lineCounter = Global::_pSong->patternLines[Global::pPlayer->_playPosition] - stepsize;												
								}
								else
								{
									Global::pPlayer->_lineCounter = 0;
									Global::pPlayer->ExecuteLine();
								}
							}
						}
					}
					//end of if added by sampler
					else
					{
						pChildView->bScrollDetatch=false;
						pChildView->ChordModeOffs = 0;

						if ( bDoingSelection && !pChildView->blockSelected)
						{
							pChildView->StartBlock(pChildView->editcur.track,pChildView->editcur.line,pChildView->editcur.col);
						}
						pChildView->PrevLine(stepsize,false);
						if ( bDoingSelection )
						{
							pChildView->ChangeBlock(pChildView->editcur.track,pChildView->editcur.line,pChildView->editcur.col);
						}
						else if ( bShiftArrowsDoSelect && Global::pConfig->_windowsBlocks) pChildView->BlockUnmark();
					}
				}
				break;

			case cdefNavPageDn:
				{
					int stepsize(0);
					if ( Global::pConfig->_pageUpSteps == 0) stepsize = Global::_pSong->LinesPerBeat();
					else if ( Global::pConfig->_pageUpSteps == 1)stepsize = Global::_pSong->LinesPerBeat()*Global::pConfig->pv_timesig;
					else stepsize = Global::pConfig->_pageUpSteps;

					//if added by sampler
					if (Global::pPlayer->_playing && Global::pConfig->_followSong)
					{
						Global::pPlayer->_lineCounter += stepsize;
					}
					//end of if added by sampler
					else
					{
						pChildView->bScrollDetatch=false;
						pChildView->ChordModeOffs = 0;
						if ( bDoingSelection && !pChildView->blockSelected)
						{
							pChildView->StartBlock(pChildView->editcur.track,pChildView->editcur.line,pChildView->editcur.col);
						}
						pChildView->AdvanceLine(stepsize,false);
						if ( bDoingSelection )
						{
							pChildView->ChangeBlock(pChildView->editcur.track,pChildView->editcur.line,pChildView->editcur.col);
						}
						else if ( bShiftArrowsDoSelect && Global::pConfig->_windowsBlocks) pChildView->BlockUnmark();
					}
				}
				break;
			
			case cdefNavTop:
				pChildView->bScrollDetatch=false;
				pChildView->ChordModeOffs = 0;
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
				else if ( bShiftArrowsDoSelect && Global::pConfig->_windowsBlocks) pChildView->BlockUnmark();

				pChildView->Repaint(draw_modes::cursor);
				break;
			
			case cdefNavBottom:
				pChildView->bScrollDetatch=false;
				pChildView->ChordModeOffs = 0;
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
				else if ( bShiftArrowsDoSelect && Global::pConfig->_windowsBlocks) pChildView->BlockUnmark();


				pChildView->Repaint(draw_modes::cursor);
				break;
			
			case cdefRowInsert:
				pChildView->bScrollDetatch=false;
				pChildView->ChordModeOffs = 0;
				pChildView->InsertCurr();
				break;

			case cdefRowDelete:
				pChildView->bScrollDetatch=false;
				pChildView->ChordModeOffs = 0;
				if (pChildView->blockSelected && Global::pConfig->_windowsBlocks)
				{
					pChildView->DeleteBlock();
				}
				else
				{
					pChildView->DeleteCurr();
				}
				break;

			case cdefRowClear:
				pChildView->bScrollDetatch=false;
				pChildView->ChordModeOffs = 0;
				if (pChildView->blockSelected && Global::pConfig->_windowsBlocks)
				{
					pChildView->DeleteBlock();
				}
				else
				{
					pChildView->ClearCurr();		
				}
				break;

			case cdefBlockStart:
				pChildView->bScrollDetatch=false;
				pChildView->ChordModeOffs = 0;
				pChildView->StartBlock(pChildView->editcur.track,pChildView->editcur.line,pChildView->editcur.col);
				break;

			case cdefBlockEnd:
				pChildView->bScrollDetatch=false;
				pChildView->ChordModeOffs = 0;
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

			case cdefSelectBar:
			//selects 4*tpb lines, 8*tpb lines 16*tpb lines, etc. up to number of lines in pattern
				{
					const int nl = Global::_pSong->patternLines[Global::_pSong->playOrder[pChildView->editPosition]];			
								
					pChildView->bScrollDetatch=false;
					pChildView->ChordModeOffs = 0;
					
					if (pChildView->blockSelectBarState == 1) 
					{
						pChildView->StartBlock(pChildView->editcur.track,pChildView->editcur.line,pChildView->editcur.col);
					}

					int blockLength = (4 * pChildView->blockSelectBarState * Global::_pSong->LinesPerBeat())-1;

					if ((pChildView->editcur.line + blockLength) >= nl-1)
					{
						pChildView->EndBlock(pChildView->editcur.track,nl-1,8);	
						pChildView->blockSelectBarState = 1;
					}
					else
					{
						pChildView->EndBlock(pChildView->editcur.track,pChildView->editcur.line + blockLength,8);
						pChildView->blockSelectBarState *= 2;
					}	
					
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
				pChildView->bScrollDetatch=false;
				pChildView->ChordModeOffs = 0;
				pChildView->DoubleLength();
				break;

			case cdefBlockHalve:
				pChildView->bScrollDetatch=false;
				pChildView->ChordModeOffs = 0;
				pChildView->HalveLength();
				break;

			case cdefBlockCut:
				pChildView->CopyBlock(true);
				break;

			case cdefBlockCopy:
				pChildView->CopyBlock(false);
				break;

			case cdefBlockPaste:
				pChildView->bScrollDetatch=false;
				pChildView->ChordModeOffs = 0;
				pChildView->PasteBlock(pChildView->editcur.track,pChildView->editcur.line,false);
				break;

			case cdefBlockMix:
				pChildView->bScrollDetatch=false;
				pChildView->ChordModeOffs = 0;
				pChildView->PasteBlock(pChildView->editcur.track,pChildView->editcur.line,true);
				break;

			case cdefBlockDelete:
				pChildView->bScrollDetatch=false;
				pChildView->ChordModeOffs = 0;
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

			case cdefPlayStart:
				pChildView->OnBarplayFromStart();
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
				pChildView->ChordModeOffs = 0;
				
				if(bCtrlPlay) Stop();
				
		//		pChildView->Repaint(draw_modes::patternHeader);
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
				if ( pChildView->viewMode == view_modes::pattern )
				{
					pChildView->OnPopPattenproperties();
				}
				break;

			case cdefInfoMachine:

				if (Global::_pSong->seqBus < MAX_MACHINES)
				{
					if (Global::_pSong->_pMachine[Global::_pSong->seqBus])
					{
						CPoint point;
						point.x = Global::_pSong->_pMachine[Global::_pSong->seqBus]->_x;
						point.y = Global::_pSong->_pMachine[Global::_pSong->seqBus]->_y;
						pMainFrame->ShowMachineGui(Global::_pSong->seqBus, point);//, Global::_pSong->seqBus);
					}
				}
				break;

			case cdefEditMachine:
				pChildView->OnMachineview();
				break;

			case cdefEditPattern:
				pChildView->OnPatternView();
				pChildView->ChordModeOffs = 0;
				break;

			case cdefEditInstr:
				pMainFrame->ShowInstrumentEditor();
				break;

			case cdefAddMachine:
				pChildView->OnNewmachine();
				break;

			case cdefMaxPattern:		
				if (pChildView->maxView == true) 
				{
					pChildView->maxView = false;
					pMainFrame->ShowControlBar(&pMainFrame->m_wndSeq,TRUE,FALSE);
					pMainFrame->ShowControlBar(&pMainFrame->m_wndControl,TRUE,FALSE);
					pMainFrame->ShowControlBar(&pMainFrame->m_wndToolBar,TRUE,FALSE);
				} 
				else
				{			
					pChildView->maxView = true;
					pMainFrame->ShowControlBar(&pMainFrame->m_wndSeq,FALSE,FALSE);
					pMainFrame->ShowControlBar(&pMainFrame->m_wndControl,FALSE,FALSE);
					pMainFrame->ShowControlBar(&pMainFrame->m_wndToolBar,FALSE,FALSE);
				}
				break;

			case cdefPatternInc:
				pChildView->ChordModeOffs = 0;
				pChildView->IncCurPattern();
				break;

			case cdefPatternDec:
				pChildView->ChordModeOffs = 0;
				pChildView->DecCurPattern();
				break;

			case cdefSongPosInc:
				pChildView->ChordModeOffs = 0;
				pChildView->IncPosition(brepeat?true:false);
				pMainFrame->StatusBarIdle(); 
				break;

			case cdefSongPosDec:
				pChildView->ChordModeOffs = 0;
				pChildView->DecPosition();
				pMainFrame->StatusBarIdle(); 
				break;

			case cdefUndo:
				pChildView->bScrollDetatch=false;
				pChildView->ChordModeOffs = 0;
				pChildView->OnEditUndo();
				break;

			case cdefRedo:
				pChildView->bScrollDetatch=false;
				pChildView->ChordModeOffs = 0;
				pChildView->OnEditRedo();
				break;
			}
		}

		void InputHandler::Stop()
		{
			pChildView->OnBarstop();
		}

		void InputHandler::PlaySong() 
		{
			pChildView->OnBarplay();
		}

		void InputHandler::PlayFromCur() 
		{
			Global::pPlayer->Start(pChildView->editPosition,pChildView->editcur.line);
			pMainFrame->StatusBarIdle();
		}


		bool InputHandler::EnterData(UINT nChar,UINT nFlags)
		{
			if ( pChildView->editcur.col == 0 )
			{
				// get command
				CmdDef cmd = Global::pInputHandler->KeyToCmd(nChar,nFlags);

		//		BOOL bRepeat = nFlags&0x4000;
				if ( cmd.GetType() == CT_Note )
				{
		//			if ((!bRepeat) || (cmd.GetNote() == notecommands::tweak) || (cmd.GetNote() == notecommands::tweakslide) || (cmd.GetNote() == notecommands::midicc))
		//			{
						pChildView->EnterNote(cmd.GetNote());
						return true;
		//			}
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
			assert(note>=0 && note < 128);

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

			if(pMachine==NULL)
			{
				int mgn = Global::_pSong->seqBus;

				if (mgn < MAX_MACHINES)
				{
					pMachine = Global::_pSong->_pMachine[mgn];
				}
			}

			for(int i=0;i<Global::_pSong->SONGTRACKS;i++)
			{
				if(notetrack[i]==note)
				{
					notetrack[i]=120;
					// build entry
					PatternEntry entry;
					entry._note = 120;
					entry._inst = Global::_pSong->auxcolSelected;
					entry._mach = Global::_pSong->seqBus;
					entry._cmd = 0;
					entry._parameter = 0;	

					// play it

					if (pMachine)
					{
						pMachine->Tick(i,&entry);
					}
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

			if(velocity != 127 && Global::pConfig->midi().velocity().record())
			{
				int par = Global::pConfig->midi().velocity().from() + (Global::pConfig->midi().velocity().to() - Global::pConfig->midi().velocity().from()) * velocity / 127;
				if (par > 255) par = 255; else if (par < 0) par = 0;
				switch(Global::pConfig->midi().velocity().type())
				{
					case 0:
						entry._cmd = Global::pConfig->midi().velocity().command();
						entry._parameter = par;
						break;
					case 3:
						entry._inst = par;
						break;
				}
			}
			else
			{
				entry._cmd=0;
				entry._parameter=0;
			}

			// play it
			if(pMachine==NULL)
			{
				if (entry._mach < MAX_MACHINES)
				{
					pMachine = Global::_pSong->_pMachine[entry._mach];
				}
			}	

			if (pMachine)
			{
				// implement lock sample to machine here.
				// if the current machine is a sampler, check 
				// if current sample is locked to a machine.
				// if so, switch entry._mach to that machine number
				if (pMachine->_type == MACH_SAMPLER)
				{
					if ((Global::_pSong->_pInstrument[Global::_pSong->auxcolSelected]->_lock_instrument_to_machine != -1)
						&& (Global::_pSong->_pInstrument[Global::_pSong->auxcolSelected]->_LOCKINST == true))
					{
						entry._mach = Global::_pSong->_pInstrument[Global::_pSong->auxcolSelected]->_lock_instrument_to_machine;
						pMachine = Global::_pSong->_pMachine[entry._mach];
						if ( !pMachine) return;
					}
				}
				// pick a track to play it on	
				if(bMultiKey)
				{
					int i;
					for (i = outtrack+1; i < Global::_pSong->SONGTRACKS; i++)
					{
						if (notetrack[i] == 120)
						{
							break;
						}
					}
					if (i >= Global::_pSong->SONGTRACKS)
					{
						for (i = 0; i <= outtrack; i++)
						{
							if (notetrack[i] == 120)
							{
								break;
							}
						}
					}
					outtrack = i;
				}
				else 
				{
					outtrack=0;
				}
				// this should check to see if a note is playing on that track
				if (notetrack[outtrack] < 120)
				{
					StopNote(notetrack[outtrack], bTranspose, pMachine);
				}

				// play
				notetrack[outtrack]=note;
				pMachine->Tick(outtrack,&entry);
			}
		}

		// configure default keys
		// messy, but not really any way around it
// This is also called in ConfigRestore().
// Remove it from there when/if making a new system.
		void InputHandler::BuildCmdLUT()
		{
			for(int i=0;i<MOD_MAX;i++)		
				for(int j=0;j<256;j++)
					cmdLUT[i][j].SetNull();

			// immediate commands
			SetCmd(cdefEditMachine,VK_F2,0,false);
			SetCmd(cdefEditPattern,VK_F3,0,false);

			SetCmd(cdefInfoMachine,VK_RETURN,MOD_S,false);
			SetCmd(cdefInfoPattern,VK_RETURN,MOD_C,false);
			SetCmd(cdefSelectMachine,VK_RETURN,0,false);		
			SetCmd(cdefAddMachine,VK_F9,0,false);
			SetCmd(cdefEditInstr,VK_F10,0,false);
			SetCmd(cdefMaxPattern,VK_TAB,MOD_C,false);

			SetCmd(cdefOctaveUp,VK_MULTIPLY,0,false);
			SetCmd(cdefOctaveDn,VK_DIVIDE,MOD_E,false);

			SetCmd(cdefMachineDec,VK_LEFT,MOD_C|MOD_E,false);
			SetCmd(cdefMachineInc,VK_RIGHT,MOD_C|MOD_E,false);

			SetCmd(cdefInstrDec,VK_DOWN,MOD_C|MOD_E,false);
			SetCmd(cdefInstrInc,VK_UP,MOD_C|MOD_E,false);

			SetCmd(cdefPlayRowTrack,'4',0,false);
			SetCmd(cdefPlayRowPattern,'8',0,false);
			
			SetCmd(cdefPlayStart,VK_F5,MOD_S,false);
			SetCmd(cdefPlaySong,VK_F5,0,false);
			SetCmd(cdefPlayBlock,VK_F6,0,false);
			SetCmd(cdefPlayFromPos,VK_F7,0,false);
			SetCmd(cdefPlayStop,VK_F8,0,false);

			SetCmd(cdefPatternInc,VK_UP,MOD_S|MOD_E,false);
			SetCmd(cdefPatternDec,VK_DOWN,MOD_S|MOD_E,false);
			SetCmd(cdefSongPosInc,VK_RIGHT,MOD_S|MOD_E,false);
			SetCmd(cdefSongPosDec,VK_LEFT,MOD_S|MOD_E,false);

			SetCmd(cdefEditToggle,' ',0,false);

			// editor commands
			SetCmd(cdefColumnNext,VK_TAB,0,false);
			SetCmd(cdefColumnPrev,VK_TAB,MOD_S,false);

			SetCmd(cdefNavUp,VK_UP,MOD_E,false);
			SetCmd(cdefNavDn,VK_DOWN,MOD_E,false);
			SetCmd(cdefNavLeft,VK_LEFT,MOD_E,false);
			SetCmd(cdefNavRight,VK_RIGHT,MOD_E,false);

			SetCmd(cdefNavPageUp,VK_PRIOR,MOD_E,false);
			SetCmd(cdefNavPageDn,VK_NEXT,MOD_E,false);
			SetCmd(cdefNavTop,VK_HOME,MOD_E,false);
			SetCmd(cdefNavBottom,VK_END,MOD_E,false);

			SetCmd(cdefTransposeChannelDec,VK_F1,MOD_C,false);	
			SetCmd(cdefTransposeChannelInc,VK_F2,MOD_C,false);	
			SetCmd(cdefTransposeChannelDec12,VK_F1,MOD_C|MOD_S,false);
			SetCmd(cdefTransposeChannelInc12,VK_F2,MOD_C|MOD_S,false);

			SetCmd(cdefTransposeBlockDec,VK_F11,MOD_C,false);
			SetCmd(cdefTransposeBlockInc,VK_F12,MOD_C,false);
			SetCmd(cdefTransposeBlockDec12,VK_F11,MOD_C|MOD_S,false);
			SetCmd(cdefTransposeBlockInc12,VK_F12,MOD_C|MOD_S,false);	

			SetCmd(cdefPatternCut,VK_F3,MOD_C,false);
			SetCmd(cdefPatternCopy,VK_F4,MOD_C,false);
			SetCmd(cdefPatternPaste,VK_F5,MOD_C,false);
			SetCmd(cdefPatternMixPaste,VK_F5,MOD_C|MOD_S,false);
			SetCmd(cdefPatternTrackMute,VK_F9,MOD_C,false);
			SetCmd(cdefPatternTrackSolo,VK_F8,MOD_C,false);
			SetCmd(cdefPatternTrackRecord,VK_F7,MOD_C,false);
			SetCmd(cdefFollowSong,'F',MOD_C,false);
			SetCmd(cdefPatternDelete,VK_F3,MOD_C|MOD_S,false);

			SetCmd(cdefRowInsert,VK_INSERT,MOD_E,false);
			SetCmd(cdefRowDelete,VK_BACK,0,false);
			SetCmd(cdefRowClear,VK_DELETE,MOD_E,false);

			SetCmd(cdefBlockStart,'B',MOD_C,false);
			SetCmd(cdefBlockEnd,'E',MOD_C,false);
			SetCmd(cdefBlockUnMark,'U',MOD_C,false);
			SetCmd(cdefBlockDouble,'D',MOD_C,false);
			SetCmd(cdefBlockHalve,'H',MOD_C,false);
			SetCmd(cdefBlockCut,'X',MOD_C,false);
			SetCmd(cdefBlockCopy,'C',MOD_C,false);
			SetCmd(cdefBlockPaste,'V',MOD_C,false);
			SetCmd(cdefBlockMix,'M',MOD_C,false);
			SetCmd(cdefBlockInterpolate,'I',MOD_C,false);
			SetCmd(cdefBlockSetMachine,'G',MOD_C,false);
			SetCmd(cdefBlockSetInstr,'T',MOD_C,false);
			SetCmd(cdefBlockDelete,'X',MOD_C|MOD_S,false);

			SetCmd(cdefSelectAll,'A',MOD_C,false);
			SetCmd(cdefSelectCol,'R',MOD_C,false);
			SetCmd(cdefSelectBar,'K',MOD_C,false);

			SetCmd(cdefEditQuantizeInc,221,0,false);    // lineskip + 1
			SetCmd(cdefEditQuantizeDec,219,0,false);    // lineskip - 1

			// note keys

			// octave 0
			SetCmd(cdefKeyC_0,'Z',0,false);
			SetCmd(cdefKeyCS0,'S',0,false);
			SetCmd(cdefKeyD_0,'X',0,false);
			SetCmd(cdefKeyDS0,'D',0,false);
			SetCmd(cdefKeyE_0,'C',0,false);
			SetCmd(cdefKeyF_0,'V',0,false);
			SetCmd(cdefKeyFS0,'G',0,false);
			SetCmd(cdefKeyG_0,'B',0,false);
			SetCmd(cdefKeyGS0,'H',0,false);
			SetCmd(cdefKeyA_0,'N',0,false);
			SetCmd(cdefKeyAS0,'J',0,false);
			SetCmd(cdefKeyB_0,'M',0,false);

			// octave 1
			SetCmd(cdefKeyC_1,'Q',0,false);
			SetCmd(cdefKeyCS1,'2',0,false);
			SetCmd(cdefKeyD_1,'W',0,false);
			SetCmd(cdefKeyDS1,'3',0,false);
			SetCmd(cdefKeyE_1,'E',0,false);
			SetCmd(cdefKeyF_1,'R',0,false);
			SetCmd(cdefKeyFS1,'5',0,false);
			SetCmd(cdefKeyG_1,'T',0,false);
			SetCmd(cdefKeyGS1,'6',0,false);
			SetCmd(cdefKeyA_1,'Y',0,false);
			SetCmd(cdefKeyAS1,'7',0,false);
			SetCmd(cdefKeyB_1,'U',0,false);

			// octave 2
			SetCmd(cdefKeyC_2,'I',0,false);
			SetCmd(cdefKeyCS2,'9',0,false);
			SetCmd(cdefKeyD_2,'O',0,false);
			SetCmd(cdefKeyDS2,'0',0,false);
			SetCmd(cdefKeyE_2,'P',0,false);

			// special
			SetCmd(cdefKeyStop,'1',0,false);
			SetCmd(cdefKeyStopAny,'1',MOD_C,false);
			SetCmd(cdefTweakM,192,0,false);        // tweak machine (`)
			SetCmd(cdefTweakS,192,MOD_C,false);        // tweak machine (`)
			SetCmd(cdefMIDICC,192,MOD_S,false);    // Previously Tweak Effect. Now Mcm Command (~)

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
			SetCmd(cdefUndo,'Z',MOD_C,false);
			SetCmd(cdefRedo,'Z',MOD_C|MOD_S,false);
		}
	}
}
