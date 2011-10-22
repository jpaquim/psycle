///\file
///\brief implementation file for psycle::host::InputHandler.
#include <psycle/host/detail/project.private.hpp>
#include "InputHandler.hpp"

#include "MainFrm.hpp"
#include "ChildView.hpp"

#include "Machine.hpp"
#include "Player.hpp"

namespace psycle
{
	namespace host
	{
		SPatternUndo::SPatternUndo()
			:pData(NULL)
			,dataSize(0)
		{}

		SPatternUndo::SPatternUndo(const SPatternUndo& other)
		{
			Copy(other);
		}
		SPatternUndo& SPatternUndo::operator=(const SPatternUndo &other)
		{
			if(pData != NULL) delete pData;
			Copy(other);
			return *this;
		}
		void SPatternUndo::Copy(const SPatternUndo &other)
		{
			if(other.dataSize > 0) {
				pData = new unsigned char[other.dataSize];
				memmove(pData,other.pData,other.dataSize);
				dataSize = other.dataSize;
			}
			else {
				pData = NULL;
				dataSize = 0;
			}
			type = other.type;
			pattern = other.pattern;
			x = other.x;
			y = other.y;
			tracks = other.tracks;
			lines = other.lines;
			edittrack = other.edittrack;
			editline = other.editline;
			editcol = other.editcol;
			seqpos = other.seqpos;
			counter = other.counter;
		}
		SPatternUndo::~SPatternUndo()
		{
			if(pData) delete pData;
		}

		InputHandler::InputHandler()
			:UndoCounter(0)
			,UndoSaved(0)
			,UndoMacCounter(0)
			,UndoMacSaved(0)
		{
			bDoingSelection = false;

			// set up multi-channel playback
			for(UINT i=0;i<MAX_TRACKS;i++) {
				notetrack[i]=notecommands::release;
				instrtrack[i]=255;
				mactrack[i]=255;
			}
			outtrack=0;
		}

		InputHandler::~InputHandler()
		{
			KillRedo();
			KillUndo();
		}


		// KeyToCmd
		// IN: key + modifiers from OnKeyDown
		// OUT: command mapped to key
		CmdDef InputHandler::KeyToCmd(UINT nChar, UINT nFlags)
		{
			PsycleConfig::InputHandler& settings = Global::psycleconf().inputHandler();
			PsycleConfig& config = Global::psycleconf();
			bDoingSelection=false;
			// invalid?	
			if(nChar>255)
			{
				CmdDef cmdNull;
				return cmdNull;
			}
			TRACE("Key nChar : %u pressed. Flags %u\n",nChar,nFlags);

			// special: right control mapped to PLAY
			if(settings.bCtrlPlay && GetKeyState(VK_RCONTROL)<0)
			{
				CmdDef cmdPlay(cdefPlaySong);
				return cmdPlay;
			}
			else
			{
				if (settings.bShiftArrowsDoSelect && GetKeyState(VK_SHIFT)<0 
					&& !(Global::pPlayer->_playing && config._followSong))
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
				std::pair<int, int> pair(GetModifierIdx(nFlags), nChar);
				CmdDef& thisCmd = settings.keyMap[pair];

				if ( thisCmd.GetType() == CT_Note ) {
					return thisCmd;
				} else if ( thisCmd.GetID() == cdefNull) {
					std::pair<int, int> pair2(GetModifierIdx(nFlags) & ~MOD_S, nChar);
					thisCmd = settings.keyMap[pair2];
					return thisCmd;
				} else {
					return thisCmd;
				}
			}
		}	

		// StringToCmd
		// IN: command name (string)
		// OUT: command
		CmdDef InputHandler::StringToCmd(LPCTSTR str)
		{
			PsycleConfig::InputHandler& settings = Global::psycleconf().inputHandler();
			std::map<CmdSet,std::pair<int,int>>::const_iterator it;
			for(it = settings.setMap.begin(); it != settings.setMap.end(); it++)
			{
				CmdDef ret(it->first);
				if(!strcmp(ret.GetName(),str))
					return ret;
			}
			CmdDef ret(cdefNull);
			return ret;
		}

