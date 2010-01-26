// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2010 members of the psycle project http://psycle.sourceforge.net
///\file
///\brief implementation file for psycle::host::CChildView.

#include "ChildView.hpp"

#include <cderr.h>
#include <cmath>

#include <psycle/core/internal_machines.h>
#include <psycle/core/vstplugin.h>
#include <psycle/core/player.h>
#include <psycle/core/song.h>

//todo: check if these file are really needed.
#include "Configuration.hpp"
#include "MachineView.hpp"
#include "PatternView.hpp"
#include "NativeGui.hpp"
#include "XMSamplerUI.hpp"
#include "MainFrm.hpp"
#include "MidiInput.hpp"
#include "ConfigDlg.hpp"
#include "GreetDialog.hpp"
#include "ProjectData.hpp"
#include "SaveWavDlg.hpp"
#include "SongpDlg.hpp"
#include "XMSongLoader.hpp"
#include "XMSongExport.hpp"
#include "ITModule2.h"
#include "DeleteBlockCommand.hpp"
#include "BlockTransposeCommand.hpp"
#include "PatTransposeCommand.hpp"
#include "PatPasteCommand.hpp"
#include "PatDeleteCommand.hpp"
#include "ChangeGenCommand.hpp"
#include "ChangeInsCommand.hpp"
#if !defined NDEBUG
   #define new DEBUG_NEW
   #undef THIS_FILE
   static char THIS_FILE[] = __FILE__;
#endif

namespace psycle {
	namespace host {

		CChildView::CChildView(CMainFrame* main_frame, ProjectData* projects)
			: main_frame_(main_frame),
			  projects_(projects),
			  SamplerMachineDialog(NULL),
			  XMSamplerMachineDialog(NULL),
			  WaveInMachineDialog(NULL),
			  updateMode(0),
			  updatePar(0),
			  viewMode(view_modes::machine),
			  _outputActive(false),
			  CW(300),
			  CH(200),
			  textLeftEdge(2),
			  bmpDC(NULL),
			  output_driver_(0),
			  last_pos_(-1) {			
			for (int c=0; c<256; c++) { 
				FLATSIZES[c]=8;
			}	
			Global::pInputHandler->SetChildView(this);
			// Creates a new song object. The application Song.
			//projects_->active_project()->song().New();
			// machine_view_->Rebuild();
			// its done in psycle.cpp, todo check config load order
		}

		void CChildView::AddModules(Project* project) {
			machine_view_ = project->mac_view();
			machine_view_->SetParent(this, main_frame_);
			pattern_view_ = project->pat_view();
			pattern_view_->SetParent(this, main_frame_);
		}

		CChildView::~CChildView() {
			Global::pInputHandler->SetChildView(NULL);
			if (bmpDC) {
				char buf[100];
				sprintf(buf,"CChildView::~CChildView(). Deleted bmpDC (was 0x%.8X)\n",(int)bmpDC);
				TRACE(buf);
				bmpDC->DeleteObject();
				delete bmpDC; bmpDC = 0;
			}		
		}

