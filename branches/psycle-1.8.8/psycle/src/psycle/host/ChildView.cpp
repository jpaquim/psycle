///\file
///\brief implementation file for psycle::host::CChildView.
#include <psycle/host/detail/project.private.hpp>
#include "ChildView.hpp"

#include "Configuration.hpp"
#include "MainFrm.hpp"
#include "InputHandler.hpp"
#include "MidiInput.hpp"

#include "ConfigDlg.hpp"
#include "GreetDialog.hpp"
#include "SaveWavDlg.hpp"
#include "SongpDlg.hpp"
#include "MasterDlg.hpp"
#include "XMSamplerUI.hpp"
#include "WireDlg.hpp"
#include "MacProp.hpp"
#include "NewMachine.hpp"
#include "TransformPatternDlg.hpp"
#include "PatDlg.hpp"
#include "SwingFillDlg.hpp"
#include "InterpolateCurveDlg.hpp"

#include "ITModule2.h"
#include "XMSongLoader.hpp"
#include "XMSongExport.hpp"
#include "Player.hpp"
#include "VstHost24.hpp" //included because of the usage of a call in the Timer function. It should be standarized to the Machine class.

#include <cmath> // SwingFill

namespace psycle { namespace host {
		int const ID_TIMER_VIEW_REFRESH =39;
		int const ID_TIMER_AUTOSAVE = 159;

		CMainFrame		*pParentMain;
		char* CChildView::notes_tab_a440[256] = {
			"C-m","C#m","D-m","D#m","E-m","F-m","F#m","G-m","G#m","A-m","A#m","B-m", //0
			"C-0","C#0","D-0","D#0","E-0","F-0","F#0","G-0","G#0","A-0","A#0","B-0", //1
			"C-1","C#1","D-1","D#1","E-1","F-1","F#1","G-1","G#1","A-1","A#1","B-1", //2
			"C-2","C#2","D-2","D#2","E-2","F-2","F#2","G-2","G#2","A-2","A#2","B-2", //3
			"C-3","C#3","D-3","D#3","E-3","F-3","F#3","G-3","G#3","A-3","A#3","B-3", //4
			"C-4","C#4","D-4","D#4","E-4","F-4","F#4","G-4","G#4","A-4","A#4","B-4", //5
			"C-5","C#5","D-5","D#5","E-5","F-5","F#5","G-5","G#5","A-5","A#5","B-5", //6
			"C-6","C#6","D-6","D#6","E-6","F-6","F#6","G-6","G#6","A-6","A#6","B-6", //7
			"C-7","C#7","D-7","D#7","E-7","F-7","F#7","G-7","G#7","A-7","A#7","B-7", //8
			"C-8","C#8","D-8","D#8","E-8","F-8","F#8","G-8","G#8","A-8","A#8","B-8", //9
			"off","twk","twf","mcm","tws","   ","   ","   ",
			"   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ",
			"   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ",
			"   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ",
			"   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ",
			"   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ",
			"   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ",
			"   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ",
			"   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ",
		};
		char* CChildView::notes_tab_a220[256] = {
			"C-0","C#0","D-0","D#0","E-0","F-0","F#0","G-0","G#0","A-0","A#0","B-0", //0
			"C-1","C#1","D-1","D#1","E-1","F-1","F#1","G-1","G#1","A-1","A#1","B-1", //1
			"C-2","C#2","D-2","D#2","E-2","F-2","F#2","G-2","G#2","A-2","A#2","B-2", //2
			"C-3","C#3","D-3","D#3","E-3","F-3","F#3","G-3","G#3","A-3","A#3","B-3", //3
			"C-4","C#4","D-4","D#4","E-4","F-4","F#4","G-4","G#4","A-4","A#4","B-4", //4
			"C-5","C#5","D-5","D#5","E-5","F-5","F#5","G-5","G#5","A-5","A#5","B-5", //5
			"C-6","C#6","D-6","D#6","E-6","F-6","F#6","G-6","G#6","A-6","A#6","B-6", //6
			"C-7","C#7","D-7","D#7","E-7","F-7","F#7","G-7","G#7","A-7","A#7","B-7", //7
			"C-8","C#8","D-8","D#8","E-8","F-8","F#8","G-8","G#8","A-8","A#8","B-8", //8
			"C-9","C#9","D-9","D#9","E-9","F-9","F#9","G-9","G#9","A-9","A#9","B-9", //9
			"off","twk","twf","mcm","tws","   ","   ","   ",
			"   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ",
			"   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ",
			"   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ",
			"   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ",
			"   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ",
			"   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ",
			"   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ",
			"   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ","   ",
		};
		char* CChildView::hex_tab[16] = {
			"0","1","2","3","4","5","6","7","8","9","A","B","C","D","E","F"
		};

		CChildView::CChildView()
			:pParentFrame(0)
			,hRecentMenu(0)
			,MasterMachineDialog(NULL)
			,SamplerMachineDialog(NULL)
			,XMSamplerMachineDialog(NULL)
			,WaveInMachineDialog(NULL)
			,blockSelected(false)
			,blockStart(false)
			,blockswitch(false)
			,blockSelectBarState(1)
			,bScrollDetatch(false)
			,bEditMode(true)
			,patStep(1)
			,editPosition(0)
			,prevEditPosition(0)
			,ChordModeOffs(0)
			,updateMode(0)
			,updatePar(0)
			,viewMode(view_modes::machine)
			,_outputActive(false)
			,CW(300)
			,CH(200)
			,maxView(false)
			,textLeftEdge(2)
			,bmpDC(NULL)
			,playpos(-1)
			,newplaypos(-1) 
			,numPatternDraw(0)
			,smac(-1)
			,smacmode(smac_modes::move)
			,wiresource(-1)
			,wiredest(-1)
			,wiremove(-1)
			,wireSX(0)
			,wireSY(0)
			,wireDX(0)
			,wireDY(0)
			,allowcontextmenu(false)
			,maxt(1)
			,maxl(1)
			,tOff(0)
			,lOff(0)
			,ntOff(0)
			,nlOff(0)
			,scrollDelay(0)
			,rntOff(0)
			,rnlOff(0)
			,isBlockCopied(false)
			,blockNTracks(0)
			,blockNLines(0)
			,mcd_x(0)
			,mcd_y(0)
			,patBufferLines(0)
			,patBufferCopy(false)
		{
			for(int c(0) ; c < MAX_WIRE_DIALOGS ; ++c)
			{
				WireDialog[c] = NULL;
			}
			for (int c=0; c<256; c++)	{ FLATSIZES[c]=8; }
			selpos.bottom=0;
			newselpos.bottom=0;
			szBlankParam[0]='\0';
			note_tab_selected=NULL;
			MBStart.x=0;
			MBStart.y=0;

			patView = &Global::psycleconf().patView();
			macView = &Global::psycleconf().macView();
			PatHeaderCoords = &patView->PatHeaderCoords;
			MachineCoords = &macView->MachineCoords;

			Global::pInputHandler->SetChildView(this);

			// Creates a new song object. The application Song.
			Global::_pSong->New();

			// Referencing the childView song pointer to the
			// Main Global::_pSong object [The application Global::_pSong]
			_pSong = Global::_pSong;
		}

		CChildView::~CChildView()
		{
			Global::pInputHandler->SetChildView(NULL);

			if ( bmpDC != NULL )
			{
				char buf[100];
				sprintf(buf,"CChildView::~CChildView(). Deleted bmpDC (was 0x%.8X)\n",(int)bmpDC);
				TRACE(buf);
				bmpDC->DeleteObject();
				delete bmpDC; bmpDC = 0;
			}
		}

		BEGIN_MESSAGE_MAP(CChildView,CWnd )
			ON_WM_TIMER()
			ON_WM_PAINT()
			ON_WM_DESTROY()
			ON_WM_SIZE()
			ON_WM_CONTEXTMENU()
			ON_WM_HSCROLL()
			ON_WM_VSCROLL()
			ON_WM_KEYDOWN()
			ON_WM_KEYUP()
			ON_WM_LBUTTONDOWN()
			ON_WM_RBUTTONDOWN()
			ON_WM_LBUTTONUP()
			ON_WM_RBUTTONUP()
			ON_WM_LBUTTONDBLCLK()
			ON_WM_MBUTTONDOWN()
			ON_WM_MOUSEMOVE()
			ON_WM_MOUSEWHEEL()
//Main menu and toolbar (A few entries are in MainFrm)
			ON_COMMAND(ID_FILE_NEW, OnFileNew)
			ON_COMMAND(ID_FILE_LOADSONG, OnFileLoadsong)
			ON_COMMAND(ID_FILE_IMPORT_XMFILE, OnFileImportModulefile)
			ON_COMMAND_EX(ID_FILE_SAVE, OnFileSave)
			ON_COMMAND_EX(ID_FILE_SAVE_AS, OnFileSaveAs)
			ON_COMMAND_EX(ID_EXPORT, OnExport)
			ON_COMMAND(ID_FILE_SAVEAUDIO, OnFileSaveaudio)
			ON_COMMAND(ID_FILE_SONGPROPERTIES, OnFileSongproperties)
			ON_COMMAND(ID_FILE_REVERT, OnFileRevert)
			ON_COMMAND(ID_FILE_RECENT_01, OnFileRecent_01)
			ON_COMMAND(ID_FILE_RECENT_02, OnFileRecent_02)
			ON_COMMAND(ID_FILE_RECENT_03, OnFileRecent_03)
			ON_COMMAND(ID_FILE_RECENT_04, OnFileRecent_04)
			ON_COMMAND(ID_RECORDB, OnRecordWav)
			ON_UPDATE_COMMAND_UI(ID_RECORDB, OnUpdateRecordWav)
			ON_COMMAND(ID_EDIT_UNDO, OnEditUndo)
			ON_COMMAND(ID_EDIT_REDO, OnEditRedo)
			ON_COMMAND(ID_EDIT_CUT, patCut)
			ON_COMMAND(ID_EDIT_COPY, patCopy)
			ON_COMMAND(ID_EDIT_PASTE, patPaste)
			ON_COMMAND(ID_EDIT_MIXPASTE, patMixPaste)
			ON_COMMAND(ID_EDIT_DELETE, patDelete)
			ON_UPDATE_COMMAND_UI(ID_EDIT_UNDO, OnUpdateUndo)
			ON_UPDATE_COMMAND_UI(ID_EDIT_REDO, OnUpdateRedo)
			ON_UPDATE_COMMAND_UI(ID_EDIT_CUT, OnUpdatePatternCutCopy)
			ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, OnUpdatePatternCutCopy)
			ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE, OnUpdatePatternPaste)
			ON_UPDATE_COMMAND_UI(ID_EDIT_MIXPASTE, OnUpdatePatternPaste)
			ON_UPDATE_COMMAND_UI(ID_EDIT_DELETE, OnUpdatePatternCutCopy)
			ON_COMMAND(ID_BARREC, OnBarrec)
			ON_COMMAND(ID_BARPLAYFROMSTART, OnBarplayFromStart)
			ON_COMMAND(ID_BARPLAY, OnBarplay)
			ON_COMMAND(ID_BUTTONPLAYSEQBLOCK, OnButtonplayseqblock)
			ON_COMMAND(ID_BARSTOP, OnBarstop)
			ON_COMMAND(ID_AUTOSTOP, OnAutostop)
			ON_COMMAND(ID_CONFIGURATION_LOOPPLAYBACK, OnConfigurationLoopplayback)
			ON_UPDATE_COMMAND_UI(ID_BARREC, OnUpdateBarrec)
			ON_UPDATE_COMMAND_UI(ID_BARPLAYFROMSTART, OnUpdateBarplayFromStart)
			ON_UPDATE_COMMAND_UI(ID_BARPLAY, OnUpdateBarplay)
			ON_UPDATE_COMMAND_UI(ID_BUTTONPLAYSEQBLOCK, OnUpdateButtonplayseqblock)
			ON_UPDATE_COMMAND_UI(ID_AUTOSTOP, OnUpdateAutostop)
			ON_UPDATE_COMMAND_UI(ID_CONFIGURATION_LOOPPLAYBACK, OnUpdateConfigurationLoopplayback)
			ON_COMMAND(ID_MACHINEVIEW, OnMachineview)
			ON_COMMAND(ID_PATTERNVIEW, OnPatternView)	
			ON_COMMAND(ID_SHOWPSEQ, OnShowPatternSeq)
			//Show Gear Rack is the command IDC_GEAR_RACK of the machine bar (in mainfrm)
			ON_COMMAND(ID_NEWMACHINE, OnNewmachine)
			ON_COMMAND(ID_VIEW_INSTRUMENTEDITOR, OnViewInstrumenteditor)
			//Show Wave editor is the command IDC_WAVEBUT of the machine bar (in mainfrm)
			ON_UPDATE_COMMAND_UI(ID_MACHINEVIEW, OnUpdateMachineview)
			ON_UPDATE_COMMAND_UI(ID_PATTERNVIEW, OnUpdatePatternView)
			ON_UPDATE_COMMAND_UI(ID_SHOWPSEQ, OnUpdatePatternSeq)
			ON_COMMAND(ID_CONFIGURATION_SETTINGS, OnConfigurationSettings)
			ON_COMMAND(ID_CONFIGURATION_ENABLEAUDIO, OnEnableAudio)
			ON_UPDATE_COMMAND_UI(ID_CONFIGURATION_ENABLEAUDIO, OnUpdateEnableAudio)
			ON_COMMAND(ID_CPUPERFORMANCE, OnHelpPsycleenviromentinfo)
			ON_COMMAND(ID_MIDI_MONITOR, OnMidiMonitorDlg)
			ON_COMMAND(ID_HELP_README, OnHelpReadme)
			ON_COMMAND(ID_HELP_KEYBTXT, OnHelpKeybtxt)
			ON_COMMAND(ID_HELP_TWEAKING, OnHelpTweaking)
			ON_COMMAND(ID_HELP_WHATSNEW, OnHelpWhatsnew)
			ON_COMMAND(ID_HELP_SALUDOS, OnHelpSaludos)