		// CmdToKey
		// IN: command def
		// OUT: key/mod command is defined for, 0/0 if not defined
		void InputHandler::CmdToKey(CmdSet cmd,WORD & key,WORD &mods)
		{
			key = 0;
			mods = 0;
			PsycleConfig::InputHandler& settings = Global::psycleconf().inputHandler();
			std::map<CmdSet,std::pair<int,int>>::const_iterator it;
			it = settings.setMap.find(cmd);
			if (it != settings.setMap.end())
			{
				key = it->second.second;

				if(it->second.first & MOD_S)
					mods|=HOTKEYF_SHIFT;
				if(it->second.first & MOD_C)
					mods|=HOTKEYF_CONTROL;				
				if(it->second.first & MOD_E)
					mods|=HOTKEYF_EXT;
			}
		}


		// operations

		// perform command
		///\todo move to a callback system... this is disgustingly messy
		void InputHandler::PerformCmd(CmdDef &cmd, BOOL brepeat)
		{
			PsycleConfig::InputHandler& settings = Global::psycleconf().inputHandler();
			PsycleConfig::PatternView& patSettings = Global::psycleconf().patView();
			PsycleConfig& config = Global::psycleconf();
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
				pChildView->patMixPaste();
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
				pMainFrame->ToggleFollowSong();
				break;

			case cdefKeyStopAny:
				pChildView->EnterNoteoffAny();
				break;

			case cdefColumnNext:
				pChildView->bScrollDetatch=false;
				pChildView->ChordModeOffs = 0;
				pChildView->AdvanceTrack(1,settings._wrapAround);
				break;

			case cdefColumnPrev:
				pChildView->bScrollDetatch=false;
				pChildView->ChordModeOffs = 0;
				pChildView->PrevTrack(1,settings._wrapAround);
				break;

			case cdefNavLeft:
				pChildView->bScrollDetatch=false;
				pChildView->ChordModeOffs = 0;
				if ( !bDoingSelection )
				{
					pChildView->PrevCol(settings._wrapAround);
					if ( settings.bShiftArrowsDoSelect && settings._windowsBlocks) pChildView->BlockUnmark();
				}
				else
				{
					if ( !pChildView->blockSelected )
					{
						pChildView->StartBlock(pChildView->editcur.track,pChildView->editcur.line,pChildView->editcur.col);
					}
					pChildView->PrevTrack(1,settings._wrapAround);
					pChildView->ChangeBlock(pChildView->editcur.track,pChildView->editcur.line,pChildView->editcur.col);
				}

				bDoingSelection = false;
				break;
			case cdefNavRight:
				pChildView->bScrollDetatch=false;
				pChildView->ChordModeOffs = 0;
				if ( !bDoingSelection )
				{
					pChildView->NextCol(settings._wrapAround);
					if ( settings.bShiftArrowsDoSelect && settings._windowsBlocks) pChildView->BlockUnmark();
				}
				else
				{
					if ( !pChildView->blockSelected)
					{
						pChildView->StartBlock(pChildView->editcur.track,pChildView->editcur.line,pChildView->editcur.col);
					}
					pChildView->AdvanceTrack(1,settings._wrapAround);
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
					pChildView->PrevLine(1,settings._wrapAround);
				else
					//if added by sampler. New option.
					if (!settings._NavigationIgnoresStep)
						pChildView->PrevLine(pChildView->patStep,settings._wrapAround);//before
					else
						pChildView->PrevLine(1,settings._wrapAround);//new option
				if ( bDoingSelection )
				{
					pChildView->ChangeBlock(pChildView->editcur.track,pChildView->editcur.line,pChildView->editcur.col);
				}
				else if ( settings.bShiftArrowsDoSelect && settings._windowsBlocks) pChildView->BlockUnmark();
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
					pChildView->AdvanceLine(1,settings._wrapAround);
				else
					//if added by sampler. New option.
					if (!settings._NavigationIgnoresStep)
						pChildView->AdvanceLine(pChildView->patStep,settings._wrapAround); //before
					else
						pChildView->AdvanceLine(1,settings._wrapAround);//new option
				if ( bDoingSelection )
				{
					pChildView->ChangeBlock(pChildView->editcur.track,pChildView->editcur.line,pChildView->editcur.col);
				}
				else if ( settings.bShiftArrowsDoSelect && settings._windowsBlocks) pChildView->BlockUnmark();
				bDoingSelection = false;
				break;
			case cdefNavPageUp:
				{
					int stepsize(0);
					if ( settings._pageUpSteps == 0) stepsize = Global::_pSong->LinesPerBeat();
					else if ( settings._pageUpSteps == 1)stepsize = Global::_pSong->LinesPerBeat()*patSettings.timesig;
					else stepsize = settings._pageUpSteps;

					//if added by sampler to move backward 16 lines when playing
					if (Global::pPlayer->_playing && config._followSong)
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
						else if ( settings.bShiftArrowsDoSelect && settings._windowsBlocks) pChildView->BlockUnmark();
					}
				}
				break;

			case cdefNavPageDn:
				{
					int stepsize(0);
					if ( settings._pageUpSteps == 0) stepsize = Global::_pSong->LinesPerBeat();
					else if ( settings._pageUpSteps == 1)stepsize = Global::_pSong->LinesPerBeat()*patSettings.timesig;
					else stepsize = settings._pageUpSteps;

					//if added by sampler
					if (Global::pPlayer->_playing && config._followSong)
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
						else if ( settings.bShiftArrowsDoSelect && settings._windowsBlocks) pChildView->BlockUnmark();
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
				if(settings.bFT2HomeBehaviour)
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
				else if ( settings.bShiftArrowsDoSelect && settings._windowsBlocks) pChildView->BlockUnmark();

				pChildView->Repaint(draw_modes::cursor);
				break;
			
			case cdefNavBottom:
				pChildView->bScrollDetatch=false;
				pChildView->ChordModeOffs = 0;
				if ( bDoingSelection && !pChildView->blockSelected)
				{
					pChildView->StartBlock(pChildView->editcur.track,pChildView->editcur.line,pChildView->editcur.col);
				}
				if(settings.bFT2HomeBehaviour)
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
				else if ( settings.bShiftArrowsDoSelect && settings._windowsBlocks) pChildView->BlockUnmark();


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
				if (pChildView->blockSelected && settings._windowsBlocks)
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
				if (pChildView->blockSelected && settings._windowsBlocks)
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
			//selects 1 bar, 2 bars, 4 bars... up to number of lines in pattern
				{
					const int nl = Global::_pSong->patternLines[Global::_pSong->playOrder[pChildView->editPosition]];			
								
					pChildView->bScrollDetatch=false;
					pChildView->ChordModeOffs = 0;
					
					if (pChildView->blockSelectBarState == 1) 
					{
						pChildView->StartBlock(pChildView->editcur.track,pChildView->editcur.line,pChildView->editcur.col);
					}

					int blockLength = (patSettings.timesig * pChildView->blockSelectBarState * Global::_pSong->LinesPerBeat())-1;

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
				pChildView->AdvanceLine(1,settings._wrapAround);
				break;

			case cdefPlayRowPattern:
				pChildView->PlayCurrentRow();
				pChildView->AdvanceLine(1,settings._wrapAround);
				break;

			case cdefPlayBlock:
				pChildView->OnButtonplayseqblock();
				break;

			case cdefEditToggle:
				pChildView->bEditMode = !pChildView->bEditMode;
				pChildView->ChordModeOffs = 0;
				
				if(settings.bCtrlPlay) Stop();
				
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
					pChildView->ShowPatternDlg();
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
					pMainFrame->ShowControlBar(&pMainFrame->m_seqBar,TRUE,FALSE);
					pMainFrame->ShowControlBar(&pMainFrame->m_songBar,TRUE,FALSE);
					pMainFrame->ShowControlBar(&pMainFrame->m_wndToolBar,TRUE,FALSE);
				} 
				else
				{			
					pChildView->maxView = true;
					pMainFrame->ShowControlBar(&pMainFrame->m_seqBar,FALSE,FALSE);
					pMainFrame->ShowControlBar(&pMainFrame->m_songBar,FALSE,FALSE);
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

		void InputHandler::StopNote(int note, int instr, bool bTranspose,Machine*pMachine)
		{
			assert(note>=0 && note < 128);

			int instNo;
			if (instr < 255) instNo = instr;
			else instNo = Global::_pSong->auxcolSelected;

			// octave offset 
			if(note<notecommands::release)
			{
				if(bTranspose)
					note+=Global::_pSong->currentOctave*12;

				if (note > notecommands::b9) 
					note = notecommands::b9;
			}

			if(pMachine==NULL)
			{
				int mgn = Global::_pSong->seqBus;

				if (mgn < MAX_MACHINES)
				{
					pMachine = Global::_pSong->_pMachine[mgn];
				}
				if(!pMachine) return;
			}

			// build entry
			PatternEntry entry;
			entry._note = 120;
			entry._mach = pMachine->_macIndex;
			entry._cmd = 0;
			entry._parameter = 0;	

			for(int i=0;i<Global::_pSong->SONGTRACKS;i++)
			{
				if(notetrack[i]==note && instrtrack[i]==instNo)
				{
					notetrack[i]=notecommands::release;
					instrtrack[i]=255;
					entry._inst = instNo;

					// play it
					pMachine->Tick(i,&entry);
				}
			}
		}

		// velocity range 0 -> 127
		void InputHandler::PlayNote(int note,int instr, int velocity,bool bTranspose,Machine*pMachine)
		{
			PsycleConfig& config = Global::psycleconf();

			// stop any (stuck) notes with the same value
			StopNote(note,instr,bTranspose,pMachine);

			int instNo;
			if (instr < 255) instNo = instr;
			else instNo = Global::_pSong->auxcolSelected;

			if(note<0)
				return;

			// octave offset
			if(note<120)
			{
				if(bTranspose)
					note+=Global::_pSong->currentOctave*12;

				if (note > notecommands::b9) 
					note = notecommands::b9;
			}

			// build entry
			PatternEntry entry;
			entry._note = note;
			entry._inst = instNo;
			entry._mach = Global::_pSong->seqBus;

			if(velocity != 127 && config.midi().velocity().record())
			{
				int par = config.midi().velocity().from() + (config.midi().velocity().to() - config.midi().velocity().from()) * velocity / 127;
				if (par > 255) par = 255; else if (par < 0) par = 0;
				entry._cmd = config.midi().velocity().command();
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
				if (entry._mach < MAX_MACHINES)
				{
					pMachine = Global::_pSong->_pMachine[entry._mach];
				}
			}	

			if (pMachine)
			{
				// if the current machine is a sampler, check 
				// if current sample is locked to a machine.
				// if so, switch entry._mach to that machine number
				if (pMachine->_type == MACH_SAMPLER)
				{
					if ((Global::_pSong->_pInstrument[instNo]->_lock_instrument_to_machine != -1)
						&& (Global::_pSong->_pInstrument[instNo]->_LOCKINST == true))
					{
						entry._mach = Global::_pSong->_pInstrument[instNo]->_lock_instrument_to_machine;
						pMachine = Global::_pSong->_pMachine[entry._mach];
						if ( !pMachine) return;
					}
				}
				// pick a track to play it on	
				if(config.inputHandler().bMultiKey)
				{
					int i;
					for (i = outtrack+1; i < Global::_pSong->SONGTRACKS; i++)
					{
						if (notetrack[i] == notecommands::release)
						{
							break;
						}
					}
					if (i >= Global::_pSong->SONGTRACKS)
					{
						for (i = 0; i <= outtrack; i++)
						{
							if (notetrack[i] == notecommands::release)
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
				if (notetrack[outtrack] < notecommands::release)
				{
					StopNote(notetrack[outtrack], instrtrack[outtrack], bTranspose, pMachine);
				}

				// play
				notetrack[outtrack]=note;
				instrtrack[outtrack]=instNo;
				pMachine->Tick(outtrack,&entry);
			}
		}



		bool InputHandler::EnterData(UINT nChar,UINT nFlags)
		{
			if ( pChildView->editcur.col == 0 )
			{
				// get command
				CmdDef cmd = KeyToCmd(nChar,nFlags);

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


		///////////////////////////////////////////////////////////////////////////////////////////////////
		// MidiPatternNote
		//
		// DESCRIPTION	  : Called by the MIDI input interface to insert pattern notes
		// PARAMETERS     : int outnote - note to insert/stop . int velocity - velocity of the note, or zero if noteoff
		// RETURNS		  : <void>
		// 
		void InputHandler::MidiPatternNote(int outnote , int macidx, int channel, int velocity)
		{
			PsycleConfig::InputHandler& settings = Global::psycleconf().inputHandler();
			PsycleConfig& config = Global::psycleconf();
			Machine* mac = NULL;
			if(macidx >=0 && macidx <MAX_BUSES)
			{
				mac = Global::_pSong->_pMachine[macidx];
			}
			// undo code not required, enter note handles it
			if(pChildView->viewMode == view_modes::pattern && pChildView->bEditMode)
			{ 
				// add note
				if(velocity > 0 || 
					(settings._RecordNoteoff && Global::pPlayer->_playing && config._followSong))
				{
					pChildView->EnterNote(outnote, channel,velocity,false, mac);
				}
				else
				{
					StopNote(outnote,channel,false, mac);	// note end
				}			
			}
			else 
			{
				// play note
				if(velocity>0)
					PlayNote(outnote,channel,velocity,false,mac);
				else
					StopNote(outnote,channel,false,mac);
			}

		}

		void InputHandler::MidiPatternTweak(int busMachine, int command, int value, bool slide) {
			pChildView->MousePatternTweak(busMachine, command, value, slide);

			Song& song = Global::song();
			// play it
			Machine* pMachine = song._pMachine[busMachine];
			if (pMachine)
			{
				// build entry
				PatternEntry entry;
				entry._mach = busMachine;
				entry._cmd = (value>>8)&255;
				entry._parameter = value&255;
				entry._inst = command;
				entry._note = (slide)?notecommands::tweakslide : notecommands::tweak;
				// play
				pMachine->Tick(pChildView->editcur.track,&entry);
			}
		}

		//These are just redirections right now.
		void InputHandler::MidiPatternCommand(int busMachine, int command, int value){
			pChildView->MidiPatternCommand(busMachine, command, value);
		}

		void InputHandler::MidiPatternMidiCommand(int busMachine, int command, int value){
			pChildView->MidiPatternMidiCommand(busMachine, command, value);
		}

		void InputHandler::Automate(int macIdx, int param, int value, bool undo)
		{
			PsycleConfig::InputHandler& settings = Global::psycleconf().inputHandler();

			if(undo || true) {
				AddMacViewUndo();
			}

			if(settings._RecordTweaks)
			{
				if(settings._RecordMouseTweaksSmooth)
					pChildView->MousePatternTweak(macIdx, param, value,true);
				else
					pChildView->MousePatternTweak(macIdx, param, value );
			}
		}
		////////////////////////////////////////////////////////////////////////////////////////////////////////
		// undo/redo code
		////////////////////////////////////////////////////////////////////////////////////////////////////////

		void InputHandler::AddMacViewUndo()
		{
			// i have not written the undo code yet for machine and instruments
			// however, for now it at least tracks changes for save/new/open/close warnings
			UndoMacCounter++;
			pChildView->SetTitleBarText();
		}

		void InputHandler::AddUndo(int pattern, int x, int y, int tracks, int lines, int edittrack, int editline, int editcol, int seqpos, BOOL bWipeRedo, int counter)
		{
			Song& song = Global::song();
			SPatternUndo pNew;

			// fill data
			pNew.dataSize = tracks*lines*EVENT_SIZE;
			unsigned char* pData = new unsigned char[pNew.dataSize];
			pNew.pData = pData;
			pNew.pattern = pattern;
			pNew.x = x;
			pNew.y = y;
			if (tracks+x > song.SONGTRACKS)
			{
				tracks = song.SONGTRACKS-x;
			}
			pNew.tracks = tracks;
						
			const int nl = song.patternLines[pattern];
			
			if (lines+y > nl)
			{
				lines = nl-y;
			}
			pNew.lines = lines;
			pNew.type = UNDO_PATTERN;
			pNew.edittrack = edittrack;
			pNew.editline = editline;
			pNew.editcol = editcol;
			pNew.seqpos = seqpos;

			for (int t=x;t<x+tracks;t++)
			{
				for (int l=y;l<y+lines;l++)
				{
					unsigned char *offset_source=song._ptrackline(pattern,t,l);
					
					memcpy(pData,offset_source,EVENT_SIZE);
					pData+=EVENT_SIZE;
				}
			}
			if (bWipeRedo)
			{
				KillRedo();
				UndoCounter++;
				pNew.counter = UndoCounter;
			}
			else
			{
				pNew.counter = counter;
			}
			pUndoList.push_back(pNew);
			if(pUndoList.size() > 100) {
				pUndoList.pop_front();
			}
			pChildView->SetTitleBarText();
		}

		void InputHandler::AddRedo(int pattern, int x, int y, int tracks, int lines, int edittrack, int editline, int editcol, int seqpos, int counter)
		{
			Song& song = Global::song();
			SPatternUndo pNew;
			pNew.dataSize = tracks*lines*EVENT_SIZE;
			unsigned char* pData = new unsigned char[pNew.dataSize];
			pNew.pData = pData;
			pNew.pattern = pattern;
			pNew.x = x;
			pNew.y = y;
			if (tracks+x > song.SONGTRACKS)
			{
				tracks = song.SONGTRACKS-x;
			}
			pNew.tracks = tracks;
			const int nl = song.patternLines[pattern];
			if (lines+y > nl)
			{
				lines = nl-y;
			}
			pNew.tracks = tracks;
			pNew.lines = lines;
			pNew.type = UNDO_PATTERN;
			pNew.edittrack = edittrack;
			pNew.editline = editline;
			pNew.editcol = editcol;
			pNew.seqpos = seqpos;
			pNew.counter = counter;

			for (int t=x;t<x+tracks;t++)
			{
				for (int l=y;l<y+lines;l++)
				{
					unsigned char *offset_source=song._ptrackline(pattern,t,l);
					
					memcpy(pData,offset_source,EVENT_SIZE);
					pData+=EVENT_SIZE;
				}
			}
			pRedoList.push_back(pNew);
			if(pRedoList.size() > 100) {
				pRedoList.pop_front();
			}
		}

		void InputHandler::AddUndoLength(int pattern, int lines, int edittrack, int editline, int editcol, int seqpos, BOOL bWipeRedo, int counter)
		{
			SPatternUndo pNew;
			pNew.pattern = pattern;
			pNew.x = 0;
			pNew.y = 0;
			pNew.tracks = 0;
			pNew.lines = lines;
			pNew.type = UNDO_LENGTH;
			pNew.edittrack = edittrack;
			pNew.editline = editline;
			pNew.editcol = editcol;
			pNew.seqpos = seqpos;

			if (bWipeRedo)
			{
				KillRedo();
				UndoCounter++;
				pNew.counter = UndoCounter;
			}
			else
			{
				pNew.counter = counter;
			}
			pUndoList.push_back(pNew);
			if(pUndoList.size() > 100) {
				pUndoList.pop_front();
			}

			pChildView->SetTitleBarText();
		}

		void InputHandler::AddRedoLength(int pattern, int lines, int edittrack, int editline, int editcol, int seqpos, int counter)
		{
			SPatternUndo pNew;
			pNew.pattern = pattern;
			pNew.x = 0;
			pNew.y = 0;
			pNew.tracks = 0;
			pNew.lines = lines;
			pNew.type = UNDO_LENGTH;
			pNew.edittrack = edittrack;
			pNew.editline = editline;
			pNew.editcol = editcol;
			pNew.seqpos = seqpos;
			pNew.counter = counter;
			pRedoList.push_back(pNew);
			if(pRedoList.size() > 100) {
				pRedoList.pop_front();
			}
		}

		void InputHandler::AddUndoSequence(int lines, int edittrack, int editline, int editcol, int seqpos, BOOL bWipeRedo, int counter)
		{
			SPatternUndo pNew;
			pNew.dataSize = MAX_SONG_POSITIONS;
			pNew.pData = new unsigned char[pNew.dataSize];
			memcpy(pNew.pData, Global::song().playOrder, MAX_SONG_POSITIONS*sizeof(char));
			pNew.pattern = 0;
			pNew.x = 0;
			pNew.y = 0;
			pNew.tracks = 0;
			pNew.lines = lines;
			pNew.type = UNDO_SEQUENCE;
			pNew.edittrack = edittrack;
			pNew.editline = editline;
			pNew.editcol = editcol;
			pNew.seqpos = seqpos;

			if (bWipeRedo)
			{
				KillRedo();
				UndoCounter++;
				pNew.counter = UndoCounter;
			}
			else
			{
				pNew.counter = counter;
			}
			pUndoList.push_back(pNew);
			if(pUndoList.size() > 100) {
				pUndoList.pop_front();
			}

			pChildView->SetTitleBarText();
		}

		void InputHandler::AddUndoSong(int edittrack, int editline, int editcol, int seqpos, BOOL bWipeRedo, int counter)
		{
			Song& song = Global::song();
			SPatternUndo pNew;
			// fill data
			// count used patterns
			unsigned short count = 0;
			for (unsigned short i = 0; i < MAX_PATTERNS; i++)
			{
				if (song.ppPatternData[i])
				{
					count++;
				}
			}
			pNew.dataSize = MAX_SONG_POSITIONS+sizeof(count)+MAX_PATTERNS+count*MULTIPLY2;
			pNew.pData = new unsigned char[pNew.dataSize];
			unsigned char *pWrite=pNew.pData;
			memcpy(pWrite, song.playOrder, MAX_SONG_POSITIONS*sizeof(char));
			pWrite+=MAX_SONG_POSITIONS*sizeof(char);

			memcpy(pWrite, &count, sizeof(count));
			pWrite+=sizeof(count);

			for (unsigned short i = 0; i < MAX_PATTERNS; i++)
			{
				if (song.ppPatternData[i])
				{
					memcpy(pWrite, &i, sizeof(i));
					pWrite+=sizeof(i);
					memcpy(pWrite, song.ppPatternData[i], MULTIPLY2);
					pWrite+=MULTIPLY2;
				}
			}

			pNew.pattern = 0;
			pNew.x = 0;
			pNew.y = 0;
			pNew.tracks = 0;
			pNew.lines = song.playLength;
			pNew.type = UNDO_SONG;
			pNew.edittrack = edittrack;
			pNew.editline = editline;
			pNew.editcol = editcol;
			pNew.seqpos = seqpos;

			if (bWipeRedo)
			{
				KillRedo();
				UndoCounter++;
				pNew.counter = UndoCounter;
			}
			else
			{
				pNew.counter = counter;
			}
			pUndoList.push_back(pNew);
			if(pUndoList.size() > 100) {
				pUndoList.pop_front();
			}
			pChildView->SetTitleBarText();
		}

		void InputHandler::AddRedoSong(int edittrack, int editline, int editcol, int seqpos, int counter)
		{
			Song& song = Global::song();
			SPatternUndo pNew;
			// fill data
			// count used patterns
			unsigned char count = 0;
			for (unsigned short i = 0; i < MAX_PATTERNS; i++)
			{
				if (song.ppPatternData[i])
				{
					count++;
				}
			}
			pNew.dataSize = MAX_SONG_POSITIONS+sizeof(count)+MAX_PATTERNS+count*MULTIPLY2;
			pNew.pData = new unsigned char[pNew.dataSize];
			unsigned char *pWrite=pNew.pData;
			memcpy(pWrite, song.playOrder, MAX_SONG_POSITIONS*sizeof(char));
			pWrite+=MAX_SONG_POSITIONS*sizeof(char);

			memcpy(pWrite, &count, sizeof(count));
			pWrite+=sizeof(count);

			for (unsigned short i = 0; i < MAX_PATTERNS; i++)
			{
				if (song.ppPatternData[i])
				{
					memcpy(pWrite, &i, sizeof(i));
					pWrite+=sizeof(i);
					memcpy(pWrite, song.ppPatternData[i], MULTIPLY2);
					pWrite+=MULTIPLY2;
				}
			}

			pNew.pattern = 0;
			pNew.x = 0;
			pNew.y = 0;
			pNew.tracks = 0;
			pNew.lines = song.playLength;
			pNew.type = UNDO_SONG;
			pNew.edittrack = edittrack;
			pNew.editline = editline;
			pNew.editcol = editcol;
			pNew.seqpos = seqpos;
			pNew.counter = counter;
			pRedoList.push_back(pNew);
			if(pRedoList.size() > 100) {
				pRedoList.pop_front();
			}
		}

		void InputHandler::AddRedoSequence(int lines, int edittrack, int editline, int editcol, int seqpos, int counter)
		{
			Song &song = Global::song();
			SPatternUndo pNew;
			// fill data
			pNew.dataSize = MAX_SONG_POSITIONS;
			pNew.pData = new unsigned char[pNew.dataSize];
			memcpy(pNew.pData, song.playOrder, MAX_SONG_POSITIONS*sizeof(char));
			pNew.pattern = 0;
			pNew.x = 0;
			pNew.y = 0;
			pNew.tracks = 0;
			pNew.lines = lines;
			pNew.type = UNDO_SEQUENCE;
			pNew.edittrack = edittrack;
			pNew.editline = editline;
			pNew.editcol = editcol;
			pNew.seqpos = seqpos;
			pNew.counter = counter;
			pRedoList.push_back(pNew);
			if(pRedoList.size() > 100) {
				pRedoList.pop_front();
			}
		}

		void InputHandler::KillRedo()
		{
			pRedoList.clear();
		}

		void InputHandler::KillUndo()
		{
			pUndoList.clear();
			UndoCounter = 0;
			UndoSaved = 0;

			UndoMacCounter=0;
			UndoMacSaved=0;
		}

		bool InputHandler::IsModified()
		{
			if(pUndoList.empty() && UndoSaved != 0)
			{
				return true;
			}
			if(!pUndoList.empty() && pUndoList.back().counter != UndoSaved)
			{
				return true;
			}
			if (UndoMacSaved != UndoMacCounter)
			{
				return true;
			}
			return false;
		}
		void InputHandler::SafePoint()
		{
			if(!pUndoList.empty())
			{
				UndoSaved = pUndoList.back().counter;
			}
			else
			{
				UndoSaved = 0;
			}
			UndoMacSaved = UndoMacCounter;
			pChildView->SetTitleBarText();
		}
		bool InputHandler::HasRedo(int viewMode)
		{
			if(pRedoList.empty()) {
				return false;
			}
			else {
				SPatternUndo& redo = pRedoList.back();
				switch (redo.type)
				{
				case UNDO_SEQUENCE:
					return true;
					break;
				default:
					if(viewMode == view_modes::pattern)// && bEditMode)
					{
						return true;
					}
					else
					{
						return false;
					}
					break;
				}
			}
		}
		bool InputHandler::HasUndo(int viewMode)
		{
			if(pUndoList.empty()) {
				return false;
			}
			else {
				SPatternUndo& undo = pUndoList.back();
				switch (undo.type)
				{
				case UNDO_SEQUENCE:
					return true;
					break;
				default:
					if(viewMode == view_modes::pattern)// && bEditMode)
					{
						return true;
					}
					else
					{
						return false;
					}
					break;
				}
			}
		}
	}
}