		BEGIN_MESSAGE_MAP(CChildView,CWnd )
			ON_WM_PAINT()
			ON_WM_LBUTTONDOWN()
			ON_WM_RBUTTONDOWN()
			ON_WM_RBUTTONUP()
			ON_WM_LBUTTONUP()
			ON_WM_MOUSEMOVE()
			ON_WM_LBUTTONDBLCLK()
			ON_COMMAND(ID_CPUPERFORMANCE, OnHelpPsycleenviromentinfo)
			ON_COMMAND(ID_MIDI_MONITOR, OnMidiMonitorDlg)
			ON_WM_DESTROY()
			ON_COMMAND(ID_APP_EXIT, OnAppExit)
			ON_COMMAND(ID_MACHINEVIEW, OnMachineview)
			ON_COMMAND(ID_PATTERNVIEW, OnPatternView)	
			ON_WM_KEYDOWN()
			ON_WM_KEYUP()
			ON_COMMAND(ID_BARPLAY, OnBarplay)
			ON_COMMAND(ID_BARPLAYFROMSTART, OnBarplayFromStart)
			ON_COMMAND(ID_BARREC, OnBarrec)
			ON_COMMAND(ID_BARSTOP, OnBarstop)
			ON_COMMAND(ID_RECORDB, OnRecordWav)
			ON_WM_TIMER()
			ON_UPDATE_COMMAND_UI(ID_RECORDB, OnUpdateRecordWav)
			ON_COMMAND(ID_FILE_NEW, OnFileNew)
			ON_COMMAND_EX(ID_EXPORT, OnExport)
			ON_COMMAND_EX(ID_FILE_SAVE, OnFileSave)
			ON_COMMAND_EX(ID_FILE_SAVE_AS, OnFileSaveAs)
			ON_COMMAND(ID_FILE_LOADSONG, OnFileLoadsong)
			ON_COMMAND(ID_FILE_REVERT, OnFileRevert)
			ON_COMMAND(ID_HELP_SALUDOS, OnHelpSaludos)
			ON_COMMAND(ID_CONFIGURATION_SETTINGS, OnConfigurationSettings)
			ON_UPDATE_COMMAND_UI(ID_PATTERNVIEW, OnUpdatePatternView)
			ON_UPDATE_COMMAND_UI(ID_MACHINEVIEW, OnUpdateMachineview)
			ON_UPDATE_COMMAND_UI(ID_BARPLAY, OnUpdateBarplay)
			ON_UPDATE_COMMAND_UI(ID_BARPLAYFROMSTART, OnUpdateBarplayFromStart)
			ON_UPDATE_COMMAND_UI(ID_BARREC, OnUpdateBarrec)
			ON_COMMAND(ID_FILE_SONGPROPERTIES, OnFileSongproperties)
			ON_COMMAND(ID_VIEW_INSTRUMENTEDITOR, OnViewInstrumenteditor)
			ON_COMMAND(ID_NEWMACHINE, OnNewmachine)
			ON_COMMAND(ID_BUTTONPLAYSEQBLOCK, OnButtonplayseqblock)
			ON_UPDATE_COMMAND_UI(ID_BUTTONPLAYSEQBLOCK, OnUpdateButtonplayseqblock)
			ON_COMMAND(ID_POP_CUT, OnPopCut)
			ON_UPDATE_COMMAND_UI(ID_POP_CUT, OnUpdateCutCopy)
			ON_COMMAND(ID_POP_COPY, OnPopCopy)
			ON_COMMAND(ID_POP_PASTE, OnPopPaste)
			ON_UPDATE_COMMAND_UI(ID_POP_PASTE, OnUpdatePaste)
			ON_COMMAND(ID_POP_MIXPASTE, OnPopMixpaste)
			ON_COMMAND(ID_POP_DELETE, OnPopDelete)
			ON_COMMAND(ID_POP_INTERPOLATE, OnPopInterpolate)
			ON_COMMAND(ID_POP_INTERPOLATE_CURVE, OnPopInterpolateCurve)
			ON_COMMAND(ID_POP_CHANGEGENERATOR, OnPopChangegenerator)
			ON_COMMAND(ID_POP_CHANGEINSTRUMENT, OnPopChangeinstrument)
			ON_COMMAND(ID_POP_TRANSPOSE1, OnPopTranspose1)
			ON_COMMAND(ID_POP_TRANSPOSE12, OnPopTranspose12)
			ON_COMMAND(ID_POP_TRANSPOSE_1, OnPopTranspose_1)
			ON_COMMAND(ID_POP_TRANSPOSE_12, OnPopTranspose_12)
			ON_COMMAND(ID_AUTOSTOP, OnAutostop)
			ON_UPDATE_COMMAND_UI(ID_AUTOSTOP, OnUpdateAutostop)
			ON_COMMAND(ID_POP_TRANSFORMPATTERN, OnPopTransformpattern)
			ON_COMMAND(ID_POP_PATTENPROPERTIES, OnPopPattenproperties)
			ON_COMMAND(ID_POP_BLOCK_SWINGFILL, OnPopBlockSwingfill)
			ON_COMMAND(ID_POP_TRACK_SWINGFILL, OnPopTrackSwingfill)
			ON_WM_SIZE()
			ON_COMMAND(ID_CONFIGURATION_SETTINGS, OnConfigurationSettings)
			ON_WM_CONTEXTMENU()
			ON_WM_HSCROLL()
			ON_WM_VSCROLL()
			ON_COMMAND(ID_FILE_IMPORT_XMFILE, OnFileImportModulefile)
			ON_COMMAND(ID_FILE_RECENT_01, OnFileRecent_01)
			ON_COMMAND(ID_FILE_RECENT_02, OnFileRecent_02)
			ON_COMMAND(ID_FILE_RECENT_03, OnFileRecent_03)
			ON_COMMAND(ID_FILE_RECENT_04, OnFileRecent_04)
			ON_COMMAND(ID_EDIT_UNDO, OnEditUndo)
			ON_COMMAND(ID_EDIT_REDO, OnEditRedo)
			ON_UPDATE_COMMAND_UI(ID_EDIT_UNDO, OnUpdateUndo)
			ON_UPDATE_COMMAND_UI(ID_EDIT_REDO, OnUpdateRedo)
			ON_WM_MOUSEWHEEL()
			ON_WM_MBUTTONDOWN()
			ON_UPDATE_COMMAND_UI(ID_EDIT_CUT, OnUpdatePatternCutCopy)
			ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE, OnUpdatePatternPaste)
			ON_COMMAND(ID_FILE_SAVEAUDIO, OnFileSaveaudio)
			ON_COMMAND(ID_HELP_KEYBTXT, OnHelpKeybtxt)
			ON_COMMAND(ID_HELP_README, OnHelpReadme)
			ON_COMMAND(ID_HELP_TWEAKING, OnHelpTweaking)
			ON_COMMAND(ID_HELP_WHATSNEW, OnHelpWhatsnew)
			ON_COMMAND(ID_CONFIGURATION_LOOPPLAYBACK, OnConfigurationLoopplayback)
			ON_UPDATE_COMMAND_UI(ID_CONFIGURATION_LOOPPLAYBACK, OnUpdateConfigurationLoopplayback)
			ON_UPDATE_COMMAND_UI(ID_POP_COPY, OnUpdateCutCopy)
			ON_UPDATE_COMMAND_UI(ID_POP_MIXPASTE, OnUpdatePaste)
			ON_UPDATE_COMMAND_UI(ID_POP_DELETE, OnUpdateCutCopy)
			ON_UPDATE_COMMAND_UI(ID_POP_INTERPOLATE, OnUpdateCutCopy)
			ON_UPDATE_COMMAND_UI(ID_POP_INTERPOLATE_CURVE, OnUpdateCutCopy)
			ON_UPDATE_COMMAND_UI(ID_POP_CHANGEGENERATOR, OnUpdateCutCopy)
			ON_UPDATE_COMMAND_UI(ID_POP_CHANGEINSTRUMENT, OnUpdateCutCopy)
			ON_UPDATE_COMMAND_UI(ID_POP_TRANSPOSE1, OnUpdateCutCopy)
			ON_UPDATE_COMMAND_UI(ID_POP_TRANSPOSE12, OnUpdateCutCopy)
			ON_UPDATE_COMMAND_UI(ID_POP_TRANSPOSE_1, OnUpdateCutCopy)
			ON_UPDATE_COMMAND_UI(ID_POP_TRANSPOSE_12, OnUpdateCutCopy)
			ON_UPDATE_COMMAND_UI(ID_POP_BLOCK_SWINGFILL, OnUpdateCutCopy)
			ON_COMMAND(ID_EDIT_CUT, OnPatCut)
			ON_COMMAND(ID_EDIT_COPY, OnPatCopy)
			ON_COMMAND(ID_EDIT_PASTE, OnPatPaste)
			ON_COMMAND(ID_EDIT_MIXPASTE, OnPatMixPaste)
			ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, OnUpdatePatternCutCopy)
			ON_UPDATE_COMMAND_UI(ID_EDIT_MIXPASTE, OnUpdatePatternPaste)
			ON_COMMAND(ID_EDIT_DELETE, OnPatDelete)
			ON_UPDATE_COMMAND_UI(ID_EDIT_DELETE, OnUpdatePatternCutCopy)
			ON_COMMAND(ID_SHOWPSEQ, OnShowPatternSeq)
			ON_UPDATE_COMMAND_UI(ID_SHOWPSEQ, OnUpdatePatternSeq)
			ON_COMMAND(ID_POP_BLOCKSWITCH, OnPopBlockswitch)
			ON_UPDATE_COMMAND_UI(ID_POP_BLOCKSWITCH, OnUpdatePopBlockswitch)
			END_MESSAGE_MAP()

		BOOL CChildView::PreCreateWindow(CREATESTRUCT& cs) 
		{
			if (!CWnd::PreCreateWindow(cs))
				return FALSE;
			
			cs.dwExStyle |= WS_EX_CLIENTEDGE;
			cs.style &= ~WS_BORDER;
			cs.lpszClass = AfxRegisterWndClass
				(
					CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS,
					//::LoadCursor(NULL, IDC_ARROW), HBRUSH(COLOR_WINDOW+1), NULL);
					::LoadCursor(NULL, IDC_ARROW),
					0,
					0
				);
			return TRUE;
		}		

		/// Timer initialization
		void CChildView::InitTimer()
		{
			KillTimer(31);
			KillTimer(159);
			if (!SetTimer(31,30,NULL)) // GUI update. 
			{
				AfxMessageBox(IDS_COULDNT_INITIALIZE_TIMER, MB_ICONERROR);
			}

			if ( Global::pConfig->autosaveSong )
			{
				if (!SetTimer(159,Global::pConfig->autosaveSongTime*60000,NULL)) // Autosave Song
				{
					AfxMessageBox(IDS_COULDNT_INITIALIZE_TIMER, MB_ICONERROR);
				}
			}
		}

		/// Timer handler
		void CChildView::OnTimer( UINT nIDEvent )
		{
			if (nIDEvent == 31)
			{
				///\todo : IMPORTANT! change this lock to a more flexible one
				// It is causing skips on sound when there is a pattern change because
				// it is not allowing the player to work. Do the same in the one inside Player::Work()
				//todo: we need that lock sincronization
				psycle::core::Song* song = &projects_->active_project()->song();
				if (song->machine(MASTER_INDEX))
				{
					psycle::core::Master* master = (psycle::core::Master*) (song->machine(MASTER_INDEX));
					main_frame_->UpdateVumeters
						(							
							master->_lMax,
							master->_rMax,
							Global::pConfig->vu1,
							Global::pConfig->vu2,
							Global::pConfig->vu3,
							master->_clip
						);
					main_frame_->UpdateMasterValue(master->_outDry);
					//if ( MasterMachineDialog ) MasterMachineDialog->UpdateUI(); maybe a todo
					master->vuupdated = true;
				}

				if (viewMode == view_modes::machine)
				{
					//\todo : Move the commented code to a "Tweak", so we can reuse the code below of "Global::pPlayer->Tweaker"
/*					if (Global::pPlayer->playing() && Global::pPlayer->_lineChanged)
					{
						// This is meant to repaint the whole machine in case the panning/mute/solo/bypass has changed. (not really implemented right now)
						Repaint(draw_modes::all_machines);
					}
					else
					{*/
						CClientDC dc(this);
						DrawAllMachineVumeters(&dc);
//					}
				}

				for(int c=0; c<MAX_MACHINES; c++)
				{
					if (projects_->active_project()->song().machine(c))
					{
						if(projects_->active_project()->song().machine(c)->getMachineKey().host() == Hosts::VST) {
							((vst::plugin*)projects_->active_project()->song().machine(c))->Idle();
						}
					}
				}
				Global::pPlayer->Tweaker = false;

				if (XMSamplerMachineDialog != NULL )
					XMSamplerMachineDialog->UpdateUI();
				if (Global::pPlayer->playing())
				{
					Sequence& sequence = pattern_view()->song()->sequence();
					psycle::core::SequenceEntry* entry = sequence.GetEntryOnPosition(*(sequence.begin()+1), Player::singleton().playPos());
					int pos = (entry) ? (Player::singleton().playPos() - entry->tickPosition())	* static_cast<int>(projects_->active_project()->beat_zoom())
									  : 0;
					if ( last_pos_!= pos ) {
						last_pos_ = pos;
						main_frame_->SetAppSongBpm(0);
						main_frame_->SetAppSongTpb(0);
						if (Global::pConfig->_followSong)
						{											
							if (entry && main_frame_->m_wndSeq.selected_entry() != entry) {
								// change seqview
								pattern_view()->main()->m_wndSeq.SetEntry(entry);
							}
							// this scrolls the pattern view, if follow song is activated
							pattern_view()->editcur.line=pos;
							if ( viewMode == view_modes::pattern )  { 
//								Repaint(draw_modes::pattern);
								Repaint(draw_modes::playback);
							}
						}
//					} else
//                    if (viewMode == view_modes::pattern) {
//						Repaint(draw_modes::playback);
					}

				} else {
#if PSYCLE__CONFIGURATION__USE_PSYCORE
					if (pattern_view()->main()->m_wndSeq.sel_block_play() ) {
						SequencerView& s_view = pattern_view()->main()->m_wndSeq;
						s_view.SwitchToNormalPlay();
					}
#endif
				}
			}
			if (nIDEvent == 159 && !Global::pPlayer->recording())
			{
				//MessageBox("Saving Disabled");
				//return;
				CString filepath = Global::pConfig->GetSongDir().c_str();
				filepath += "\\autosave.psy";
				projects_->active_project()->song().save(filepath.GetBuffer(1),3);
			}
		}

		void CChildView::EnableSound()
		{
#if PSYCLE__CONFIGURATION__USE_PSYCORE			
			psycle::core::Player & player(psycle::core::Player::singleton());
			output_driver_ =  Global::pConfig->_pOutputDriver;
			if (&player.driver() == output_driver_) {
				if (output_driver_->started())
					return;
				output_driver_->set_started(true);
			} else {
				player.setDriver(*output_driver_);
			}
#else
			if (_outputActive)
			{
				AudioDriver* pOut = Global::pConfig->_pOutputDriver;
				if (pOut->Enabled()) return;

				_outputActive = false;
				if (!pOut->Initialized())
				{
					pOut->Initialize(m_hWnd, Global::pPlayer->Work, Global::pPlayer);
				}
				if (!pOut->Configured())
				{
					pOut->Configure();
					Global::pPlayer->SampleRate(pOut->_samplesPerSec);
					_outputActive = true;
				}
				if (pOut->Enable(true))
				{
					_outputActive = true;
				}
				// MIDI IMPLEMENTATION
				Global::pConfig->_pMidiInput->Open();

				// set midi input mode to real-time or step
				if(Global::pConfig->_midiMachineViewSeqMode)
					CMidiInput::Instance()->m_midiMode = MODE_REALTIME;
				else
					CMidiInput::Instance()->m_midiMode = MODE_STEP;

				Global::pPlayer->SampleRate(Global::pConfig->_pOutputDriver->_samplesPerSec);
				Global::pPlayer->SetBPM(projects_->active_project()->song().BeatsPerMin(),projects_->active_project()->song().LinesPerBeat());
			}
#endif
		}


		/// Put exit destroying code here...
		void CChildView::OnDestroy()
		{
			psycle::core::Player & player(psycle::core::Player::singleton());
			player.driver().set_started(false);
			KillTimer(31);
			KillTimer(159);
		}

		void CChildView::OnAppExit() 
		{
			main_frame_->ClosePsycle();
		}

		void CChildView::OnPaint() 
		{		
			if (!GetUpdateRect(NULL) ) return; // If no area to update, exit.	
			CRgn pRgn;
			pRgn.CreateRectRgn(0, 0, 0, 0);
			GetUpdateRgn(&pRgn, FALSE);
			CPaintDC dc(this);		

			if ( bmpDC == NULL && Global::pConfig->useDoubleBuffer ) // buffer creation
			{
				CRect rc;
				GetClientRect(&rc);
				bmpDC = new CBitmap;
				bmpDC->CreateCompatibleBitmap(&dc,rc.right-rc.left,rc.bottom-rc.top);
				char buf[100];
				sprintf(buf,"CChildView::OnPaint(). Initialized bmpDC to 0x%.8X\n",(int)bmpDC);
				TRACE(buf);
			}
			else if ( bmpDC != NULL && !Global::pConfig->useDoubleBuffer ) // buffer deletion
			{
				char buf[100];
				sprintf(buf,"CChildView::OnPaint(). Deleted bmpDC (was 0x%.8X)\n",(int)bmpDC);
				TRACE(buf);
				bmpDC->DeleteObject();
				delete bmpDC; bmpDC = 0;
			}
			if ( Global::pConfig->useDoubleBuffer )
			{
				CDC bufDC;
				bufDC.CreateCompatibleDC(&dc);
				CBitmap* oldbmp;
				oldbmp = bufDC.SelectObject(bmpDC);
				if (viewMode==view_modes::machine)	// Machine view paint handler
				{
					machine_view_->Draw(&bufDC, pRgn); 
				}
				else if (viewMode == view_modes::pattern)	// Pattern view paint handler
				{
					pattern_view_->Draw(&bufDC, pRgn);
				}
				else if ( viewMode == view_modes::sequence)
				{
					// todo create a derived Canvas class and add the Draw method here
				}

				CRect rc;
				GetClientRect(&rc);
				dc.BitBlt(0,0,rc.right-rc.left,rc.bottom-rc.top,&bufDC,0,0,SRCCOPY);
				bufDC.SelectObject(oldbmp);
				bufDC.DeleteDC();
			}
			else
			{
				if (viewMode==view_modes::machine) // Machine view paint handler
				{
					machine_view_->Draw(&dc, pRgn);
				}
				else if (viewMode == view_modes::pattern)	// Pattern view paint handler
				{
					pattern_view_->Draw(&dc, pRgn);
				}
				else if ( viewMode == view_modes::sequence)
				{
					// todo create a derived Canvas class and add the Draw method here
				}
			}
		}

		void CChildView::Repaint(draw_modes::draw_mode drawMode)
		{
			if ( viewMode == view_modes::machine )
			{
				if ( drawMode <= draw_modes::machine )
				{
					updateMode = drawMode;
					Invalidate(false);
				}
			}
			else if ( viewMode == view_modes::pattern )
			{
				if (drawMode >= draw_modes::pattern || drawMode == draw_modes::all )	
				{
					pattern_view()->PreparePatternRefresh(drawMode);
				}
			}
			if ( viewMode == view_modes::sequence )
			{
				Invalidate(false);
			}
		}

		void CChildView::OnSize(UINT nType, int cx, int cy) 
		{
			CWnd ::OnSize(nType, cx, cy);
			machine_view_->OnSize(cx, cy);
			CW = cx;
			CH = cy;
			if (bmpDC != NULL && Global::pConfig->useDoubleBuffer) { // remove old buffer to force recreating it with new size
				TRACE("CChildView::OnResize(). Deleted bmpDC");
				bmpDC->DeleteObject();
				delete bmpDC; bmpDC = 0;
			}
			if (viewMode == view_modes::pattern) {
				pattern_view()->OnSize(nType, cx, cy);				
			}
			Repaint();
		}

		// "Save Song" Function
		BOOL CChildView::OnExport(UINT id)  {
			return projects_->active_project()->Export(id);
		}

		// "Save Song" Function
		BOOL CChildView::OnFileSave(UINT id) 
		{
			return projects_->active_project()->OnFileSave(id);
		}

		// "Save Song As" Function
		BOOL CChildView::OnFileSaveAs(UINT id) 
		{
			return projects_->active_project()->OnFileSaveAs(id);
		}

		void CChildView::OnFileLoadsong()
		{
			OPENFILENAME ofn; // common dialog box structure
			char szFile[_MAX_PATH]; // buffer for file name
			
			szFile[0]='\0';
			// Initialize OPENFILENAME
			ZeroMemory(&ofn, sizeof(OPENFILENAME));
			ofn.lStructSize = sizeof(OPENFILENAME);
			ofn.hwndOwner = GetParent()->m_hWnd;
			ofn.lpstrFile = szFile;
			ofn.nMaxFile = sizeof(szFile);
			ofn.lpstrFilter = "Songs (*.psy)\0*.psy\0Psycle Pattern (*.psb)\0*.psb\0All (*.*)\0*.*\0";
			ofn.nFilterIndex = 1;
			ofn.lpstrFileTitle = NULL;
			ofn.nMaxFileTitle = 0;
			std::string tmpstr = Global::pConfig->GetCurrentSongDir();
			ofn.lpstrInitialDir = tmpstr.c_str();
			ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
			
			// Display the Open dialog box. 
			if(::GetOpenFileName(&ofn)==TRUE)
			{
				projects_->active_project()->OnFileLoadsongNamed(szFile, ofn.nFilterIndex);
			}
			else
			{
				DWORD comDlgErr = CommDlgExtendedError();
				switch(comDlgErr)
				{
				case CDERR_DIALOGFAILURE:
					::MessageBox(0, "CDERR_DIALOGFAILURE", "exception", MB_OK | MB_ICONERROR);
					break;
				case CDERR_FINDRESFAILURE:
					::MessageBox(0, "CDERR_FINDRESFAILURE", "exception", MB_OK | MB_ICONERROR);
					break;
				case CDERR_INITIALIZATION:
					::MessageBox(0, "CDERR_INITIALIZATION", "exception", MB_OK | MB_ICONERROR);
					break;
				case CDERR_LOADRESFAILURE:
					::MessageBox(0, "CDERR_LOADRESFAILURE", "exception", MB_OK | MB_ICONERROR);
					break;
				case CDERR_LOADSTRFAILURE:
					::MessageBox(0, "CDERR_LOADSTRFAILURE", "exception", MB_OK | MB_ICONERROR);
					break;
				case CDERR_LOCKRESFAILURE:
					::MessageBox(0, "CDERR_LOCKRESFAILURE", "exception", MB_OK | MB_ICONERROR);
					break;
				case CDERR_MEMALLOCFAILURE:
					::MessageBox(0, "CDERR_MEMALLOCFAILURE", "exception", MB_OK | MB_ICONERROR);
					break;
				case CDERR_MEMLOCKFAILURE:
					::MessageBox(0, "CDERR_MEMLOCKFAILURE", "exception", MB_OK | MB_ICONERROR);
					break;
				case CDERR_NOHINSTANCE:
					::MessageBox(0, "CDERR_NOHINSTANCE", "exception", MB_OK | MB_ICONERROR);
					break;
				case CDERR_NOHOOK:
					::MessageBox(0, "CDERR_NOHOOK", "exception", MB_OK | MB_ICONERROR);
					break;
				case CDERR_NOTEMPLATE:
					::MessageBox(0, "CDERR_NOTEMPLATE", "exception", MB_OK | MB_ICONERROR);
					break;
				case CDERR_REGISTERMSGFAIL:
					::MessageBox(0, "CDERR_REGISTERMSGFAIL", "exception", MB_OK | MB_ICONERROR);
					break;
				case CDERR_STRUCTSIZE:
					::MessageBox(0, "CDERR_STRUCTSIZE", "exception", MB_OK | MB_ICONERROR);
					break;
				}
			}
			main_frame_->StatusBarIdle();
		}

		void CChildView::OnFileNew() {			
			projects_->FileNew();
		}

		void CChildView::OnFileSaveaudio() 
		{
			OnBarstop();
			KillTimer(31);
			KillTimer(159);
			OnTimer(159); // Autosave
			CSaveWavDlg dlg(this, &pattern_view()->blockSel);
			dlg.DoModal();
			InitTimer();
		}

		void CChildView::OnFileRevert()
		{
			if (MessageBox("Warning! You will lose all changes since song was last saved! Proceed?","Revert to Saved",MB_YESNO | MB_ICONEXCLAMATION) == IDYES)
			{
				if (projects_->active_project()->song()._saved)
				{
					std::ostringstream fullpath;
					fullpath << Global::pConfig->GetCurrentSongDir().c_str()
						<< '\\' << projects_->active_project()->song().fileName.c_str();
					projects_->active_project()->FileLoadsongNamed(fullpath.str());
				}
			}
			main_frame_->StatusBarIdle();
		}

		/// Tool bar buttons and View Commands
		void CChildView::OnMachineview() 
		{
			if (viewMode != view_modes::machine)
			{
				viewMode = view_modes::machine;
				ShowScrollBar(SB_BOTH,FALSE);

				// set midi input mode to real-time or Step
				if(Global::pConfig->_midiMachineViewSeqMode)
					CMidiInput::Instance()->m_midiMode = MODE_REALTIME;
				else
					CMidiInput::Instance()->m_midiMode = MODE_STEP;

				Repaint();
				main_frame_->StatusBarIdle();
			}
			SetFocus();
		}

		void CChildView::OnUpdateMachineview(CCmdUI* pCmdUI) 
		{
			pCmdUI->SetCheck(viewMode==view_modes::machine);
		}

		void CChildView::OnPatternView() 
		{
			if (viewMode != view_modes::pattern)
			{
				pattern_view()->RecalcMetrics();
				viewMode = view_modes::pattern;
				//ShowScrollBar(SB_BOTH,FALSE);
			
				// set midi input mode to step insert
				CMidiInput::Instance()->m_midiMode = MODE_STEP;			
				GetParent()->SetActiveWindow();
#if PSYCLE__CONFIGURATION__USE_PSYCORE
				// skipped for psycore. the idea is that the timer has already updated it.
#else
				if
					(
						Global::pConfig->_followSong &&
						pattern_view()->editPosition  != Global::pPlayer->_sequencePosition &&
						Global::pPlayer->playing()
					)
				{
					pattern_view()->editPosition=Global::pPlayer->_sequencePosition;
				}
#endif
				Repaint();
				main_frame_->StatusBarIdle();
			}
			SetFocus();
		}

		void CChildView::OnUpdatePatternView(CCmdUI* pCmdUI) 
		{			
			pCmdUI->SetCheck(viewMode == view_modes::pattern);		
		}

		void CChildView::OnShowPatternSeq() 
		{
			/*
			if (viewMode != view_modes::sequence)
			{
				viewMode = view_modes::sequence;
				ShowScrollBar(SB_BOTH,FALSE);
				
				// set midi input mode to step insert
				CMidiInput::Instance()->m_midiMode = MODE_STEP;
				
				GetParent()->SetActiveWindow();
				Repaint();
				main_frame_->StatusBarIdle();
			}	
			*/
			SetFocus();
		}

		void CChildView::OnUpdatePatternSeq(CCmdUI* pCmdUI) 
		{
			pCmdUI->SetCheck(viewMode==view_modes::sequence);
		}

		void CChildView::OnBarplay() 
		{
			if (Global::pConfig->_followSong)
			{
				pattern_view()->bScrollDetatch=false;
			}
			pattern_view()->prevEditPosition=pattern_view()->editPosition;
#if PSYCLE__CONFIGURATION__USE_PSYCORE
			psycle::core::Player & player(psycle::core::Player::singleton());			
			psycle::core::SequenceEntry* entry = pattern_view()->main()->m_wndSeq.selected_entry();				    
			player.start(entry->tickPosition());			
#else			
			Global::pPlayer->Start(pattern_view()->editPosition,0);
#endif
			main_frame_->StatusBarIdle();
		}

		void CChildView::OnBarplayFromStart() 
		{
			if (Global::pConfig->_followSong)
			{
				pattern_view()->bScrollDetatch=false;
			}
#if PSYCLE__CONFIGURATION__USE_PSYCORE
			psycle::core::Player & player(psycle::core::Player::singleton());
			player.start(0);		
#else			
			pattern_view()->prevEditPosition=pattern_view()->editPosition;
			Global::pPlayer->Start(0,0);
#endif			
			main_frame_->StatusBarIdle();
		}

		void CChildView::OnUpdateBarplay(CCmdUI* pCmdUI) 
		{
			pCmdUI->SetCheck(Global::pPlayer->playing());		
		}

		void CChildView::OnUpdateBarplayFromStart(CCmdUI* pCmdUI) 
		{
			pCmdUI->SetCheck(0);
		}

		void CChildView::OnBarrec() 
		{
			if (Global::pConfig->_followSong && pattern_view()->bEditMode)
			{
				pattern_view()->bEditMode = FALSE;
			}
			else
			{
				Global::pConfig->_followSong = TRUE;
				pattern_view()->bEditMode = TRUE;
				CButton*cb=(CButton*)main_frame_->m_wndSeq.GetDlgItem(IDC_FOLLOW);
				cb->SetCheck(1);
			}
			main_frame_->StatusBarIdle();
		}

		void CChildView::OnUpdateBarrec(CCmdUI* pCmdUI) 
		{			
			pCmdUI->SetCheck(Global::pConfig->_followSong && pattern_view()->bEditMode);			
		}

		void CChildView::OnButtonplayseqblock() 
		{
			if (Global::pConfig->_followSong)
			{
				pattern_view()->bScrollDetatch=false;
			}
#if PSYCLE__CONFIGURATION__USE_PSYCORE
			SequencerView& s_view = pattern_view()->main()->m_wndSeq;
			s_view.SwitchToSelBlockPlay();
			Global::pPlayer->start(0);			
#else
 			pattern_view()->prevEditPosition=pattern_view()->editPosition;
			int i=0;
			for ( ; projects_->active_project()->song().playOrderSel[i] == false ; ++i);

			pattern_view()->prevEditPosition=pattern_view()->editPosition;
			if(!Global::pPlayer->playing())
			Global::pPlayer->Start(i,0);
			Global::pPlayer->_playBlock=!Global::pPlayer->_playBlock;
#endif
			main_frame_->StatusBarIdle();
			if ( viewMode == view_modes::pattern ) Repaint(draw_modes::pattern);
		}

		void CChildView::OnUpdateButtonplayseqblock(CCmdUI* pCmdUI) 
		{
#if PSYCLE__CONFIGURATION__USE_PSYCORE
			PlayerTimeInfo tinfo = Player::singleton().timeInfo();
			int check = Player::singleton().loopEnabled() && 
				(tinfo.cycleStartPos() > 0.0f || tinfo.cycleEndPos() < projects_->active_project()->song().sequence().tickLength());
#else
			int check = Global::pPlayer->_playBlock;
#endif
			pCmdUI->SetCheck(check);			
		}

		void CChildView::OnBarstop()
		{
#if PSYCLE__CONFIGURATION__USE_PSYCORE
			psycle::core::Player & player(psycle::core::Player::singleton());
			PlayerTimeInfo tinfo = player.timeInfo();
			bool pl = player.playing();
			bool blk = player.loopEnabled()  && 
				(tinfo.cycleStartPos() > 0.0f || tinfo.cycleEndPos() < projects_->active_project()->song().sequence().tickLength());
			player.stop();
#else
			bool pl = Global::pPlayer->playing();
			bool blk = Global::pPlayer->_playBlock;
			Global::pPlayer->stop();
#endif
			main_frame_->SetAppSongBpm(0);
			main_frame_->SetAppSongTpb(0);

			if (pl)
			{
				if ( Global::pConfig->_followSong && blk)
				{
					pattern_view()->editPosition=pattern_view()->prevEditPosition;
					main_frame_->m_wndSeq.UpdatePlayOrder(false); // <- This restores the selected block
					Repaint(draw_modes::pattern);
				}
				else
				{
#if PSYCLE__CONFIGURATION__USE_PSYCORE
					// TODO
#else
					memset(projects_->active_project()->song().playOrderSel,0,MAX_SONG_POSITIONS*sizeof(bool));
					projects_->active_project()->song().playOrderSel[pattern_view()->editPosition] = true;
					Repaint(draw_modes::cursor); 
#endif
				}
			}
		}

		void CChildView::OnRecordWav() 
		{
			if (!Global::pPlayer->recording()) {
				static char BASED_CODE szFilter[] = "Wav Files (*.wav)|*.wav|All Files (*.*)|*.*||";				
				CFileDialog dlg(false,"wav",NULL,OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,szFilter);
				if ( dlg.DoModal() == IDOK ) {
					Player::singleton().setFileName(dlg.GetFileName().GetBuffer(4));
					Player::singleton().startRecording();
				}
				if ( Global::pConfig->autoStopMachines ) {
					OnAutostop();
				}
			} else {
				Global::pPlayer->stopRecording();
			}
		}

		void CChildView::OnUpdateRecordWav(CCmdUI* pCmdUI) 
		{
			pCmdUI->SetCheck(Global::pPlayer->recording());
		}

		void CChildView::OnAutostop() 
		{
			if ( Global::pConfig->autoStopMachines )
			{
				Global::pConfig->autoStopMachines = false;
				for (int c=0; c<MAX_MACHINES; c++)
				{
					if (projects_->active_project()->song().machine(c))
					{
						projects_->active_project()->song().machine(c)->Standby(false);
					}
				}
			}
			else Global::pConfig->autoStopMachines = true;
		}

		void CChildView::OnUpdateAutostop(CCmdUI* pCmdUI) 
		{
			pCmdUI->SetCheck(Global::pConfig->autoStopMachines);			
		}

		void CChildView::OnFileSongproperties() 
		{
			CSongpDlg dlg(&projects_->active_project()->song());
			dlg.DoModal();
			main_frame_->StatusBarIdle();
			//Repaint();
		}

		void CChildView::OnViewInstrumenteditor()
		{
			main_frame_->ShowInstrumentEditor();
		}

		/// Show the CPU Performance dialog
		void CChildView::OnHelpPsycleenviromentinfo() 
		{
			main_frame_->ShowPerformanceDlg();
		}

		/// Show the MIDI monitor dialog
		void CChildView::OnMidiMonitorDlg() 
		{
			main_frame_->ShowMidiMonitorDlg();
		}
		
		void CChildView::OnNewmachine() 
		{
			machine_view()->ShowNewMachineDlg(-1, -1, 0, false);
		}	

		void CChildView::OnConfigurationSettings() 
		{
			CConfigDlg dlg("Psycle Settings");
			_outputActive = false;
			dlg.Init(Global::pConfig);
			if (dlg.DoModal() == IDOK)
			{
				KillTimer(159);
				if ( Global::pConfig->autosaveSong )
				{
					SetTimer(159,Global::pConfig->autosaveSongTime*60000,NULL);
				}
				_outputActive = true;
				EnableSound();
			}
			//Repaint();
		}

		void CChildView::OnHelpSaludos() 
		{
			CGreetDialog dlg;
			dlg.DoModal();
			//Repaint();
		}	

		//// Right Click Popup Menu
		void CChildView::OnPopCut() { 
			pattern_view()->CopyBlock(true);
		}

		void CChildView::OnUpdateCutCopy(CCmdUI* pCmdUI) {
			pCmdUI->Enable(pattern_view()->blockSelected
						   && (viewMode == view_modes::pattern));
		}

		void CChildView::OnPopCopy() {
			pattern_view()->CopyBlock(false);
		}

		void CChildView::OnPopPaste() {
			pattern_view()->OnPopPaste();
		}
		void CChildView::OnUpdatePaste(CCmdUI* pCmdUI) 
		{
			pCmdUI->Enable(pattern_view()->isBlockCopied  
						   && (viewMode == view_modes::pattern));
		}

		void CChildView::OnPopMixpaste() { 
			pattern_view()->OnPopMixpaste();
		}

		void CChildView::OnPopBlockswitch() {
			pattern_view()->OnPopBlockswitch();
		}

		void CChildView::OnUpdatePopBlockswitch(CCmdUI *pCmdUI) {
			pCmdUI->Enable(pattern_view()->isBlockCopied
						   && (viewMode == view_modes::pattern));
		}

		void CChildView::OnPopDelete() {
			projects_->active_project()->cmd_manager()->ExecuteCommand(new DeleteBlockCommand(pattern_view()));
		}

		void CChildView::OnPopInterpolate() {
			pattern_view()->BlockParamInterpolate();
		}

		void CChildView::OnPopInterpolateCurve() {
			pattern_view()->OnPopInterpolateCurve();
		}

		void CChildView::OnPopChangegenerator() {
			projects_->active_project()->cmd_manager()->ExecuteCommand(
				new ChangeGenCommand(pattern_view(), projects_->active_project()->song().seqBus));
		}

		void CChildView::OnPopChangeinstrument() { 
			projects_->active_project()->cmd_manager()->ExecuteCommand(
				new ChangeInsCommand(pattern_view(), projects_->active_project()->song().auxcolSelected));
		}

		void CChildView::OnPopTranspose1() {
			projects_->active_project()->cmd_manager()->ExecuteCommand(
				new BlockTransposeCommand(pattern_view(), 1));
		}

		void CChildView::OnPopTranspose12() {
			projects_->active_project()->cmd_manager()->ExecuteCommand(
				new BlockTransposeCommand(pattern_view(), 12));
		}

		void CChildView::OnPopTranspose_1() {
			projects_->active_project()->cmd_manager()->ExecuteCommand(
				new BlockTransposeCommand(pattern_view(), -1));
		}

		void CChildView::OnPopTranspose_12() {
			projects_->active_project()->cmd_manager()->ExecuteCommand(
				new BlockTransposeCommand(pattern_view(), -12));
		}

		void CChildView::OnPopTransformpattern() {
			pattern_view()->ShowTransformPatternDlg();			
		}

		void CChildView::OnPopPattenproperties() {
			pattern_view()->ShowPatternDlg();
		}

		/// fill block
		void CChildView::OnPopBlockSwingfill() {
			pattern_view()->ShowSwingFillDlg(FALSE);
		}

		/// fill track
		void CChildView::OnPopTrackSwingfill() {
			pattern_view()->ShowSwingFillDlg(TRUE);
		}

		void CChildView::OnUpdateUndo(CCmdUI* pCmdUI) {
			pattern_view()->OnUpdateUndo(pCmdUI);
		}

		void CChildView::OnUpdateRedo(CCmdUI* pCmdUI) {
			pattern_view()->OnUpdateRedo(pCmdUI);
		}

		void CChildView::OnUpdatePatternCutCopy(CCmdUI* pCmdUI) {
			pCmdUI->Enable(viewMode == view_modes::pattern);
		}

		void CChildView::OnUpdatePatternPaste(CCmdUI* pCmdUI) {
			pCmdUI->Enable(pattern_view()->patBufferCopy&&(viewMode == view_modes::pattern));			
		}

		void CChildView::OnFileImportModulefile() {
			projects_->active_project()->FileImportModulefile();
		}

		void CChildView::OnFileRecent_01() {
			CallOpenRecent(0);
		}

		void CChildView::OnFileRecent_02() {
			CallOpenRecent(1);
		}

		void CChildView::OnFileRecent_03() {
			CallOpenRecent(2);
		}

		void CChildView::OnFileRecent_04() {
			CallOpenRecent(3);
		}

		void CChildView::CallOpenRecent(int pos) {
			UINT nameSize;
			nameSize = GetMenuString(hRecentMenu, pos, 0, 0, MF_BYPOSITION) + 1;
			char* nameBuff = new char[nameSize];
			GetMenuString(hRecentMenu, pos, nameBuff, nameSize, MF_BYPOSITION);
			projects_->active_project()->OnFileLoadsongNamed(nameBuff, 1);
			delete [] nameBuff; nameBuff = 0;
		}

		void CChildView::SetTitleBarText() {
			std::string titlename = "[" + projects_->active_project()->song().fileName;
			if (projects_->active_project()->cmd_manager()->UndoSize() != 0) {
				titlename+=" *";			
			}
			// don't know how to access to the IDR_MAINFRAME String Title.
			titlename += "] Psycle Modular Music Creation Studio (" PSYCLE__VERSION ")";
			main_frame_->SetWindowText(titlename.c_str());
		}

		void CChildView::OnHelpKeybtxt() {
			char path[MAX_PATH];
			sprintf(path,"%sdocs\\keys.txt",Global::pConfig->appPath().c_str());
			ShellExecute(main_frame_->m_hWnd,"open",path,NULL,"",SW_SHOW);
		}

		void CChildView::OnHelpReadme() {
			char path[MAX_PATH];
			sprintf(path,"%sdocs\\readme.txt",Global::pConfig->appPath().c_str());
			ShellExecute(main_frame_->m_hWnd,"open",path,NULL,"",SW_SHOW);
		}

		void CChildView::OnHelpTweaking() {
			char path[MAX_PATH];
			sprintf(path,"%sdocs\\tweaking.txt",Global::pConfig->appPath().c_str());
			ShellExecute(main_frame_->m_hWnd,"open",path,NULL,"",SW_SHOW);
		}

		void CChildView::OnHelpWhatsnew() {
			char path[MAX_PATH];
			sprintf(path,"%sdocs\\whatsnew.txt",Global::pConfig->appPath().c_str());
			ShellExecute(main_frame_->m_hWnd,"open",path,NULL,"",SW_SHOW);
		}

		void CChildView::patTrackMute() {
			if (viewMode == view_modes::pattern) {
				pattern_view()->patTrackMute();
				Repaint(draw_modes::track_header);
			}
		}

		void CChildView::patTrackSolo() {
			if (viewMode == view_modes::pattern) {
				pattern_view()->patTrackSolo();
			}
		}

		void CChildView::patTrackRecord() {
			if (viewMode == view_modes::pattern) {
				pattern_view()->patTrackRecord();
			}
		}
		
		void CChildView::OnConfigurationLoopplayback() {
			if (Player::singleton().loopEnabled()) {
				Player::singleton().UnsetLoop();
			} else {
				Player::singleton().setLoopSong();
			}
		}

		void CChildView::OnUpdateConfigurationLoopplayback(CCmdUI* pCmdUI) {
			pCmdUI->SetCheck(Player::singleton().loopEnabled());
		}

		void CChildView::DrawAllMachineVumeters(CDC *devc) {
			if (Global::pConfig->draw_vus)
				machine_view_->UpdateVUs(devc);
		}

		//////////////////////////////////////////////////////////////////////
		// Pattern Modifier functions ( Copy&paste , Transpose, ... )

		void CChildView::OnPatCut() {
			if (viewMode == view_modes::pattern) {
				pattern_view()->patCut();
			}
		}

		void CChildView::OnPatCopy() {
			if (viewMode == view_modes::pattern) {	
				pattern_view()->patCopy();
			}
		}

		void CChildView::OnPatPaste() {
			projects_->active_project()->cmd_manager()->ExecuteCommand(
				new PatPasteCommand(pattern_view()));
		}

		void CChildView::OnPatMixPaste() {
			projects_->active_project()->cmd_manager()->ExecuteCommand(
				new PatPasteCommand(pattern_view(), true));
		}

		void CChildView::OnPatDelete() {
			if (viewMode == view_modes::pattern) {
				projects_->active_project()->cmd_manager()->ExecuteCommand(
					new PatDeleteCommand(pattern_view()));
			}
		}	

		void CChildView::OnEditUndo() {
			if (projects_->active_project())
				projects_->active_project()->cmd_manager()->Undo();
		}

		void CChildView::OnEditRedo() {
			if (projects_->active_project())
				projects_->active_project()->cmd_manager()->Redo();
		}

	}	// namespace host
}	// namespace psycle


// User/Mouse Responses, private headers included only by this translation unit
#include "KeybHandler.private.hpp"
#include "MouseHandler.private.hpp"