//Pattern Popup
			ON_COMMAND(ID_POP_CUT, OnPopCut)
			ON_COMMAND(ID_POP_COPY, OnPopCopy)
			ON_COMMAND(ID_POP_PASTE, OnPopPaste)
			ON_COMMAND(ID_POP_MIXPASTE, OnPopMixpaste)
			ON_COMMAND(ID_POP_BLOCKSWITCH, OnPopBlockswitch)
			ON_COMMAND(ID_POP_DELETE, OnPopDelete)
			ON_COMMAND(ID_POP_ADDNEWTRACK, OnPopAddNewTrack)
			ON_COMMAND(ID_POP_INTERPOLATE, OnPopInterpolate)
			ON_COMMAND(ID_POP_INTERPOLATE_CURVE, OnPopInterpolateCurve)
			ON_COMMAND(ID_POP_CHANGEGENERATOR, OnPopChangegenerator)
			ON_COMMAND(ID_POP_CHANGEINSTRUMENT, OnPopChangeinstrument)
			ON_COMMAND(ID_POP_TRANSPOSE1, OnPopTranspose1)
			ON_COMMAND(ID_POP_TRANSPOSE12, OnPopTranspose12)
			ON_COMMAND(ID_POP_TRANSPOSE_1, OnPopTranspose_1)
			ON_COMMAND(ID_POP_TRANSPOSE_12, OnPopTranspose_12)
			ON_COMMAND(ID_POP_BLOCK_SWINGFILL, OnPopBlockSwingfill)
			ON_COMMAND(ID_POP_TRACK_SWINGFILL, OnPopTrackSwingfill)
			ON_COMMAND(ID_POP_TRANSFORMPATTERN, OnPopTransformpattern)
			ON_COMMAND(ID_POP_PATTENPROPERTIES, OnPopPattenproperties)
			ON_UPDATE_COMMAND_UI(ID_POP_CUT, OnUpdateCutCopy)
			ON_UPDATE_COMMAND_UI(ID_POP_COPY, OnUpdateCutCopy)
			ON_UPDATE_COMMAND_UI(ID_POP_MIXPASTE, OnUpdatePaste)
			ON_UPDATE_COMMAND_UI(ID_POP_PASTE, OnUpdatePaste)
			ON_UPDATE_COMMAND_UI(ID_POP_BLOCKSWITCH, OnUpdatePopBlockswitch)
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
					(HBRUSH)GetStockObject( HOLLOW_BRUSH ),
					NULL
				);
			return TRUE;
		}

		/// This function gives to the pParentMain the pointer to a CMainFrm
		/// object. Call this function from the CMainFrm side object to
		/// allow CChildView call functions of the CMainFrm parent object
		/// Call this function after creating both the CChildView object and
		/// the CMainFrm object
		void CChildView::ValidateParent()
		{
			pParentMain=(CMainFrame *)pParentFrame;
		}

		/// Timer initialization
		void CChildView::InitTimer()
		{
			KillTimer(ID_TIMER_VIEW_REFRESH);
			KillTimer(ID_TIMER_AUTOSAVE);
			if (!SetTimer(ID_TIMER_VIEW_REFRESH,30,NULL)) // GUI update. 
			{
				AfxMessageBox(IDS_COULDNT_INITIALIZE_TIMER, MB_ICONERROR);
			}

			if ( Global::psycleconf().autosaveSong )
			{
				if (!SetTimer(ID_TIMER_AUTOSAVE,Global::psycleconf().autosaveSongTime*60000,NULL)) // Autosave Song
				{
					AfxMessageBox(IDS_COULDNT_INITIALIZE_TIMER, MB_ICONERROR);
				}
			}
		}

		/// Timer handler
		void CChildView::OnTimer( UINT_PTR nIDEvent )
		{
			if (nIDEvent == ID_TIMER_VIEW_REFRESH)
			{
				CSingleLock lock(&Global::_pSong->semaphore, FALSE);
				if (!lock.Lock(50)) return;
				Master* master = ((Master*)Global::_pSong->_pMachine[MASTER_INDEX]);
				if (master)
				{
					pParentMain->UpdateVumeters
						(
							master->volumeDisplayLeft,
							master->volumeDisplayRight,
							macView->vu1,
							macView->vu2,
							macView->vu3,
							master->_clip
						);
					pParentMain->UpdateMasterValue(master->_outDry);
					if ( MasterMachineDialog ) {
						MasterMachineDialog->UpdateUI();
						for (int i=0;i<MAX_CONNECTIONS; i++)
						{
							if ( master->_inputCon[i])
							{
								if (_pSong->_pMachine[master->_inputMachines[i]])
								{
									strcpy(MasterMachineDialog->macname[i],_pSong->_pMachine[master->_inputMachines[i]]->_editName);
								}
							}
							else {
								strcpy(MasterMachineDialog->macname[i],"");
							}
						}
					}
					master->vuupdated = true;
				}
				if (viewMode == view_modes::machine)
				{
						Repaint(draw_modes::playback);
				}

				for(int c=0; c<MAX_MACHINES; c++)
				{
					if (_pSong->_pMachine[c])
					{
						if ( _pSong->_pMachine[c]->_type == MACH_VST ||
							_pSong->_pMachine[c]->_type == MACH_VSTFX )
						{
							//I don't know if this has to be done in a synchronized thread
							//(like this one) neither if it can take a moderate amount of time.
							((vst::plugin*)_pSong->_pMachine[c])->Idle();
						}
					}
				}

				if (XMSamplerMachineDialog != NULL ) XMSamplerMachineDialog->UpdateUI();
				if (Global::pPlayer->_playing)
				{
					if (Global::pPlayer->_lineChanged)
					{
						Global::pPlayer->_lineChanged = false;
						pParentMain->SetAppSongBpm(0);
						pParentMain->SetAppSongTpb(0);

						if (Global::psycleconf()._followSong)
						{
							CListBox* pSeqList = (CListBox*)pParentMain->m_seqBar.GetDlgItem(IDC_SEQLIST);
							editcur.line=Global::pPlayer->_lineCounter;
							if (editPosition != Global::pPlayer->_playPosition)
							//if (pSeqList->GetCurSel() != Global::pPlayer->_playPosition)
							{
								pSeqList->SelItemRange(false,0,pSeqList->GetCount());
								pSeqList->SetSel(Global::pPlayer->_playPosition,true);
								int top = Global::pPlayer->_playPosition - 0xC;
								if (top < 0) top = 0;
								pSeqList->SetTopIndex(top);
								editPosition=Global::pPlayer->_playPosition;
								if ( viewMode == view_modes::pattern ) 
								{ 
									Repaint(draw_modes::pattern);//draw_modes::playback_change);  // Until this mode is coded there is no point in calling it since it just makes patterns not refresh correctly currently
									Repaint(draw_modes::playback);
								}
							}
							else if( viewMode == view_modes::pattern ) Repaint(draw_modes::playback);
						}
						else if ( viewMode == view_modes::pattern ) Repaint(draw_modes::playback);
						if ( viewMode == view_modes::sequence ) Repaint(draw_modes::playback);
					}
				}
			}
			if (nIDEvent == ID_TIMER_AUTOSAVE && !Global::pPlayer->_recording)
			{
				CString filepath = Global::psycleconf().GetSongDir().c_str();
				filepath += "\\autosave.psy";
				OldPsyFile file;
				if(!file.Create(static_cast<LPCTSTR>(filepath), true)) return;
				CProgressDialog progress(NULL,false);
				_pSong->Save(&file,progress, true);
				if (!file.Close())
				{
					std::ostringstream s;
					s << "Error writing to file '" << file.szName << "'" << std::endl;
					MessageBox(s.str().c_str(),"File Error!!!",0);
				}
			}
		}

		void CChildView::OnEnableAudio()
		{
			AudioDriver* pOut = Global::psycleconf()._pOutputDriver;
			if (pOut->Enabled()) {
				pOut->Enable(false);
				_outputActive = false;
			}
			else {
				_outputActive = pOut->Enable(true);
			}
		}
		void CChildView::OnUpdateEnableAudio(CCmdUI* pCmdUI) 
		{
			AudioDriver* pOut = Global::psycleconf()._pOutputDriver;
			pCmdUI->SetCheck(pOut->Enabled());
		}
		void CChildView::EnableSound()
		{
			if (_outputActive)
			{
				AudioDriver* pOut = Global::psycleconf()._pOutputDriver;

				_outputActive = false;
				if (!pOut->Enabled())
				{
					_outputActive = pOut->Enable(true);
				}
				else {
					_outputActive = true;
				}
			}
			// set midi input mode to real-time or step
			if(viewMode == view_modes::machine && Global::psycleconf().midi()._midiMachineViewSeqMode)
				Global::midi().m_midiMode = MODE_REALTIME;
			else
				Global::midi().m_midiMode = MODE_STEP;
		}


		/// Put exit destroying code here...
		void CChildView::OnDestroy()
		{
			if (Global::psycleconf()._pOutputDriver->Initialized())
			{
				Global::psycleconf()._pOutputDriver->Reset();
			}
			KillTimer(ID_TIMER_VIEW_REFRESH);
			KillTimer(ID_TIMER_AUTOSAVE);
		}

		void CChildView::OnPaint() 
		{
			if (!GetUpdateRect(NULL) ) return; // If no area to update, exit.
			CPaintDC dc(this);

			if ( bmpDC == NULL ) // buffer creation
			{
				CRect rc;
				GetClientRect(&rc);
				bmpDC = new CBitmap;
				bmpDC->CreateCompatibleBitmap(&dc,rc.right-rc.left,rc.bottom-rc.top);
				char buf[100];
				sprintf(buf,"CChildView::OnPaint(). Initialized bmpDC to 0x%.8X\n",(int)bmpDC);
				TRACE(buf);
			}
			CDC bufDC;
			bufDC.CreateCompatibleDC(&dc);
			CBitmap* oldbmp;
			oldbmp = bufDC.SelectObject(bmpDC);
			if (viewMode==view_modes::machine)	// Machine view paint handler
			{
				switch (updateMode)
				{
				case draw_modes::all:
					DrawMachineEditor(&bufDC);
					break;
				case draw_modes::machine:
					//ClearMachineSpace(Global::_pSong->_pMachines[updatePar], updatePar, &bufDC);
					DrawMachine(updatePar, &bufDC);
					DrawMachineVumeters(updatePar, &bufDC);
					updateMode=draw_modes::all;
					break;
				case draw_modes::playback:
					///\todo need to refresh also mute/solo/bypass and panning.
					//warning: doing DrawMachine can cause problems if transparent
					//graphics or if machine text is drawn outside of the machine.
					DrawAllMachineVumeters(&bufDC);
					break;
				case draw_modes::all_machines:
					for (int i=0;i<MAX_MACHINES;i++)
					{
						if (_pSong->_pMachine[i])
						{
							DrawMachine(i, &bufDC);
						}
					}
					DrawAllMachineVumeters(&bufDC);
					break;
				}
			}
			else if (viewMode == view_modes::pattern)	// Pattern view paint handler
			{
				DrawPatEditor(&bufDC);
			}
			else if ( viewMode == view_modes::sequence)
			{
				DrawSeqEditor(&bufDC);
			}

			CRect rc;
			GetClientRect(&rc);
			dc.BitBlt(0,0,rc.right-rc.left,rc.bottom-rc.top,&bufDC,0,0,SRCCOPY);
			bufDC.SelectObject(oldbmp);
			bufDC.DeleteDC();
		}

		void CChildView::Repaint(draw_modes::draw_mode drawMode)
		{
			switch(viewMode)
			{
			case view_modes::machine:
				{
					if ( drawMode <= draw_modes::machine )
					{
						updateMode = drawMode;
						Invalidate(false);
					}
					else if ( drawMode == draw_modes::playback )
					{
						updateMode = drawMode;
						Invalidate(false);
					}
				}
				break;
			case view_modes::pattern:
				{
					if (drawMode >= draw_modes::pattern || drawMode == draw_modes::all )	
					{
						PreparePatternRefresh(drawMode);
					}
				}
				break;
			case view_modes::sequence:
				{
					Invalidate(false);
				}
				break;
			}
		}

		void CChildView::OnSize(UINT nType, int cx, int cy) 
		{
			CWnd ::OnSize(nType, cx, cy);

			CW = cx;
			CH = cy;
			
			if ( bmpDC != NULL ) // remove old buffer to force recreating it with new size
			{
				TRACE("CChildView::OnResize(). Deleted bmpDC");
				bmpDC->DeleteObject();
				delete bmpDC; bmpDC = 0;
			}
			if (viewMode == view_modes::pattern)
			{
				RecalcMetrics();
			}
			Repaint();
		}

		/// "Save Song" Function
		BOOL CChildView::OnExport(UINT id) 
		{
			OPENFILENAME ofn; // common dialog box structure
			std::string ifile = Global::_pSong->fileName.substr(0,Global::_pSong->fileName.length()-4) + ".xm";
			std::string if2 = ifile.substr(0,ifile.find_first_of("\\/:*\"<>|"));
			
			char szFile[_MAX_PATH];

			szFile[_MAX_PATH-1]=0;
			strncpy(szFile,if2.c_str(),_MAX_PATH-1);
			
			// Initialize OPENFILENAME
			ZeroMemory(&ofn, sizeof(OPENFILENAME));
			ofn.lStructSize = sizeof(OPENFILENAME);
			ofn.hwndOwner = GetParent()->m_hWnd;
			ofn.lpstrFile = szFile;
			ofn.nMaxFile = sizeof(szFile);
			ofn.lpstrFilter = "FastTracker 2 Song (*.xm)\0*.xm\0All (*.*)\0*.*\0";
			ofn.nFilterIndex = 1;
			ofn.lpstrFileTitle = NULL;
			ofn.nMaxFileTitle = 0;
			std::string tmpstr = Global::psycleconf().GetCurrentSongDir();
			ofn.lpstrInitialDir = tmpstr.c_str();
			ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;
			BOOL bResult = TRUE;
			
			// Display the Open dialog box. 
			if (GetSaveFileName(&ofn) == TRUE)
			{
				CString str = ofn.lpstrFile;

				CString str2 = str.Right(3);
				if ( str2.CompareNoCase(".xm") != 0 ) str.Insert(str.GetLength(),".xm");
				int index = str.ReverseFind('\\');
				XMSongExport file;

				if (index != -1)
				{
					Global::psycleconf().SetCurrentSongDir(static_cast<char const *>(str.Left(index)));
				}
				
				if (!file.Create(static_cast<LPCTSTR>(str), true))
				{
					MessageBox("Error creating file!", "Error!", MB_OK);
					return FALSE;
				}
				file.exportsong(*Global::_pSong);
				file.Close();
			}
			else
			{
				return FALSE;
			}
			return bResult;
		}

		/// "Save Song" Function
		BOOL CChildView::OnFileSave(UINT id) 
		{
			//MessageBox("Saving Disabled");
			//return false;
			BOOL bResult = TRUE;
			if ( Global::_pSong->_saved )
			{
				if (MessageBox("Proceed with Saving?","Song Save",MB_YESNO) == IDYES)
				{
					std::string filepath = Global::psycleconf().GetCurrentSongDir();
					filepath += '\\';
					filepath += Global::_pSong->fileName;
					
					OldPsyFile file;
					CProgressDialog progress;
					if (!file.Create((char*)filepath.c_str(), true))
					{
						MessageBox("Error creating file!", "Error!", MB_OK);
						return FALSE;
					}
					progress.SetWindowText("Saving...");
					progress.ShowWindow(SW_SHOW);
					if (!_pSong->Save(&file, progress))
					{
						MessageBox("Error saving file!", "Error!", MB_OK);
						bResult = FALSE;
					}
					else 
					{
						_pSong->_saved=true;
						Global::pInputHandler->SafePoint();
					}
					progress.SendMessage(WM_CLOSE);
					if (!file.Close())
					{
						std::ostringstream s;
						s << "Error writing to file '" << file.szName << "'" << std::endl;
						MessageBox(s.str().c_str(),"File Error!!!",0);
					}
				}
				else 
				{
					return FALSE;
				}
			}
			else 
			{
				return OnFileSaveAs(0);
			}
			return bResult;
		}

		//////////////////////////////////////////////////////////////////////
		// "Save Song As" Function

		BOOL CChildView::OnFileSaveAs(UINT id) 
		{
			//MessageBox("Saving Disabled");
			//return false;
			OPENFILENAME ofn; // common dialog box structure
			std::string ifile = Global::_pSong->fileName;
			std::string if2 = ifile.substr(0,ifile.find_first_of("\\/:*\"<>|"));
			
			char szFile[_MAX_PATH];

			szFile[_MAX_PATH-1]=0;
			strncpy(szFile,if2.c_str(),_MAX_PATH-1);
			
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
			std::string tmpstr = Global::psycleconf().GetCurrentSongDir();
			ofn.lpstrInitialDir = tmpstr.c_str();
			ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;
			BOOL bResult = TRUE;
			
			// Display the Open dialog box. 
			if (GetSaveFileName(&ofn) == TRUE)
			{
				CString str = ofn.lpstrFile;
				if ( ofn.nFilterIndex == 2 ) 
				{
					CString str2 = str.Right(4);
					if ( str2.CompareNoCase(".psb") != 0 ) str.Insert(str.GetLength(),".psb");
					sprintf(szFile,str);
					FILE* hFile=fopen(szFile,"wb");
					SaveBlock(hFile);
					fflush(hFile);
					fclose(hFile);
				}
				else 
				{ 
					CString str2 = str.Right(4);
					if ( str2.CompareNoCase(".psy") != 0 ) str.Insert(str.GetLength(),".psy");
					int index = str.ReverseFind('\\');
					OldPsyFile file;
					CProgressDialog progress;
					if (index != -1)
					{
						Global::psycleconf().SetCurrentSongDir(static_cast<char const *>(str.Left(index)));
						Global::_pSong->fileName = str.Mid(index+1);
					}
					else
					{
						Global::_pSong->fileName = str;
					}
					
					if (!file.Create(static_cast<LPCTSTR>(str), true))
					{
						MessageBox("Error creating file!", "Error!", MB_OK);
						return FALSE;
					}

					progress.SetWindowText("Saving...");
					progress.ShowWindow(SW_SHOW);
					if (!_pSong->Save(&file,progress))
					{
						MessageBox("Error saving file!", "Error!", MB_OK);
						bResult = FALSE;
					}
					else 
					{
						_pSong->_saved=true;
						std::string recent = static_cast<LPCTSTR>(str);
						AppendToRecent(recent);
						
						Global::pInputHandler->SafePoint();
					}
					progress.SendMessage(WM_CLOSE);
					if (!file.Close())
					{
						std::ostringstream s;
						s << "Error writing to file '" << file.szName << "'" << std::endl;
						MessageBox(s.str().c_str(),"File Error!!!",0);
					}
				}
			}
			else
			{
				return FALSE;
			}
			return bResult;
		}

		#include <cderr.h>

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
			std::string tmpstr = Global::psycleconf().GetCurrentSongDir();
			ofn.lpstrInitialDir = tmpstr.c_str();
			ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
			
			// Display the Open dialog box. 
			if(::GetOpenFileName(&ofn)==TRUE)
			{
				OnFileLoadsongNamed(szFile, ofn.nFilterIndex);
			}
			else
			{
				DWORD comDlgErr = CommDlgExtendedError();
				switch(comDlgErr)
				{
				case CDERR_DIALOGFAILURE:
					MessageBox("CDERR_DIALOGFAILURE", "exception", MB_OK | MB_ICONERROR);
					break;
				case CDERR_FINDRESFAILURE:
					MessageBox("CDERR_FINDRESFAILURE", "exception", MB_OK | MB_ICONERROR);
					break;
				case CDERR_INITIALIZATION:
					MessageBox("CDERR_INITIALIZATION", "exception", MB_OK | MB_ICONERROR);
					break;
				case CDERR_LOADRESFAILURE:
					MessageBox("CDERR_LOADRESFAILURE", "exception", MB_OK | MB_ICONERROR);
					break;
				case CDERR_LOADSTRFAILURE:
					MessageBox("CDERR_LOADSTRFAILURE", "exception", MB_OK | MB_ICONERROR);
					break;
				case CDERR_LOCKRESFAILURE:
					MessageBox("CDERR_LOCKRESFAILURE", "exception", MB_OK | MB_ICONERROR);
					break;
				case CDERR_MEMALLOCFAILURE:
					MessageBox("CDERR_MEMALLOCFAILURE", "exception", MB_OK | MB_ICONERROR);
					break;
				case CDERR_MEMLOCKFAILURE:
					MessageBox("CDERR_MEMLOCKFAILURE", "exception", MB_OK | MB_ICONERROR);
					break;
				case CDERR_NOHINSTANCE:
					MessageBox("CDERR_NOHINSTANCE", "exception", MB_OK | MB_ICONERROR);
					break;
				case CDERR_NOHOOK:
					MessageBox("CDERR_NOHOOK", "exception", MB_OK | MB_ICONERROR);
					break;
				case CDERR_NOTEMPLATE:
					MessageBox("CDERR_NOTEMPLATE", "exception", MB_OK | MB_ICONERROR);
					break;
				case CDERR_REGISTERMSGFAIL:
					MessageBox("CDERR_REGISTERMSGFAIL", "exception", MB_OK | MB_ICONERROR);
					break;
				case CDERR_STRUCTSIZE:
					MessageBox("CDERR_STRUCTSIZE", "exception", MB_OK | MB_ICONERROR);
					break;
				}
			}
			pParentMain->StatusBarIdle();
		}

		void CChildView::OnFileNew() 
		{
			if (CheckUnsavedSong("New Song"))
			{
				Global::pInputHandler->KillUndo();
				Global::pInputHandler->KillRedo();
				pParentMain->CloseAllMacGuis();
				Global::pPlayer->Stop();

				Global::_pSong->New();
				Global::_pSong->_pMachine[MASTER_INDEX]->_x = (CW - Global::psycleconf().macView().MachineCoords.sMaster.width) / 2;
				Global::_pSong->_pMachine[MASTER_INDEX]->_y = (CH - Global::psycleconf().macView().MachineCoords.sMaster.height) / 2;

				Global::pPlayer->SetBPM(Global::_pSong->BeatsPerMin(),Global::_pSong->LinesPerBeat());
				SetTitleBarText();
				editPosition=0;
				Global::_pSong->seqBus=0;
				pParentMain->PsybarsUpdate(); // Updates all values of the bars
				pParentMain->WaveEditorBackUpdate();
				pParentMain->m_wndInst.WaveUpdate();
				pParentMain->RedrawGearRackList();
				pParentMain->UpdateSequencer();
				pParentMain->UpdatePlayOrder(false); // should be done always after updatesequencer
				//pParentMain->UpdateComboIns(); PsybarsUpdate calls UpdateComboGen that always call updatecomboins
				RecalculateColourGrid();
				Repaint();
			}
			pParentMain->StatusBarIdle();
		}


		void CChildView::OnFileSaveaudio() 
		{
			OnBarstop();
			KillTimer(ID_TIMER_VIEW_REFRESH);
			KillTimer(ID_TIMER_AUTOSAVE);
			OnTimer(ID_TIMER_AUTOSAVE); // Autosave
			CSaveWavDlg dlg(this, &blockSel);
			dlg.DoModal();
			InitTimer();
		}

		///\todo that method does not take machine changes into account  
		//  <JosepMa> is this still the case? or what does "machine changes" mean?
		BOOL CChildView::CheckUnsavedSong(std::string szTitle)
		{
			if (Global::pInputHandler->IsModified()
				&& Global::psycleconf().bFileSaveReminders)
			{
				std::string filepath = Global::psycleconf().GetCurrentSongDir();
				filepath += '\\';
				filepath += Global::_pSong->fileName;
				OldPsyFile file;
				CProgressDialog progress;
				std::ostringstream szText;
				szText << "Save changes to \"" << Global::_pSong->fileName
					<< "\"?";
				int result = MessageBox(szText.str().c_str(),szTitle.c_str(),MB_YESNOCANCEL | MB_ICONEXCLAMATION);
				switch (result)
				{
				case IDYES:
					progress.SetWindowText("Saving...");
					progress.ShowWindow(SW_SHOW);
					if (!file.Create((char*)filepath.c_str(), true))
					{
						std::ostringstream szText;
						szText << "Error writing to \"" << filepath << "\"!!!";
						MessageBox(szText.str().c_str(),szTitle.c_str(),MB_ICONEXCLAMATION);
						return FALSE;
					}
					_pSong->Save(&file,progress);
					progress.SendMessage(WM_CLOSE);
					if (!file.Close())
					{
						std::ostringstream s;
						s << "Error writing to file '" << file.szName << "'" << std::endl;
						MessageBox(s.str().c_str(),"File Error!!!",0);
					}
					return TRUE;
					break;
				case IDNO:
					return TRUE;
					break;
				case IDCANCEL:
					return FALSE;
					break;
				}
			}
			return TRUE;
		}

		void CChildView::OnFileRevert()
		{
			if (MessageBox("Warning! You will lose all changes since song was last saved! Proceed?","Revert to Saved",MB_YESNO | MB_ICONEXCLAMATION) == IDYES)
			{
				if (Global::_pSong->_saved)
				{
					std::ostringstream fullpath;
					fullpath << Global::psycleconf().GetCurrentSongDir().c_str()
						<< '\\' << Global::_pSong->fileName.c_str();
					FileLoadsongNamed(fullpath.str());
				}
			}
			pParentMain->StatusBarIdle();
		}

		/// Tool bar buttons and View Commands
		void CChildView::OnMachineview() 
		{
			if (viewMode != view_modes::machine)
			{
				viewMode = view_modes::machine;
				ShowScrollBar(SB_BOTH,FALSE);

				// set midi input mode to real-time or Step
				if(Global::psycleconf().midi()._midiMachineViewSeqMode)
					Global::midi().m_midiMode = MODE_REALTIME;
				else
					Global::midi().m_midiMode = MODE_STEP;

				Repaint();
				pParentMain->StatusBarIdle();
			}
			SetFocus();
		}

		void CChildView::OnUpdateMachineview(CCmdUI* pCmdUI) 
		{
			if (viewMode==view_modes::machine)
				pCmdUI->SetCheck(1);
			else
				pCmdUI->SetCheck(0);
		}

		void CChildView::OnPatternView() 
		{
			if (viewMode != view_modes::pattern)
			{
				RecalcMetrics();

				viewMode = view_modes::pattern;
				//ShowScrollBar(SB_BOTH,FALSE);
				
				// set midi input mode to step insert
				Global::midi().m_midiMode = MODE_STEP;
				
				GetParent()->SetActiveWindow();

				if (Global::psycleconf()._followSong &&
					editPosition  != Global::pPlayer->_playPosition &&
					Global::pPlayer->_playing)
				{
					editPosition=Global::pPlayer->_playPosition;
				}
				Repaint();
				pParentMain->StatusBarIdle();
			}
			SetFocus();
		}

		void CChildView::OnUpdatePatternView(CCmdUI* pCmdUI) 
		{
			if(viewMode == view_modes::pattern)
				pCmdUI->SetCheck(1);
			else
				pCmdUI->SetCheck(0);
		}

		void CChildView::OnShowPatternSeq() 
		{
			/*
			if (viewMode != view_modes::sequence)
			{
				viewMode = view_modes::sequence;
				ShowScrollBar(SB_BOTH,FALSE);
				
				// set midi input mode to step insert
				Global::midi().m_midiMode = MODE_STEP;
				
				GetParent()->SetActiveWindow();
				Repaint();
				pParentMain->StatusBarIdle();
			}	
			*/
			SetFocus();
		}

		void CChildView::OnUpdatePatternSeq(CCmdUI* pCmdUI) 
		{
			if (viewMode==view_modes::sequence)
				pCmdUI->SetCheck(1);
			else
				pCmdUI->SetCheck(0);	
		}

		void CChildView::OnBarplay() 
		{
			if (Global::psycleconf()._followSong)
			{
				bScrollDetatch=false;
			}
			prevEditPosition=editPosition;
			Global::pPlayer->Start(editPosition,0);
			pParentMain->StatusBarIdle();
		}

		void CChildView::OnBarplayFromStart() 
		{
			if (Global::psycleconf()._followSong)
			{
				bScrollDetatch=false;
			}
			prevEditPosition=editPosition;
			Global::pPlayer->Start(0,0);
			pParentMain->StatusBarIdle();
		}

		void CChildView::OnUpdateBarplay(CCmdUI* pCmdUI) 
		{
			if (Global::pPlayer->_playing)
				pCmdUI->SetCheck(1);
			else
				pCmdUI->SetCheck(0);
		}

		void CChildView::OnUpdateBarplayFromStart(CCmdUI* pCmdUI) 
		{
			pCmdUI->SetCheck(0);
		}

		void CChildView::OnBarrec() 
		{
			if (Global::psycleconf()._followSong && bEditMode)
			{
				bEditMode = FALSE;
			}
			else
			{
				Global::psycleconf()._followSong = TRUE;
				bEditMode = TRUE;
				CButton*cb=(CButton*)pParentMain->m_seqBar.GetDlgItem(IDC_FOLLOW);
				cb->SetCheck(1);
			}
			pParentMain->StatusBarIdle();
		}

		void CChildView::OnUpdateBarrec(CCmdUI* pCmdUI) 
		{
			if (Global::psycleconf()._followSong && bEditMode)
				pCmdUI->SetCheck(1);
			else
				pCmdUI->SetCheck(0);
		}

		void CChildView::OnButtonplayseqblock() 
		{
			if (Global::psycleconf()._followSong)
			{
				bScrollDetatch=false;
			}

			prevEditPosition=editPosition;
			int i=0;
			while ( Global::_pSong->playOrderSel[i] == false ) i++;
			
			if(!Global::pPlayer->_playing)
				Global::pPlayer->Start(i,0);

			Global::pPlayer->_playBlock=!Global::pPlayer->_playBlock;

			pParentMain->StatusBarIdle();
			if ( viewMode == view_modes::pattern ) Repaint(draw_modes::pattern);
		}

		void CChildView::OnUpdateButtonplayseqblock(CCmdUI* pCmdUI) 
		{
			if ( Global::pPlayer->_playBlock == true ) pCmdUI->SetCheck(TRUE);
			else pCmdUI->SetCheck(FALSE);
		}

		void CChildView::OnBarstop()
		{
			bool pl = Global::pPlayer->_playing;
			bool blk = Global::pPlayer->_playBlock;
			Global::pPlayer->Stop();
			pParentMain->SetAppSongBpm(0);
			pParentMain->SetAppSongTpb(0);

			if (pl)
			{
				if ( Global::psycleconf()._followSong && blk)
				{
					editPosition=prevEditPosition;
					pParentMain->UpdatePlayOrder(false); // <- This restores the selected block
					Repaint(draw_modes::pattern);
				}
				else
				{
					memset(Global::_pSong->playOrderSel,0,MAX_SONG_POSITIONS*sizeof(bool));
					Global::_pSong->playOrderSel[editPosition] = true;
					Repaint(draw_modes::cursor); 
				}
			}
		}

		void CChildView::OnRecordWav() 
		{
			if (!Global::pPlayer->_recording)
			{
				static char BASED_CODE szFilter[] = "Wav Files (*.wav)|*.wav|All Files (*.*)|*.*||";
				
				CFileDialog dlg(false,"wav",NULL,OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,szFilter);
				if ( dlg.DoModal() == IDOK ) 
				{
					Global::pPlayer->StartRecording(static_cast<LPCTSTR>(dlg.GetPathName()));
				}
				//If autoStopMachine is activated, deactivate it while recording
				if ( Global::psycleconf().UsesAutoStopMachines() ) 
				{
					OnAutostop();
				}
			}
			else
			{
				Global::pPlayer->StopRecording();
			}
		}

		void CChildView::OnUpdateRecordWav(CCmdUI* pCmdUI) 
		{
			if (Global::pPlayer->_recording)
			{
				pCmdUI->SetCheck(1);
			}
			else
			{
				pCmdUI->SetCheck(0);
			}
		}

		void CChildView::OnAutostop() 
		{
			if ( Global::psycleconf().UsesAutoStopMachines() )
			{
				Global::psycleconf().UseAutoStopMachines(false);
				for (int c=0; c<MAX_MACHINES; c++)
				{
					if (Global::_pSong->_pMachine[c])
					{
						Global::_pSong->_pMachine[c]->Standby(false);
					}
				}
			}
			else Global::psycleconf().UseAutoStopMachines(true);
		}

		void CChildView::OnUpdateAutostop(CCmdUI* pCmdUI) 
		{
			if (Global::psycleconf().UsesAutoStopMachines() ) pCmdUI->SetCheck(TRUE);
			else pCmdUI->SetCheck(FALSE);
		}

		void CChildView::OnFileSongproperties() 
		{	CSongpDlg dlg(Global::_pSong);
			dlg.DoModal();
			pParentMain->StatusBarIdle();
			//Repaint();
		}

		void CChildView::OnViewInstrumenteditor()
		{
			pParentMain->ShowInstrumentEditor();
		}


		/// Show the CPU Performance dialog
		void CChildView::OnHelpPsycleenviromentinfo() 
		{
			pParentMain->ShowPerformanceDlg();
		}

		/// Show the MIDI monitor dialog
		void CChildView::OnMidiMonitorDlg() 
		{
			pParentMain->ShowMidiMonitorDlg();
		}

		void CChildView::ShowTransformPatternDlg(void)
		{
			CTransformPatternDlg dlg(_pSong);

			if (dlg.DoModal() == IDOK)
			{
				///\todo: Implement.
			}
		}

		void CChildView::ShowPatternDlg(void)
		{
			CPatDlg dlg;
			int patNum = _pSong->playOrder[editPosition];
			int nlines = _pSong->patternLines[patNum];
			char name[32];
			strcpy(name,_pSong->patternName[patNum]);

			dlg.patLines= nlines;
			strcpy(dlg.patName,name);
			dlg.patIdx = patNum;
			dlg.m_pSong = _pSong;
			dlg.m_shownames = Global::psycleconf().patView().showTrackNames_?1:0;
			dlg.m_independentnames = _pSong->shareTrackNames?0:1;

			pParentMain->UpdateSequencer();
			
			if (dlg.DoModal() == IDOK)
			{
				if ( nlines != dlg.patLines )
				{
					Global::pInputHandler->AddUndo(patNum,0,0,MAX_TRACKS,nlines,editcur.track,editcur.line,editcur.col,editPosition);
					Global::pInputHandler->AddUndoLength(patNum,nlines,editcur.track,editcur.line,editcur.col,editPosition);
					_pSong->AllocNewPattern(patNum,dlg.patName,dlg.patLines,dlg.m_adaptsize?true:false);
					if ( strcmp(name,dlg.patName) != 0 )
					{
						strcpy(_pSong->patternName[patNum],dlg.patName);
						pParentMain->StatusBarIdle();
					}
					Repaint();
				}
				else if ( strcmp(name,dlg.patName) != 0 )
				{
					strcpy(_pSong->patternName[patNum],dlg.patName);
					pParentMain->UpdateSequencer();
					pParentMain->StatusBarIdle();
					//Repaint(draw_modes::patternHeader);
				}
			}
		}

		void CChildView::OnNewmachine() 
		{
			NewMachine();
		}

		/// Show new machine dialog
		void CChildView::NewMachine(int x, int y, int mac) 
		{
			CNewMachine dlg(this);
			if(mac >= 0)
			{
				if (mac < MAX_BUSES)
				{
					dlg.selectedMode = modegen;
				}
				else
				{
					dlg.selectedMode = modefx;
				}
			}
			if ((dlg.DoModal() == IDOK) && (dlg.Outputmachine >= 0))
			{
				int fb,xs,ys;
				if (mac < 0)
				{
					Global::pInputHandler->AddMacViewUndo();
					if (dlg.selectedMode == modegen) 
					{
						fb = Global::_pSong->GetFreeBus();
						xs = MachineCoords->sGenerator.width;
						ys = MachineCoords->sGenerator.height;
					}
					else 
					{
						fb = Global::_pSong->GetFreeFxBus();
						xs = MachineCoords->sEffect.width;
						ys = MachineCoords->sEffect.height;
					}
				}
				else
				{
					if (mac >= MAX_BUSES && dlg.selectedMode != modegen)
					{
						Global::pInputHandler->AddMacViewUndo();
						fb = mac;
						xs = MachineCoords->sEffect.width;
						ys = MachineCoords->sEffect.height;
					}
					else if (mac < MAX_BUSES && dlg.selectedMode == modegen)
					{
						Global::pInputHandler->AddMacViewUndo();
						fb = mac;
						xs = MachineCoords->sGenerator.width;
						ys = MachineCoords->sGenerator.height;
					}
					else
					{
						MessageBox("Wrong Class of Machine!");
						return;
					}
				}
				// Get info of old machine and close any open gui.
				if (Global::_pSong->_pMachine[fb])
				{
					x = Global::_pSong->_pMachine[fb]->_x;
					y = Global::_pSong->_pMachine[fb]->_y;
					pParentMain->CloseMacGui(fb);
				}
				else if ((x < 0) || (y < 0))
				{
					 // random position
					bool bCovered = TRUE;
					while (bCovered)
					{
						x = (rand())%(CW-xs);
						y = (rand())%(CH-ys);
						bCovered = FALSE;
						for (int i=0; i < MAX_MACHINES; i++)
						{
							if (Global::_pSong->_pMachine[i])
							{
								if ((abs(Global::_pSong->_pMachine[i]->_x - x) < 32) &&
									(abs(Global::_pSong->_pMachine[i]->_y - y) < 32))
								{
									bCovered = TRUE;
									i = MAX_MACHINES;
								}
							}
						}
					}
				}

				if ( fb == -1)
				{
					MessageBox("Machine Creation Failed","Error!",MB_OK);
				}
				else
				{
					bool created=false;
					if (Global::_pSong->_pMachine[fb] )
					{
						created = Global::_pSong->ReplaceMachine(Global::_pSong->_pMachine[fb],(MachineType)dlg.Outputmachine, x, y, dlg.psOutputDll.c_str(),fb,dlg.shellIdx);
					}
					else 
					{
						created = Global::_pSong->CreateMachine((MachineType)dlg.Outputmachine, x, y, dlg.psOutputDll.c_str(),fb,dlg.shellIdx);
					}
					if (created)
					{
						if ( dlg.selectedMode == modegen)
						{
							Global::_pSong->seqBus = fb;
						}

						// make sure that no 2 machines have the same name, because that is irritating

						int number = 1;
						char buf[sizeof(_pSong->_pMachine[fb]->_editName)+4];
						strcpy (buf,_pSong->_pMachine[fb]->_editName);

						for (int i = 0; i < MAX_MACHINES-1; i++)
						{
							if (i!=fb)
							{
								if (_pSong->_pMachine[i])
								{
									if (strcmp(_pSong->_pMachine[i]->_editName,buf)==0)
									{
										number++;
										sprintf(buf,"%s %d",_pSong->_pMachine[fb]->_editName,number);
										i = -1;
									}
								}
							}
						}

						buf[sizeof(_pSong->_pMachine[fb]->_editName)-1] = 0;
						strcpy(_pSong->_pMachine[fb]->_editName,buf);

						pParentMain->UpdateComboGen();
						Repaint(draw_modes::all);
						//Repaint(draw_modes::all_machines); // Seems that this doesn't always work (multiple calls to Repaint?)
					}
					else MessageBox("Machine Creation Failed","Error!",MB_OK);
				}
			}
			//Repaint();
			pParentMain->RedrawGearRackList();
		}

		void CChildView::OnConfigurationSettings() 
		{
			CConfigDlg dlg("Psycle Settings");
			if (dlg.DoModal() == IDOK)
			{
				Global::psycleconf().RefreshSettings();
				RecalculateColourGrid();
				RecalcMetrics();
				InitTimer();
			}
			Repaint();
		}

		void CChildView::OnHelpSaludos() 
		{
			CGreetDialog dlg;
			dlg.DoModal();
		}

		///\todo extemely toxic pollution
		#define TWOPI_F (2.0f*3.141592665f)

		void CChildView::ShowSwingFillDlg(bool bTrackMode)
		{
			int st = Global::_pSong->BeatsPerMin();
			static int sw = 2;
			static float sv = 13.0f;
			static float sp = -90.0f;
			static BOOL of = true;
			CSwingFillDlg dlg;
			dlg.tempo = st;
			dlg.width = sw;
			dlg.variance = sv;
			dlg.phase = sp;
			dlg.offset = true;

			dlg.DoModal();
			if (dlg.bGo)
			{
				st = dlg.tempo;
				sw = dlg.width;
				sv = dlg.variance;
				sp = dlg.phase;
				of = dlg.offset;
				float var = (sv/100.0f);

				// time to do our fill
				// first some math
				// our range has to go from spd+var to spd-var and back in width+1 lines
				float step = TWOPI_F/(sw);
				float index = sp*TWOPI_F/360;

				int l;
				int x;
				int y;
				int ny;
				if (bTrackMode)
				{
					x = editcur.track;
					y = 0;
					ny = _pSong->patternLines[_ps()];
				}
				else
				{
					x = blockSel.start.track;
					y = blockSel.start.line;
					ny = 1+blockSel.end.line-blockSel.start.line;
				}

				// remember we are at each speed for the length of time it takes to do one tick
				// this approximately calculates the offset
				float dcoffs = 0;
				if (of)
				{
					float swing=0;
					for (l=0;l<sw;l++)
					{
						float val = ((sinf(index)*var*st)+st);
						swing += (val/st)*(val/st);
						index+=step;
					}
					dcoffs = ((swing-sw)*st)/sw;
				}

				// now fill the pattern
				unsigned char *base = _ppattern();
				if (base)
				{
					Global::pInputHandler->AddUndo(_ps(),x,y,1,ny,editcur.track,editcur.line,editcur.col,editPosition);
					for (l=y;l<y+ny;l++)
					{
						int const displace=x*EVENT_SIZE+l*MULTIPLY;
						
						unsigned char *offset=base+displace;
						
						PatternEntry *entry = (PatternEntry*) offset;
						entry->_cmd = 0xff;
						int val = helpers::math::lround<int,float>(((sinf(index)*var*st)+st)+dcoffs);//-0x20; // ***** proposed change to ffxx command to allow more useable range since the tempo bar only uses this range anyway...
						if (val < 1)
						{
							val = 1;
						}
						else if (val > 255)
						{
							val = 255;
						}
						entry->_parameter = unsigned char (val);
						index+=step;
					}
					NewPatternDraw(x,x,y,y+ny);	
					Repaint(draw_modes::data);
				}
			}
		}

		//// Right Click Popup Menu
		void CChildView::OnPopCut() { CopyBlock(true); }

		void CChildView::OnUpdateCutCopy(CCmdUI* pCmdUI) 
		{
			if (blockSelected && (viewMode == view_modes::pattern)) pCmdUI->Enable(TRUE);
			else pCmdUI->Enable(FALSE);
		}

		void CChildView::OnPopCopy() { CopyBlock(false); }

		void CChildView::OnPopPaste() { PasteBlock(editcur.track,editcur.line,false); }
		void CChildView::OnUpdatePaste(CCmdUI* pCmdUI) 
		{
			if (isBlockCopied && (viewMode == view_modes::pattern)) pCmdUI->Enable(TRUE);
			else  pCmdUI->Enable(FALSE);
		}

		void CChildView::OnPopMixpaste() { PasteBlock(editcur.track,editcur.line,true); }

		void CChildView::OnPopBlockswitch()
		{
			SwitchBlock(editcur.track,editcur.line);
		}

		void CChildView::OnUpdatePopBlockswitch(CCmdUI *pCmdUI)
		{
			if (isBlockCopied && (viewMode == view_modes::pattern)) pCmdUI->Enable(true);
			else  pCmdUI->Enable(false);
		}

		void CChildView::OnPopDelete() { DeleteBlock(); }
		
		void CChildView::OnPopAddNewTrack() 
		{
			int pattern = _pSong->playOrder[editPosition];
			if (MessageBox("Do you want to add the track to all patterns?","Add pattern track",MB_YESNO) == IDYES )
			{
				pattern = -1;
			}

			_pSong->AddNewTrack(pattern, editcur.track);
			pParentMain->PsybarsUpdate();
			RecalculateColourGrid();
			Repaint();
		}


		void CChildView::OnPopInterpolate() { BlockParamInterpolate(); }

		void CChildView::OnPopInterpolateCurve()
		{
			CInterpolateCurve dlg(blockSel.start.line,blockSel.end.line,_pSong->LinesPerBeat());
			
			int* valuearray = new int[blockSel.end.line-blockSel.start.line+1];
			int ps=_pSong->playOrder[editPosition];
			unsigned char notecommand = notecommands::empty;
			unsigned char targetmac = 255;
			unsigned char targettwk = 255;
			for (int i=0; i<=blockSel.end.line-blockSel.start.line; i++)
			{
				unsigned char *offset_target=_ptrackline(ps,blockSel.start.track,i+blockSel.start.line);
				if (*offset_target <= notecommands::release || *offset_target == notecommands::empty)
				{
					if ( *(offset_target+3) == 0 && *(offset_target+4) == 0 ) valuearray[i]=-1;
					else {
						targettwk = *(offset_target+3);
						valuearray[i]= *(offset_target+3)*0x100 + *(offset_target+4);
					}
				}
				else {
					notecommand = *offset_target;
					targetmac = *(offset_target+2);
					targettwk = *(offset_target+1);
					valuearray[i] = *(offset_target+3)*0x100 + *(offset_target+4);
				}
			}
			if ( notecommand == notecommands::tweak ) {
				int min=0, max=0xFFFF;
				if(_pSong->_pMachine[targetmac] != NULL) {
					_pSong->_pMachine[targetmac]->GetParamRange(targettwk,min,max);
				}
				//If the parameter uses negative number, the values are shifted up.
				max-=min;
				max&=0xFFFF;
				min=0;
				dlg.AssignInitialValues(valuearray,0,min,max);
			}
			else if ( notecommand == notecommands::tweakslide ) {
				int min=0, max=0xFFFF;
				if(_pSong->_pMachine[targetmac] != NULL) {
					_pSong->_pMachine[targetmac]->GetParamRange(targettwk,min,max);
				}
				//If the parameter uses negative number, the values are shifted up.
				max-=min;
				max&=0xFFFF;
				min=0;
				dlg.AssignInitialValues(valuearray,1,min,max);
			}
			else if ( notecommand == notecommands::midicc ) dlg.AssignInitialValues(valuearray,2,0,0x7F);
			else {
				dlg.AssignInitialValues(valuearray,-1, targettwk*0x100,targettwk*0x100 +0xFF);
			}

			if (dlg.DoModal() == IDOK )
			{
				int twktype(255);
				if ( dlg.kftwk == 0 ) twktype = notecommands::tweak;
				else if ( dlg.kftwk == 1 ) twktype = notecommands::tweakslide;
				else if ( dlg.kftwk == 2 ) twktype = notecommands::midicc;
				BlockParamInterpolate(dlg.kfresult,twktype);
			}
			delete[] valuearray;
		}


		void CChildView::OnPopChangegenerator() { BlockGenChange(_pSong->seqBus); }

		void CChildView::OnPopChangeinstrument() { BlockInsChange(_pSong->auxcolSelected); }

		void CChildView::OnPopTranspose1() { BlockTranspose(1); }

		void CChildView::OnPopTranspose12() { BlockTranspose(12); }

		void CChildView::OnPopTranspose_1() { BlockTranspose(-1); }

		void CChildView::OnPopTranspose_12() { BlockTranspose(-12); }

		void CChildView::OnPopTransformpattern() 
		{
			ShowTransformPatternDlg();			
		}

		void CChildView::OnPopPattenproperties() 
		{
			ShowPatternDlg();
		}

		/// fill block
		void CChildView::OnPopBlockSwingfill()
		{
			ShowSwingFillDlg(FALSE);
		}

		/// fill track
		void CChildView::OnPopTrackSwingfill()
		{
			ShowSwingFillDlg(TRUE);
		}

		void CChildView::OnUpdateUndo(CCmdUI* pCmdUI)
		{
			if(Global::pInputHandler->HasUndo(viewMode))
			{
				pCmdUI->Enable(TRUE);
			}
			else
			{
				pCmdUI->Enable(FALSE);
			}
		}

		void CChildView::OnUpdateRedo(CCmdUI* pCmdUI)
		{
			if(Global::pInputHandler->HasRedo(viewMode))
			{
				pCmdUI->Enable(TRUE);
			}
			else
			{
				pCmdUI->Enable(FALSE);
			}
		}

		void CChildView::OnUpdatePatternCutCopy(CCmdUI* pCmdUI) 
		{
			if(viewMode == view_modes::pattern) pCmdUI->Enable(TRUE);
			else pCmdUI->Enable(FALSE);
		}

		void CChildView::OnUpdatePatternPaste(CCmdUI* pCmdUI) 
		{
			if(patBufferCopy&&(viewMode == view_modes::pattern)) pCmdUI->Enable(TRUE);
			else pCmdUI->Enable(FALSE);
		}

		void CChildView::OnFileImportModulefile() 
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
			ofn.lpstrFilter =
				"All Module Songs (*.xm *.it *.s3m *.mod)" "\0" "*.xm;*.it;*.s3m;*.mod" "\0"
				"FastTracker II Songs (*.xm)"              "\0" "*.xm"                  "\0"
				"Impulse Tracker Songs (*.it)"             "\0" "*.it"                  "\0"
				"Scream Tracker Songs (*.s3m)"             "\0" "*.s3m"                 "\0"
				"Original Mod Format Songs (*.mod)"        "\0" "*.mod"                 "\0"
				"All (*)"                                  "\0" "*"                     "\0"
				;
			ofn.nFilterIndex = 1;
			ofn.lpstrFileTitle = NULL;
			ofn.nMaxFileTitle = 0;
			std::string tmpstr = Global::psycleconf().GetCurrentSongDir();
			ofn.lpstrInitialDir = tmpstr.c_str();
			ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
			// Display the Open dialog box. 
			if (GetOpenFileName(&ofn)==TRUE)
			{
				Global::pInputHandler->KillUndo();
				Global::pInputHandler->KillRedo();
				pParentMain->CloseAllMacGuis();
				Global::pPlayer->Stop();

				CString str = ofn.lpstrFile;
				int index = str.ReverseFind('.');
				if (index != -1)
				{
					CString ext = str.Mid(index+1);
					if (ext.CompareNoCase("XM") == 0)
					{
						XMSongLoader xmfile;
						xmfile.Open(ofn.lpstrFile);
						Global::_pSong->New();
						editPosition=0;
						xmfile.Load(*_pSong);
						xmfile.Close();
						if (Global::psycleconf().bShowSongInfoOnLoad)
						{
							CSongpDlg dlg(Global::_pSong);
							dlg.SetReadOnly();
							dlg.DoModal();
						}
					} else if (ext.CompareNoCase("IT") == 0)
					{
						ITModule2 it;
						it.Open(ofn.lpstrFile);
						Global::_pSong->New();
						editPosition=0;
						if(!it.LoadITModule(_pSong))
						{			
							MessageBox("Load failed");
							Global::_pSong->New();
							it.Close();
							return;
						}
						it.Close();
						if (Global::psycleconf().bShowSongInfoOnLoad)
						{
							CSongpDlg dlg(Global::_pSong);
							dlg.SetReadOnly();
							dlg.DoModal();
						}
					} else if (ext.CompareNoCase("S3M") == 0)
					{
						ITModule2 s3m;
						s3m.Open(ofn.lpstrFile);
						Global::_pSong->New();
						editPosition=0;
						if(!s3m.LoadS3MModuleX(_pSong))
						{			
							MessageBox("Load failed");
							Global::_pSong->New();
							s3m.Close();
							return;
						}
						s3m.Close();
						if (Global::psycleconf().bShowSongInfoOnLoad)
						{
							CSongpDlg dlg(Global::_pSong);
							dlg.SetReadOnly();
							dlg.DoModal();
						}
					} else if (ext.CompareNoCase("MOD") == 0)
					{
						MODSongLoader modfile;
						modfile.Open(ofn.lpstrFile);
						Global::_pSong->New();
						editPosition=0;
						modfile.Load(*_pSong);
						modfile.Close();
						if (Global::psycleconf().bShowSongInfoOnLoad)
						{
							CSongpDlg dlg(Global::_pSong);
							dlg.SetReadOnly();
							dlg.DoModal();
						}
					}
				}

				str = ofn.lpstrFile;
				index = str.ReverseFind('\\');
				if (index != -1)
				{
					Global::psycleconf().SetCurrentSongDir((LPCSTR)str.Left(index));
					Global::_pSong->fileName = str.Mid(index+1)+".psy";
				}
				else
				{
					Global::_pSong->fileName = str+".psy";
				}
				Global::_pSong->_pMachine[MASTER_INDEX]->_x = (CW - Global::psycleconf().macView().MachineCoords.sMaster.width) / 2;
				Global::_pSong->_pMachine[MASTER_INDEX]->_y = (CH - Global::psycleconf().macView().MachineCoords.sMaster.height) / 2;
				pParentMain->PsybarsUpdate();
				pParentMain->WaveEditorBackUpdate();
				pParentMain->m_wndInst.WaveUpdate();
				pParentMain->RedrawGearRackList();
				pParentMain->UpdateSequencer();
				pParentMain->UpdatePlayOrder(false);
				RecalculateColourGrid();
				Repaint();
			}
			SetTitleBarText();
		}

		void CChildView::AppendToRecent(std::string const& fName)
		{
			int iCount;
			char* nameBuff;
			UINT nameSize;
			UINT ids[] =
				{
					ID_FILE_RECENT_01,
					ID_FILE_RECENT_02,
					ID_FILE_RECENT_03,
					ID_FILE_RECENT_04
				};
			// Remove initial empty element, if present.
			if(GetMenuItemID(hRecentMenu, 0) == ID_FILE_RECENT_NONE)
			{
				DeleteMenu(hRecentMenu, 0, MF_BYPOSITION);
			}
			// Check for duplicates and eventually remove.
			for(iCount = GetMenuItemCount(hRecentMenu)-1; iCount >=0; iCount--)
			{
				nameSize = GetMenuString(hRecentMenu, iCount, 0, 0, MF_BYPOSITION) + 1;
				nameBuff = new char[nameSize];
				GetMenuString(hRecentMenu, iCount, nameBuff, nameSize, MF_BYPOSITION);
				if ( !strcmp(nameBuff, fName.c_str()) )
				{
					DeleteMenu(hRecentMenu, iCount, MF_BYPOSITION);
				}
				delete[] nameBuff;
			}
			// Ensure menu size doesn't exceed 4 positions.
			if (GetMenuItemCount(hRecentMenu) == 4)
			{
				DeleteMenu(hRecentMenu, 4-1, MF_BYPOSITION);
			}
			::MENUITEMINFO hNewItemInfo, hTempItemInfo;
			hNewItemInfo.cbSize		= sizeof(MENUITEMINFO);
			hNewItemInfo.fMask		= MIIM_ID | MIIM_TYPE;
			hNewItemInfo.fType		= MFT_STRING;
			hNewItemInfo.wID		= ids[0];
			hNewItemInfo.cch		= (UINT)fName.length();
			hNewItemInfo.dwTypeData = (LPSTR)fName.c_str();
			InsertMenuItem(hRecentMenu, 0, TRUE, &hNewItemInfo);
			// Update identifiers.
			for(iCount = 1;iCount < GetMenuItemCount(hRecentMenu);iCount++)
			{
				hTempItemInfo.cbSize	= sizeof(MENUITEMINFO);
				hTempItemInfo.fMask		= MIIM_ID;
				hTempItemInfo.wID		= ids[iCount];
				SetMenuItemInfo(hRecentMenu, iCount, true, &hTempItemInfo);
			}
			Global::psycleconf().AddRecentFile(fName);
		}
		void CChildView::RestoreRecent()
		{
			const std::vector<std::string> recent = Global::psycleconf().GetRecentFiles();
			UINT ids[] =
				{
					ID_FILE_RECENT_01,
					ID_FILE_RECENT_02,
					ID_FILE_RECENT_03,
					ID_FILE_RECENT_04
				};
			for(int iCount = GetMenuItemCount(hRecentMenu)-1; iCount>=0;iCount--)
			{
				DeleteMenu(hRecentMenu, iCount, MF_BYPOSITION);
			}
			for(int iCount = recent.size()-1; iCount>= 0;iCount--)
			{
				::MENUITEMINFO hNewItemInfo;
				hNewItemInfo.cbSize		= sizeof(MENUITEMINFO);
				hNewItemInfo.fMask		= MIIM_ID | MIIM_TYPE;
				hNewItemInfo.fType		= MFT_STRING;
				hNewItemInfo.wID		= ids[iCount];
				hNewItemInfo.cch		= (UINT)recent[iCount].length();
				hNewItemInfo.dwTypeData = (LPSTR)recent[iCount].c_str();
				InsertMenuItem(hRecentMenu, 0, TRUE, &hNewItemInfo);
			}
		}

		void CChildView::OnFileRecent_01()
		{
			CallOpenRecent(0);
		}

		void CChildView::OnFileRecent_02()
		{
			CallOpenRecent(1);
		}

		void CChildView::OnFileRecent_03()
		{
			CallOpenRecent(2);
		}

		void CChildView::OnFileRecent_04()
		{
			CallOpenRecent(3);
		}

		void CChildView::OnFileLoadsongNamed(std::string fName, int fType)
		{
			if( fType == 2 )
			{
				FILE* hFile=fopen(fName.c_str(),"rb");
				LoadBlock(hFile);
				fclose(hFile);
			}
			else
			{
				if (CheckUnsavedSong("Load Song"))
				{
					FileLoadsongNamed(fName);
				}
			}
		}

		void CChildView::FileLoadsongNamed(std::string fName)
		{
			pParentMain->CloseAllMacGuis();
			Global::pPlayer->Stop();
			
			OldPsyFile file;
			if (!file.Open(fName.c_str()))
			{
				MessageBox("Could not Open file. Check that the location is correct.", "Loading Error", MB_OK);
				return;
			}
			editPosition = 0;
			CProgressDialog progress;
			progress.ShowWindow(SW_SHOW);
			if(!_pSong->Load(&file,progress) || !file.Close())
			{
				std::ostringstream s;
				s << "Error reading from file '" << file.szName << "'" << std::endl;
				MessageBox(s.str().c_str(), "File Error!!!", 0);
			}
			progress.SendMessage(WM_CLOSE);
			_pSong->_saved=true;
			AppendToRecent(fName);
			std::string::size_type index = fName.rfind('\\');
			if (index != std::string::npos)
			{
				Global::psycleconf().SetCurrentSongDir(fName.substr(0,index));
				Global::_pSong->fileName = fName.substr(index+1);
			}
			else
			{
				Global::_pSong->fileName = fName;
			}
			Global::pPlayer->SetBPM(Global::_pSong->BeatsPerMin(), Global::_pSong->LinesPerBeat());
			EnforceAllMachinesOnView();
			pParentMain->PsybarsUpdate();
			pParentMain->WaveEditorBackUpdate();
			pParentMain->m_wndInst.WaveUpdate();
			pParentMain->RedrawGearRackList();
			pParentMain->UpdateSequencer();
			pParentMain->UpdatePlayOrder(false);
			//pParentMain->UpdateComboIns(); PsyBarsUpdate calls UpdateComboGen that also calls UpdatecomboIns
			RecalculateColourGrid();
			Repaint();
			Global::pInputHandler->KillUndo();
			Global::pInputHandler->KillRedo();
			SetTitleBarText();
			if (Global::psycleconf().bShowSongInfoOnLoad)
			{
				CSongpDlg dlg(Global::_pSong);
				dlg.SetReadOnly();
				dlg.DoModal();
			}
		}

		void CChildView::CallOpenRecent(int pos)
		{
			UINT nameSize;
			nameSize = GetMenuString(hRecentMenu, pos, 0, 0, MF_BYPOSITION) + 1;
			char* nameBuff = new char[nameSize];
			GetMenuString(hRecentMenu, pos, nameBuff, nameSize, MF_BYPOSITION);
			OnFileLoadsongNamed(nameBuff, 1);
			delete[] nameBuff; nameBuff = 0;
		}

		void CChildView::SetTitleBarText()
		{
			std::string titlename = "[";
			titlename+=Global::_pSong->fileName;
			if(Global::pInputHandler->IsModified())
			{
				titlename+=" *";
			}
			// don't know how to access to the IDR_MAINFRAME String Title.
			titlename += "] Psycle Modular Music Creation Studio (" PSYCLE__VERSION ")";
			pParentMain->SetWindowText(titlename.c_str());
		}

		void CChildView::OnHelpKeybtxt() 
		{
			char path[MAX_PATH];
			sprintf(path,"%sdocs\\keys.txt",Global::psycleconf().appPath().c_str());
			ShellExecute(pParentMain->m_hWnd,"open",path,NULL,"",SW_SHOW);
		}

		void CChildView::OnHelpReadme() 
		{
			char path[MAX_PATH];
			sprintf(path,"%sdocs\\readme.txt",Global::psycleconf().appPath().c_str());
			ShellExecute(pParentMain->m_hWnd,"open",path,NULL,"",SW_SHOW);
		}

		void CChildView::OnHelpTweaking() 
		{
			char path[MAX_PATH];
			sprintf(path,"%sdocs\\tweaking.txt",Global::psycleconf().appPath().c_str());
			ShellExecute(pParentMain->m_hWnd,"open",path,NULL,"",SW_SHOW);
		}

		void CChildView::OnHelpWhatsnew() 
		{
			char path[MAX_PATH];
			sprintf(path,"%sdocs\\whatsnew.txt",Global::psycleconf().appPath().c_str());
			ShellExecute(pParentMain->m_hWnd,"open",path,NULL,"",SW_SHOW);
		}

		void CChildView::EnforceAllMachinesOnView()
		{
			SMachineCoords mcoords = Global::psycleconf().macView().MachineCoords;
			for(int i(0);i<MAX_MACHINES;i++)
			{
				if(_pSong->_pMachine[i])
				{
					int x = _pSong->_pMachine[i]->_x;
					int y = _pSong->_pMachine[i]->_y;
					switch (_pSong->_pMachine[i]->_mode)
					{
					case MACHMODE_GENERATOR:
						if ( x > CW-mcoords.sGenerator.width ) 
						{
							x = CW-mcoords.sGenerator.width;
						}
						if ( y > CH-mcoords.sGenerator.height ) 
						{
							y = CH-mcoords.sGenerator.height;
						}
						break;
					case MACHMODE_FX:
						if ( x > CW-mcoords.sEffect.width )
						{
							x = CW-mcoords.sEffect.width;
						}
						if ( y > CH-mcoords.sEffect.height ) 
						{
							y = CH-mcoords.sEffect.height;
						}
						break;

					case MACHMODE_MASTER:
						if ( x > CW-mcoords.sMaster.width ) 
						{
							x = CW-mcoords.sMaster.width;
						}
						if ( y > CH-mcoords.sMaster.height )
						{
							y = CH-mcoords.sMaster.height;
						}
						break;
					}
					_pSong->_pMachine[i]->_x = x;
					_pSong->_pMachine[i]->_y = y;
				}
			}
		}

		void CChildView::RecalcMetrics()
		{
			if (patView->draw_empty_data)
			{
				strcpy(szBlankParam,".");
				notes_tab_a220[notecommands::empty]="---";
				notes_tab_a440[notecommands::empty]="---";
			}
			else
			{
				strcpy(szBlankParam," ");
				notes_tab_a220[notecommands::empty]="   ";
				notes_tab_a440[notecommands::empty]="   ";
			}
			note_tab_selected = patView->showA440 ? notes_tab_a440 : notes_tab_a220;

			TEXTHEIGHT = patView->font_y;
			ROWHEIGHT = TEXTHEIGHT+1;
			TEXTWIDTH = patView->font_x;
			for (int c=0; c<256; c++)	
			{ 
				FLATSIZES[c]=patView->font_x; 
			}
			COLX[0] = 0;
			COLX[1] = (TEXTWIDTH*3)+2;
			COLX[2] = COLX[1]+TEXTWIDTH;
			COLX[3] = COLX[2]+TEXTWIDTH+1;
			COLX[4] = COLX[3]+TEXTWIDTH;
			COLX[5] = COLX[4]+TEXTWIDTH+1;
			COLX[6] = COLX[5]+TEXTWIDTH;
			COLX[7] = COLX[6]+TEXTWIDTH;
			COLX[8] = COLX[7]+TEXTWIDTH;
			COLX[9] = COLX[8]+TEXTWIDTH+1;
			ROWWIDTH = COLX[9];
			HEADER_ROWWIDTH = PatHeaderCoords->sBackground.width+1;
			HEADER_HEIGHT = PatHeaderCoords->sBackground.height+2;
			if (ROWWIDTH < HEADER_ROWWIDTH)
			{
				int temp = (HEADER_ROWWIDTH-ROWWIDTH)/2;
				ROWWIDTH = HEADER_ROWWIDTH;
				for (int i = 0; i < 10; i++)
				{
					COLX[i] += temp;
				}
			}
			HEADER_INDENT = (ROWWIDTH - HEADER_ROWWIDTH)/2;
			if (patView->_linenumbers)
			{
				XOFFSET = (4*TEXTWIDTH);
				YOFFSET = TEXTHEIGHT+2;
				if (YOFFSET < HEADER_HEIGHT)
				{
					YOFFSET = HEADER_HEIGHT;
				}
			}
			else
			{
				XOFFSET = 1;
				YOFFSET = HEADER_HEIGHT;
			}
			VISTRACKS = (CW-XOFFSET)/ROWWIDTH;
			VISLINES = (CH-YOFFSET)/ROWHEIGHT;
			if (VISLINES < 1) 
			{ 
				VISLINES = 1; 
			}
			if (VISTRACKS < 1) 
			{ 
				VISTRACKS = 1; 
			}
			triangle_size_tall = macView->triangle_size+((23*macView->wirewidth)/16);

			triangle_size_center = triangle_size_tall/2;
			triangle_size_wide = triangle_size_tall/2;
			triangle_size_indent = triangle_size_tall/6;
		}


		void CChildView::TransparentBlt
			(
				CDC* pDC,
				int xStart,  int yStart,
				int wWidth,  int wHeight,
				CDC* pTmpDC,
				CBitmap* bmpMask,
				int xSource, // = 0
				int ySource // = 0
			)
		{
			// We are going to paint the two DDB's in sequence to the destination.
			// 1st the monochrome bitmap will be blitted using an AND operation to
			// cut a hole in the destination. The color image will then be ORed
			// with the destination, filling it into the hole, but leaving the
			// surrounding area untouched.
			CDC hdcMem;
			hdcMem.CreateCompatibleDC(pDC);
			CBitmap* hbmT = hdcMem.SelectObject(bmpMask);
			pDC->SetTextColor(RGB(0,0,0));
			pDC->SetBkColor(RGB(255,255,255));
			if (!pDC->BitBlt( xStart, yStart, wWidth, wHeight, &hdcMem, xSource, ySource, 
				SRCAND))
			{
				TRACE("Transparent Blit failure SRCAND");
			}
			// Also note the use of SRCPAINT rather than SRCCOPY.
			if (!pDC->BitBlt(xStart, yStart, wWidth, wHeight, pTmpDC, xSource, ySource,
				SRCPAINT))
			{
				TRACE("Transparent Blit failure SRCPAINT");
			}
			// Now, clean up.
			hdcMem.SelectObject(hbmT);
			hdcMem.DeleteDC();
		}

		void CChildView::patTrackMute()
		{
			if (viewMode == view_modes::pattern)
			{
				_pSong->_trackMuted[editcur.track] = !_pSong->_trackMuted[editcur.track];
				Repaint(draw_modes::track_header);
			}
		}

		void CChildView::patTrackSolo()
		{
			if (viewMode == view_modes::pattern)
			{
				if (_pSong->_trackSoloed == editcur.track)
				{
					for (int i = 0; i < MAX_TRACKS; i++)
					{
						_pSong->_trackMuted[i] = FALSE;
					}
					_pSong->_trackSoloed = -1;
				}
				else
				{
					for (int i = 0; i < MAX_TRACKS; i++)
					{
						_pSong->_trackMuted[i] = TRUE;
					}
					_pSong->_trackMuted[editcur.track] = FALSE;
					_pSong->_trackSoloed = editcur.track;
				}
				Repaint(draw_modes::track_header);
			}
		}

		void CChildView::patTrackRecord()
		{
			if (viewMode == view_modes::pattern)
			{
				_pSong->_trackArmed[editcur.track] = !_pSong->_trackArmed[editcur.track];
				_pSong->_trackArmedCount = 0;
				for ( int i=0;i<MAX_TRACKS;i++ )
				{
					if (_pSong->_trackArmed[i])
					{
						_pSong->_trackArmedCount++;
					}
				}
				Repaint(draw_modes::track_header);
			}
		}

		void CChildView::DoMacPropDialog(int propMac)
		{
			if((propMac < 0 ) || (propMac >= MAX_MACHINES-1)) return;
			CMacProp dlg;
			dlg.m_view=this;
			dlg.pMachine = Global::_pSong->_pMachine[propMac];
			dlg.pSong = Global::_pSong;
			dlg.thisMac = propMac;
			if(dlg.DoModal() == IDOK)
			{
				sprintf(dlg.pMachine->_editName, dlg.txt);
				pParentMain->StatusBarText(dlg.txt);
				pParentMain->UpdateEnvInfo();
				pParentMain->UpdateComboGen();
				if (pParentMain->pGearRackDialog)
				{
					pParentMain->RedrawGearRackList();
				}
			}
			if(dlg.deleted)
			{
				pParentMain->CloseMacGui(propMac);
				CExclusiveLock lock(&Global::_pSong->semaphore, 2, true);
				Machine* pMac = Global::song()._pMachine[propMac];
				// move wires before deleting. Don't do inside DeleteMachine since that is used in song internally
				if( pMac->_numInputs > 0 && pMac->_numOutputs > 0) {
					//For each input connection
					for(int i = 0; i < MAX_CONNECTIONS; i++) if(pMac->_inputCon[i]) {
						Machine* srcMac = Global::song()._pMachine[pMac->_inputMachines[i]];
						int wiresrc = srcMac->FindOutputWire(pMac->_macIndex);
						bool first = true;
						//Connect it to each output connection
						for(int i = 0; i < MAX_CONNECTIONS; i++) if(pMac->_connection[i]) {
							Machine* dstMac = Global::song()._pMachine[pMac->_outputMachines[i]];
							//Except if already connected
							if( dstMac->FindInputWire(srcMac->_macIndex) == -1) {
								int wiredst = dstMac->FindInputWire(pMac->_macIndex);
								//If first wire change, it can be moved. Else it needs a new connection.
								if(first) {
									Global::song().ChangeWireDestMacNonBlocking(srcMac,dstMac,wiresrc,wiredst);
									first = false;
								}
								else {
									Global::song().InsertConnectionNonBlocking(srcMac, dstMac);
								}
							}
						}
					}
				}
				Global::_pSong->DestroyMachine(propMac);

				pParentMain->UpdateEnvInfo();
				pParentMain->UpdateComboGen();
				if (pParentMain->pGearRackDialog)
				{
					pParentMain->RedrawGearRackList();
				}
			}
		}

		void CChildView::OnConfigurationLoopplayback() 
		{
			Global::pPlayer->_loopSong = !Global::pPlayer->_loopSong;
		}

		void CChildView::OnUpdateConfigurationLoopplayback(CCmdUI* pCmdUI) 
		{
			if (Global::pPlayer->_loopSong)
				pCmdUI->SetCheck(1);
			else
				pCmdUI->SetCheck(0);	
		}

}}

// graphics operations, private headers included only by this translation unit
#include "MachineView.private.hpp"
#include "PatViewNew.private.hpp"
#include "SeqView.private.hpp"

// User/Mouse Responses, private headers included only by this translation unit
#include "KeybHandler.private.hpp"
#include "MouseHandler.private.hpp"
