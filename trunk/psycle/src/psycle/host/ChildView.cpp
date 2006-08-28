///\file
///\brief implementation file for psycle::host::CChildView.
#include <packageneric/pre-compiled.private.hpp>
#include <packageneric/module.private.hpp>
#include <psycle/host/uiversion.hpp>
#include <psycle/host/psycle.hpp>
#include <psycle/host/uiconfiguration.hpp>
#include <psycle/engine/Player.hpp>
#include <psycle/host/MainFrm.hpp>
#include <psycle/host/ChildView.hpp>
#include <psycle/engine/MidiInput.hpp>
#include <psycle/host/ConfigDlg.hpp>
#include <psycle/host/GreetDialog.hpp>
#include <psycle/host/SaveWavDlg.hpp>
#include <psycle/host/SongpDlg.hpp>
#include <psycle/engine/XMSongLoader.hpp>
#include <psycle/engine/ITModule2.hpp>
#include <psycle/host/MasterDlg.hpp>
#include <psycle/engine/internal_machines.hpp>
#include <psycle/host/XMSamplerUI.hpp>
#include <psycle/host/VstEditorDlg.hpp>
#include <psycle/host/WireDlg.hpp>
#include <psycle/host/MacProp.hpp>
#include <psycle/host/NewMachine.hpp>
#include <psycle/host/PatDlg.hpp>
#include <psycle/host/SwingFillDlg.hpp>
#include <psycle/host/ProgressDialog.hpp>
#include <cmath> // SwingFill
#include <cderr.h>

UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(psycle)
	UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(host)

		CMainFrame		*pParentMain;

		//\todo: Investigate this variable ( and especially song().cpuIdle ). See if it is doing what it is supposed to.
		unsigned idletime = 0;

		CChildView::CChildView()
		:
			smac(-1),
			smacmode(smacmodes::move),
			wiresource(-1),
			wiredest(-1),
			wiremove(-1),
			wireSX(0),
			wireSY(0),
			wireDX(0),
			wireDY(0),
			maxView(false),
			textLeftEdge(2),
			bmpDC(0),
			viewMode(view_modes::machine),
			MasterMachineDialog(0),
			SamplerMachineDialog(0),
			XMSamplerMachineDialog(0),
			updateMode(0),
			updatePar(0),
			//multiPattern(true), // Long way till it can be finished!
			patStep(1),
			editPosition(0),
			prevEditPosition(0),
			bEditMode(true),

			blockSelected(false),
			blockswitch(false),
			isBlockCopied(false),
			patBufferCopy(false),
			blockNTracks(0),
			blockNLines(0),
			bScrollDetatch(false),

			blockSelectBarState(1),

			pUndoList(0),
			pRedoList(0),

			UndoCounter(0),
			UndoSaved(0),

			UndoMacCounter(0),
			UndoMacSaved(0),

			//editcur.track(0), // Not needed to initialize, since the class does it already.
			//editcur.col(0),
			//editcur.line(0),
			playpos(-1),
			newplaypos(-1),
			numPatternDraw(0),

			//scrollT(0),
			//scrollL(0),
			tOff(0),
			lOff(0),
			ntOff(0),
			nlOff(0),

			ChordModeOffs(0),

			_outputActive(false),

			// just give arbitrary values so OnSize doesn't give /0 error
			// they will be filled in correctly when we switch to pattern view
			VISLINES(2),
			VISTRACKS(8)
		{
			for (int c=0; c<256; c++) FLATSIZES[c]=8;
			for(int c(0) ; c < MAX_WIRE_DIALOGS ; ++c) WireDialog[c] = 0;

			UIGlobal::pInputHandler->SetChildView(this);
			Global::pResampler->SetQuality(dsp::R_LINEAR);

			//_getcwd(m_appdir,_MAX_PATH);
			
			// Creates a new song object. The application Song.
			//Global::song().Reset(); It's already called in _pSong->New();
			Global::song().New();
			
			// Referencing the childView song pointer to the
			// Main Global::_pSong object [The application Global::_pSong]
			_pSong = Global::pSong;
			_pSong->report.connect(this,&CChildView::onSignalReport);
			_pSong->progress.connect(this,&CChildView::onSignalProgress);
			progressdialog=0;

			selpos.bottom = 0;
			newselpos.bottom = 0;
		}

		CChildView::~CChildView()
		{
			UIGlobal::pInputHandler->SetChildView(NULL);
			KillRedo();
			KillUndo();
			if(bmpDC)
			{
				std::ostringstream s;
				s << "CChildView::~CChildView(). Deleted bmpDC (was " << bmpDC << ")";
				loggers::trace(s.str());
				bmpDC->DeleteObject();
				delete bmpDC; bmpDC = 0;
			}
			patternheader.DeleteObject();
			DeleteObject(hbmPatHeader);
			machineskin.DeleteObject();
			DeleteObject(hbmMachineSkin);
			patternheadermask.DeleteObject();
			machineskinmask.DeleteObject();
			machinebkg.DeleteObject();
			DeleteObject(hbmMachineBkg);
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
			ON_COMMAND(ID_POP_CHANGEGENERATOR, OnPopChangegenerator)
			ON_COMMAND(ID_POP_CHANGEINSTRUMENT, OnPopChangeinstrument)
			ON_COMMAND(ID_POP_TRANSPOSE1, OnPopTranspose1)
			ON_COMMAND(ID_POP_TRANSPOSE12, OnPopTranspose12)
			ON_COMMAND(ID_POP_TRANSPOSE_1, OnPopTranspose_1)
			ON_COMMAND(ID_POP_TRANSPOSE_12, OnPopTranspose_12)
			ON_COMMAND(ID_AUTOSTOP, OnAutostop)
			ON_UPDATE_COMMAND_UI(ID_AUTOSTOP, OnUpdateAutostop)
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
			ON_UPDATE_COMMAND_UI(ID_POP_CHANGEGENERATOR, OnUpdateCutCopy)
			ON_UPDATE_COMMAND_UI(ID_POP_CHANGEINSTRUMENT, OnUpdateCutCopy)
			ON_UPDATE_COMMAND_UI(ID_POP_TRANSPOSE1, OnUpdateCutCopy)
			ON_UPDATE_COMMAND_UI(ID_POP_TRANSPOSE12, OnUpdateCutCopy)
			ON_UPDATE_COMMAND_UI(ID_POP_TRANSPOSE_1, OnUpdateCutCopy)
			ON_UPDATE_COMMAND_UI(ID_POP_TRANSPOSE_12, OnUpdateCutCopy)
			ON_UPDATE_COMMAND_UI(ID_POP_BLOCK_SWINGFILL, OnUpdateCutCopy)
			ON_COMMAND(ID_EDIT_CUT, patCut)
			ON_COMMAND(ID_EDIT_COPY, patCopy)
			ON_COMMAND(ID_EDIT_PASTE, patPaste)
			ON_COMMAND(ID_EDIT_MIXPASTE, patMixPaste)
			ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, OnUpdatePatternCutCopy)
			ON_UPDATE_COMMAND_UI(ID_EDIT_MIXPASTE, OnUpdatePatternPaste)
			ON_COMMAND(ID_EDIT_DELETE, patDelete)
			ON_UPDATE_COMMAND_UI(ID_EDIT_DELETE, OnUpdatePatternCutCopy)
			ON_COMMAND(ID_SHOWPSEQ, OnShowPatternSeq)
			ON_UPDATE_COMMAND_UI(ID_SHOWPSEQ, OnUpdatePatternSeq)
			ON_COMMAND(ID_POP_BLOCKSWITCH, OnPopBlockswitch)
			ON_UPDATE_COMMAND_UI(ID_POP_BLOCKSWITCH, OnUpdatePopBlockswitch)
		END_MESSAGE_MAP()

		BOOL CChildView::PreCreateWindow(CREATESTRUCT& cs) 
		{
			if (!CWnd::PreCreateWindow(cs))
				return false;
			
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
			return true;
		}

		/// This function gives to the pParentMain the pointer to a CMainFrm
		/// object. Call this function from the CMainframe side object to
		/// allow CCHildView call functions of the CMainFrm parent object
		/// Call this function after creating both the CCHildView object and
		/// the cmainfrm object
		void CChildView::ValidateParent()
		{
			pParentMain=(CMainFrame *)pParentFrame;
			pParentMain->_pSong=Global::pSong;
		}

		void CChildView::onSignalReport(const std::string & message, const std::string &caption)
		{
			MessageBox(message.c_str(),caption.c_str(),MB_OK);
		}
		void CChildView::onSignalProgress(const std::uint32_t & type, const std::uint32_t &position, const std::string& message)
		{
			switch (type)
			{
				case 1:
					progressdialog = new CProgressDialog();
					progressdialog->Create();
					progressdialog->m_Progress.SetPos(0);
					progressdialog->ShowWindow(SW_SHOW);
					break;
				case 2:
					if (!progressdialog) throw (int)1;
					progressdialog->SetWindowText(message.c_str());
					break;
				case 3:
					if (!progressdialog) throw (int)1;
					progressdialog->m_Progress.SetRange(0,position);
				case 4:
					if (!progressdialog) throw (int)1;
					if ( position == -1)
					{
						progressdialog->m_Progress.StepIt();
					} else progressdialog->m_Progress.SetPos(position);
					break;
				case 5:
					if (progressdialog)
					{
						progressdialog->OnCancel();
						delete progressdialog;
						progressdialog=0;
					}
					break;
				default:
					break;
			}
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

			if ( Global::configuration().autosaveSong )
			{
				if (!SetTimer(159,Global::configuration().autosaveSongTime*60000,NULL)) // Autosave Song
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
				#if !defined PSYCLE__CONFIGURATION__READ_WRITE_MUTEX
					#error PSYCLE__CONFIGURATION__READ_WRITE_MUTEX isn't defined anymore, please clean the code where this error is triggered.
				#else
					#if PSYCLE__CONFIGURATION__READ_WRITE_MUTEX // new implementation
						boost::read_write_mutex::scoped_read_write_lock lock(Global::song().read_write_mutex(),boost::read_write_lock_state::read_locked);
					#else // original implementation
						//\todo : IMPORTANT! change this lock to a more flexible one
						// It is causing skips on sound when there is a pattern change because
						// it is not allowing the player to work. Do the same in the one inside
						// Player::Work()
						CSingleLock lock(&_pSong->door,true);
					#endif
				#endif
				if (Global::song()._pMachine[MASTER_INDEX])
				{
					pParentMain->UpdateVumeters
						(
							//((Master*)Global::song()._pMachine[MASTER_INDEX])->_LMAX,
							//((Master*)Global::song()._pMachine[MASTER_INDEX])->_RMAX,
							((Master*)Global::song()._pMachine[MASTER_INDEX])->_lMax,
							((Master*)Global::song()._pMachine[MASTER_INDEX])->_rMax,
							Global::configuration().vu1,
							Global::configuration().vu2,
							Global::configuration().vu3,
							((Master*)Global::song()._pMachine[MASTER_INDEX])->_clip
						);
					pParentMain->UpdateMasterValue(((Master*)Global::song()._pMachine[MASTER_INDEX])->_outDry);
					if ( MasterMachineDialog ) MasterMachineDialog->UpdateUI();
					((Master*)Global::song()._pMachine[MASTER_INDEX])->vuupdated = true;
				}
				if (viewMode == view_modes::machine)
				{
					//\todo : Move the commented code to a "Tweak", so we can reuse the code below of "Global::player().Tweaker"
/*					if (Global::player()._playing && Global::player()._lineChanged)
					{
						// This is meant to repaint the whole machine in case the panning/mute/solo/bypass has changed. (not really implemented right now)
						Repaint(all_machines);
					}
					else
					{*/
						CClientDC dc(this);
						DrawAllMachineVumeters(&dc);
//					}
				}

				if (Global::player().Tweaker)
				{
					for(int c=0; c<MAX_MACHINES; c++)
					{
						if (_pSong->_pMachine[c])
						{
							if (pParentMain->isguiopen[c])
							{
								if ( _pSong->_pMachine[c]->subclass() == MACH_PLUGIN )
								{
									pParentMain->m_pWndMac[c]->Invalidate(false);
								}
								else if ( _pSong->_pMachine[c]->subclass() == MACH_VST ||
										_pSong->_pMachine[c]->subclass() == MACH_VSTFX )
								{
									((CVstEditorDlg*)pParentMain->m_pWndMac[c])->Refresh(-1,0);
								}
							}
						}
					}
					Global::player().Tweaker = false;
				}
				if (XMSamplerMachineDialog != NULL ) XMSamplerMachineDialog->UpdateUI();
				if (Global::player()._playing)
				{
					if (Global::player()._lineChanged)
					{
						Global::player()._lineChanged = false;
						pParentMain->SetAppSongBpm(0);
						pParentMain->SetAppSongTpb(0);

						if (Global::configuration()._followSong)
						{
							CListBox* pSeqList = (CListBox*)pParentMain->m_wndSeq.GetDlgItem(IDC_SEQLIST);
							editcur.line=Global::player()._lineCounter;
							if (editPosition != Global::player()._playPosition)
							//if (pSeqList->GetCurSel() != Global::player()._playPosition)
							{
								pSeqList->SelItemRange(false,0,pSeqList->GetCount());
								pSeqList->SetSel(Global::player()._playPosition,true);
								editPosition=Global::player()._playPosition;
								if ( viewMode == view_modes::pattern ) 
								{ 
									Repaint(draw_modes::pattern);//playback_change);  // Until this mode is coded there is no point in calling it since it just makes patterns not refresh correctly currently
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
			if (nIDEvent == 159 && !Global::player()._recording)
			{
				//MessageBox("Saving Disabled");
				//return;
				CString filepath = Global::configuration().GetSongDir().c_str();
				filepath += "\\autosave.psy";
				RiffFile file;
				if(!file.Create(filepath.GetBuffer(1), true)) return;
				_pSong->Save(&file,true);
				file.Close();
			}
		}

		void CChildView::EnableSound()
		{
			if (_outputActive)
			{
				AudioDriver* pOut = Global::configuration()._pOutputDriver;
				_outputActive = false;
				if (!pOut->Initialized())
				{
					pOut->Initialize(m_hWnd, Global::player().Work, Global::pPlayer);
				}
				if (!pOut->Configured())
				{
					pOut->Configure();
					Global::player().SampleRate(pOut->_samplesPerSec);
					_outputActive = true;
				}
				if (pOut->Enable(true))
				{
					_outputActive = true;
				}
				// MIDI IMPLEMENTATION
				Global::configuration()._pMidiInput->Open();
				// set midi input mode to real-time or step
				if(Global::configuration()._midiMachineViewSeqMode)
					CMidiInput::Instance()->m_midiMode = MODE_REALTIME;
				else
					CMidiInput::Instance()->m_midiMode = MODE_STEP;

				// Test to use RMS values for Vumeters.
				#if PSYCLE__CONFIGURATION__RMS_VUS
					dsp::numRMSSamples=pOut->_samplesPerSec*0.05f;
					dsp::countRMSSamples=0;
					dsp::RMSAccumulatedLeft=0;
					dsp::RMSAccumulatedRight=0;
				#endif
			}
		}


		/// Put exit destroying code here...
		void CChildView::OnDestroy()
		{
			if (Global::configuration()._pOutputDriver->Initialized())
			{
				Global::configuration()._pOutputDriver->Reset();
			}
			KillTimer(31);
			KillTimer(159);
		}

		void CChildView::OnAppExit() 
		{
			pParentMain->ClosePsycle();
		}

		void CChildView::OnPaint() 
		{
			if (!GetUpdateRect(NULL) ) return; // If no area to update, exit.
			CPaintDC dc(this);
			if(bmpDC == NULL && Global::configuration().useDoubleBuffer ) // buffer creation
			{
				CRect rc;
				GetClientRect(&rc);
				bmpDC = new CBitmap;
				bmpDC->CreateCompatibleBitmap(&dc,rc.right-rc.left,rc.bottom-rc.top);
				{
					std::ostringstream s;
					s << "CChildView::OnPaint(). Initialized bmpDC to " << bmpDC;
					loggers::trace(s.str());
				}
			}
			else if ( bmpDC != NULL && !Global::configuration().useDoubleBuffer ) // buffer deletion
			{
				{
					std::ostringstream s;
					s << "CChildView::OnPaint(). Deleted bmpDC (was " << bmpDC << ")";
					loggers::trace(s.str());
				}
				bmpDC->DeleteObject();
				delete bmpDC; bmpDC = 0;
			}
			if ( Global::configuration().useDoubleBuffer )
			{
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
						//ClearMachineSpace(Global::song()._pMachines[updatePar], updatePar, &bufDC);
						DrawMachine(updatePar, &bufDC);
						DrawMachineVumeters(updatePar, &bufDC);
						updateMode=draw_modes::all;
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
			else
			{
				if (viewMode==view_modes::machine) // Machine view paint handler
				{
					switch (updateMode)
					{
					case draw_modes::all:
						DrawMachineEditor(&dc);
						break;
					case draw_modes::machine:
						//ClearMachineSpace(Global::song()._pMachines[updatePar], updatePar, &dc);
						DrawMachine(updatePar, &dc);
						DrawMachineVumeters(updatePar, &dc);
						updateMode=draw_modes::all;
						break;
					case draw_modes::all_machines:
						for (int i=0;i<MAX_MACHINES;i++)
						{
							if (_pSong->_pMachine[i]) 
							{
								DrawMachine(i, &dc);
							}
						}
						DrawAllMachineVumeters(&dc);
						updateMode=draw_modes::all;
						break;
					}
				}
				else if (viewMode == view_modes::pattern)	// Pattern view paint handler
				{
					DrawPatEditor(&dc);
				}
				else if ( viewMode == view_modes::sequence)
				{
					DrawSeqEditor(&dc);
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
					PreparePatternRefresh(drawMode);
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

			CW = cx;
			CH = cy;
			_pSong->viewSize.x=cx; // Hack to move machines boxes inside of the visible area.
			_pSong->viewSize.y=cy;
			
			if ( bmpDC != NULL && Global::configuration().useDoubleBuffer ) // remove old buffer to force recreating it with new size
			{
				TRACE("CChildView::OnResize(). Deleted bmpDC");
				bmpDC->DeleteObject();
				zapObject(bmpDC);
			}
			if (viewMode == view_modes::pattern)
			{
				RecalcMetrics();
			}
			Repaint();
		}

		/// "Save Song" Function
		BOOL CChildView::OnFileSave(UINT id) 
		{
			#if defined PSYCLE__CONFIGURATION__SAVE_BROKEN
				MessageBox("Sorry, saving is not functional in this experimental build.");
				return false;
			#endif
			BOOL bResult = true;
			if ( Global::song()._saved )
			{
				if (MessageBox("Proceed with Saving?","Song Save",MB_YESNO) == IDYES)
				{
					std::string filepath = Global::configuration().GetCurrentSongDir();
					filepath += '\\';
					filepath += Global::song().fileName;
					
					RiffFile file;
					if (!file.Create((char*)filepath.c_str(), true))
					{
						MessageBox("Error creating file!", "Error!", MB_OK);
						return false;
					}
					if (!_pSong->Save(&file))
					{
						MessageBox("Error saving file!", "Error!", MB_OK);
						bResult = false;
					}
					else 
					{
						_pSong->_saved=true;
						if (pUndoList)
						{
							UndoSaved = pUndoList->counter;
						}
						else
						{
							UndoSaved = 0;
						}
						UndoMacSaved = UndoMacCounter;
						SetTitleBarText();
					}				
					file.Close();
				}
				else 
				{
					return false;
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
			#if defined PSYCLE__CONFIGURATION__SAVE_BROKEN
				MessageBox("Sorry, saving is not functional in this experimental build.");
				return false;
			#endif
			OPENFILENAME ofn; // common dialog box structure
			std::string ifile = Global::song().fileName;
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
			ofn.lpstrFilter =
				"Psycle Song (*.psy)"               "\0" "*.psy"        "\0"
				#if !defined PSYCLE__CONFIGURATION__SERIALIZATION
					#error PSYCLE__CONFIGURATION__SERIALIZATION isn't defined! Check the code where this error is triggered.
				#elif PSYCLE__CONFIGURATION__SERIALIZATION
					"Psycle Song as XML (*.psycle.xml)" "\0" "*.psycle.xml" "\0"
				#endif
				"Psycle Pattern (*.psb)"            "\0" "*.psb"        "\0"
				"All (*)"                           "\0" "*"            "\0"
				;
			ofn.nFilterIndex = 1;
			ofn.lpstrFileTitle = NULL;
			ofn.nMaxFileTitle = 0;
			std::string tmpstr = Global::configuration().GetCurrentSongDir();
			ofn.lpstrInitialDir = tmpstr.c_str();
			ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;
			BOOL bResult = true;
			
			// Display the Open dialog box. 
			if (GetSaveFileName(&ofn) == true)
			{
				CString str = ofn.lpstrFile;

				if ( ofn.nFilterIndex == 2 ) 
				#if !defined PSYCLE__CONFIGURATION__SERIALIZATION
					#error PSYCLE__CONFIGURATION__SERIALIZATION isn't defined! Check the code where this error is triggered.
				#elif PSYCLE__CONFIGURATION__SERIALIZATION
				{
					std::string extension(".psycle.xml");
					CString str2 = str.Right(extension.length());
					if ( str2.CompareNoCase(extension.c_str()) != 0 ) str.Insert(str.GetLength(),extension.c_str()); ///\todo remove ".psy" first
					sprintf(szFile,str);
					try
					{
						_pSong->SaveXML(szFile);
					}
					catch(...)
					{
						MessageBox("Error creating file!", "Error!", MB_OK);
						return false;
					}
				}
				else if ( ofn.nFilterIndex == 3 ) 
				#endif
				{
					CString str2 = str.Right(4);
					if ( str2.CompareNoCase(".psb") != 0 ) str.Insert(str.GetLength(),".psb"); ///\todo remove ".psy" first
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
					if (index >= 0)
					{
						Global::configuration().SetCurrentSongDir(static_cast<char const *>(str.Left(index)));
						Global::song().fileName = str.Mid(index+1);
					}
					else
					{
						Global::song().fileName = str;
					}
					
					RiffFile file;
					if (!file.Create(str.GetBuffer(1), true))
					{
						MessageBox("Error creating file!", "Error!", MB_OK);
						return false;
					}
					if (!_pSong->Save(&file))
					{
						MessageBox("Error saving file!", "Error!", MB_OK);
						bResult = false;
					}
					else 
					{
						_pSong->_saved=true;
						AppendToRecent(str.GetBuffer(1));
						
						if (pUndoList)
						{
							UndoSaved = pUndoList->counter;
						}
						else
						{
							UndoSaved = 0;
						}
						UndoMacSaved = UndoMacCounter;
						SetTitleBarText();
					}
					file.Close();
				}
			}
			else
			{
				return false;
			}
			return bResult;
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
			ofn.lpstrFilter =
				"Psycle Songs (*.psy)"    "\0" "*.psy" "\0"
				"Psycle Patterns (*.psb)" "\0" "*.psb" "\0"
				"All (*)"                 "\0"  "*"    "\0"
				;
			ofn.nFilterIndex = 1;
			ofn.lpstrFileTitle = NULL;
			ofn.nMaxFileTitle = 0;
			std::string tmpstr = Global::configuration().GetCurrentSongDir();
			ofn.lpstrInitialDir = tmpstr.c_str();
			ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
			
			// Display the Open dialog box. 
			if(::GetOpenFileName(&ofn)==true)
			{
				OnFileLoadsongNamed(szFile, ofn.nFilterIndex);
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
			pParentMain->StatusBarIdle();
		}

		void CChildView::OnFileNew() 
		{
			if (CheckUnsavedSong("New Song"))
			{
				KillUndo();
				KillRedo();
				pParentMain->CloseAllMacGuis();
				Global::player().Stop();
				Sleep(LOCK_LATENCY);
				_outputActive = false;
				Global::configuration()._pOutputDriver->Enable(false);
				// midi implementation
				Global::configuration()._pMidiInput->Close();
				Sleep(LOCK_LATENCY);

				Global::song().New();
				_outputActive = true;
				if (!Global::configuration()._pOutputDriver->Enable(true))
				{
					_outputActive = false;
				}
				else
				{
					// midi implementation
					Global::configuration()._pMidiInput->Open();
				}
				SetTitleBarText();
				editPosition=0;
				Global::song().seqBus=0;
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
			KillTimer(31);
			KillTimer(159);
			OnTimer(159); // Autosave
			CSaveWavDlg dlg;
			dlg.DoModal();
			InitTimer();
		}

		///\todo that method does not take machine changes into account  
		//  <JosepMa> is this still the case? or what does "machine changes" mean?
		BOOL CChildView::CheckUnsavedSong(std::string szTitle)
		{
			BOOL bChecked = true;
			if (pUndoList)
			{
				if (UndoSaved != pUndoList->counter)
				{
					bChecked = false;
				}
			}
			else if (UndoMacSaved != UndoMacCounter)
			{
				bChecked = false;
			}
			else
			{
				if (UndoSaved != 0)
				{
					bChecked = false;
				}
			}
			if (!bChecked)
			{
				if (Global::configuration().bFileSaveReminders)
				{
					std::string filepath = Global::configuration().GetCurrentSongDir();
					filepath += '\\';
					filepath += Global::song().fileName;
					RiffFile file;
					std::ostringstream szText;
					szText << "Save changes to \"" << Global::song().fileName
						<< "\"?";
					int result = MessageBox(szText.str().c_str(),szTitle.c_str(),MB_YESNOCANCEL | MB_ICONEXCLAMATION);
					switch (result)
					{
					case IDYES:
						if (!file.Create((char*)filepath.c_str(), true))
						{
							std::ostringstream szText;
							szText << "Error writing to \"" << filepath << "\"!!!";
							MessageBox(szText.str().c_str(),szTitle.c_str(),MB_ICONEXCLAMATION);
							return false;
						}
						_pSong->Save(&file);
						file.Close();
						return true;
						break;
					case IDNO:
						return true;
						break;
					case IDCANCEL:
						return false;
						break;
					}
				}
			}
			return true;
		}

		void CChildView::OnFileRevert()
		{
			if (MessageBox("Warning! You will lose all changes since song was last saved! Proceed?","Revert to Saved",MB_YESNO | MB_ICONEXCLAMATION) == IDYES)
			{
				if (Global::song()._saved)
				{
					std::ostringstream fullpath;
					fullpath << Global::configuration().GetCurrentSongDir().c_str()
						<< '\\' << Global::song().fileName.c_str();
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
				ShowScrollBar(SB_BOTH,false);

				// set midi input mode to real-time or Step
				if(Global::configuration()._midiMachineViewSeqMode)
					CMidiInput::Instance()->m_midiMode = MODE_REALTIME;
				else
					CMidiInput::Instance()->m_midiMode = MODE_STEP;

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
				//ShowScrollBar(SB_BOTH,false);
				
				// set midi input mode to step insert
				CMidiInput::Instance()->m_midiMode = MODE_STEP;
				
				GetParent()->SetActiveWindow();

				if
					(
						Global::configuration()._followSong &&
						editPosition  != Global::player()._playPosition &&
						Global::player()._playing
					)
				{
					editPosition=Global::player()._playPosition;
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
			MessageBox("This feature is unimplemented in this release. Use the left side sequence for now.","Psycle Notice");
			/*
			if (viewMode != sequence)
			{
				viewMode = sequence;
				ShowScrollBar(SB_BOTH,false);
				
				// set midi input mode to step insert
				CMidiInput::Instance()->m_midiMode = MODE_STEP;
				
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
			if (Global::configuration()._followSong)
			{
				bScrollDetatch=false;
			}
			prevEditPosition=editPosition;
			Global::player().Start(editPosition,0);
			pParentMain->StatusBarIdle();
		}

		void CChildView::OnBarplayFromStart() 
		{
			if (Global::configuration()._followSong)
			{
				bScrollDetatch=false;
			}
			prevEditPosition=editPosition;
			Global::player().Start(0,0);
			pParentMain->StatusBarIdle();
		}

		void CChildView::OnUpdateBarplay(CCmdUI* pCmdUI) 
		{
			if (Global::player()._playing)
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
			if (Global::configuration()._followSong && bEditMode)
			{
				bEditMode = false;
			}
			else
			{
				Global::configuration()._followSong = true;
				bEditMode = true;
				CButton*cb=(CButton*)pParentMain->m_wndSeq.GetDlgItem(IDC_FOLLOW);
				cb->SetCheck(1);
			}
			pParentMain->StatusBarIdle();
		}

		void CChildView::OnUpdateBarrec(CCmdUI* pCmdUI) 
		{
			if (Global::configuration()._followSong && bEditMode)
				pCmdUI->SetCheck(1);
			else
				pCmdUI->SetCheck(0);
		}

		void CChildView::OnButtonplayseqblock() 
		{
			if (Global::configuration()._followSong)
			{
				bScrollDetatch=false;
			}

			prevEditPosition=editPosition;
			int i=0;
			while ( Global::song().playOrderSel[i] == false ) i++;
			
			if(!Global::player()._playing)
				Global::player().Start(i,0);

			Global::player()._playBlock=!Global::player()._playBlock;

			pParentMain->StatusBarIdle();
			if ( viewMode == view_modes::pattern ) Repaint(draw_modes::pattern);
		}

		void CChildView::OnUpdateButtonplayseqblock(CCmdUI* pCmdUI) 
		{
			if ( Global::player()._playBlock == true ) pCmdUI->SetCheck(true);
			else pCmdUI->SetCheck(false);
		}

		void CChildView::OnBarstop()
		{
			bool pl = Global::player()._playing;
			bool blk = Global::player()._playBlock;
			Global::player().Stop();
			pParentMain->SetAppSongBpm(0);
			pParentMain->SetAppSongTpb(0);

			if (pl)
			{
				if ( Global::configuration()._followSong && blk)
				{
					editPosition=prevEditPosition;
					pParentMain->UpdatePlayOrder(false); // <- This restores the selected block
					Repaint(draw_modes::pattern);
				}
				else
				{
					memset(Global::song().playOrderSel,0,MAX_SONG_POSITIONS*sizeof(bool));
					Global::song().playOrderSel[editPosition] = true;
					Repaint(draw_modes::cursor); 
				}
			}
		}

		void CChildView::OnRecordWav() 
		{
			if (!Global::player()._recording)
			{
				static char BASED_CODE szFilter[] = "Wav Files (*.wav)|*.wav|All Files (*.*)|*.*||";
				
				CFileDialog dlg(false,"wav",NULL,OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,szFilter);
				if ( dlg.DoModal() == IDOK ) 
				{
					Global::player().StartRecording(dlg.GetFileName().GetBuffer(4));
				}
				if ( Global::configuration().autoStopMachines ) 
				{
					OnAutostop();
				}
			}
			else
			{
				Global::player().StopRecording();
			}
		}

		void CChildView::OnUpdateRecordWav(CCmdUI* pCmdUI) 
		{
			if (Global::player()._recording)
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
			if ( Global::configuration().autoStopMachines )
			{
				Global::configuration().autoStopMachines = false;
				for (int c=0; c<MAX_MACHINES; c++)
				{
					if (Global::song()._pMachine[c])
					{
						Global::song()._pMachine[c]->_stopped=false;
					}
				}
			}
			else Global::configuration().autoStopMachines = true;
		}

		void CChildView::OnUpdateAutostop(CCmdUI* pCmdUI) 
		{
			if (Global::configuration().autoStopMachines == true ) pCmdUI->SetCheck(true);
			else pCmdUI->SetCheck(false);
		}

		void CChildView::OnFileSongproperties() 
		{	CSongpDlg dlg;
			dlg._pSong=Global::pSong;
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

		void CChildView::ShowPatternDlg(void)
		{
			CPatDlg dlg;
			int patNum = _pSong->playOrder[editPosition];
			int nlines = _pSong->patternLines[patNum];
			char name[32];
			strcpy(name,_pSong->patternName[patNum]);

			dlg.patLines= nlines;
			strcpy(dlg.patName,name);
			
			if (dlg.DoModal() == IDOK)
			{
				if ( nlines != dlg.patLines )
				{
					AddUndo(patNum,0,0,MAX_TRACKS,nlines,editcur.track,editcur.line,editcur.col,editPosition);
					AddUndoLength(patNum,nlines,editcur.track,editcur.line,editcur.col,editPosition);
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
					pParentMain->StatusBarIdle();
					//Repaint(pattern_header);
				}
			}

		}

		void CChildView::OnNewmachine() 
		{
			NewMachine();
		}

		/// Show new machine dialog
		void CChildView::NewMachine(int x, int y, Machine::id_type mac) 
		{
			CNewMachine dlg;
			if(dlg.DoModal() == IDOK && dlg.Outputmachine != MACH_UNDEFINED)
			{
				Machine::id_type fb;
				int xs,ys;
				if (mac < 0)
				{
					if (dlg.Outputmode == MACHMODE_FX)
					{
						fb = Global::song().GetFreeFxBus();
						xs = MachineCoords.sEffect.width;
						ys = MachineCoords.sEffect.height;
					}
					else 
					{
						fb = Global::song().GetFreeBus();
						xs = MachineCoords.sGenerator.width;
						ys = MachineCoords.sGenerator.height;
					}
				}
				else
				{
					if (mac >= MAX_BUSES && dlg.Outputmode == MACHMODE_FX)
					{
						fb = mac;
						xs = MachineCoords.sEffect.width;
						ys = MachineCoords.sEffect.height;
						// delete machine if it already exists
						if (Global::song()._pMachine[fb])
						{
							x = Global::song()._pMachine[fb]->GetPosX();
							y = Global::song()._pMachine[fb]->GetPosY();
							pParentMain->CloseMacGui(fb);
							Global::song().DestroyMachine(fb);
						}
					}
					else if (mac < MAX_BUSES && dlg.Outputmode != MACHMODE_FX)
					{
						fb = mac;
						xs = MachineCoords.sGenerator.width;
						ys = MachineCoords.sGenerator.height;
						// delete machine if it already exists
						if (Global::song()._pMachine[fb])
						{
							x = Global::song()._pMachine[fb]->GetPosX();
							y = Global::song()._pMachine[fb]->GetPosY();
							pParentMain->CloseMacGui(fb);
							Global::song().DestroyMachine(fb);
						}
					}
					else
					{
						MessageBox("Wrong Class of Machine!");
						return;
					}
				}
				AddMacViewUndo();
				// random position
				if ((x < 0) || (y < 0))
				{
					bool bCovered = true;
					while (bCovered)
					{
						x = (rand())%(CW-xs);
						y = (rand())%(CH-ys);
						bCovered = false;
						for (int i=0; i < MAX_MACHINES; i++)
						{
							if (Global::song()._pMachine[i])
							{
								if ((abs(Global::song()._pMachine[i]->GetPosX() - x) < 32) &&
									(abs(Global::song()._pMachine[i]->GetPosY() - y) < 32))
								{
									bCovered = true;
									i = MAX_MACHINES;
								}
							}
						}
					}
				}
				// Stop driver to handle possible conflicts between threads.
				// should be no conflicts because last thing create machine does is set active machine flag.
				// busses are set last, so no messages will be sent until after machine is created anyway
				/*
				_outputActive = false;
				Global::configuration()._pOutputDriver->Enable(false);
				// MIDI IMPLEMENTATION
				Global::configuration()._pMidiInput->Close();
				*/

				if ( fb == -1 || !Global::song().CreateMachine(dlg.Outputmachine, x, y, dlg.psOutputDll.c_str(), fb))
				{
					MessageBox("Machine Creation Failed","Error!",MB_OK);
				}
				else
				{
					if(dlg.Outputmode) Global::song().seqBus = fb;

					// make sure that no 2 machines have the same name, because that is irritating
					{
						std::stringstream ss;
						ss << _pSong->_pMachine[fb]->GetEditName() << " " << std::hex << fb;
						_pSong->_pMachine[fb]->SetEditName(ss.str());
					}

					pParentMain->UpdateComboGen();
					Repaint(draw_modes::all);
					//Repaint(draw_modes::machine); // Seems that this doesn't always work (multiple calls to Repaint?)
				}
				
				/*
				// Restarting the driver...
				pParentMain->UpdateEnvInfo();
				_outputActive = true;
				if (!Global::configuration()._pOutputDriver->Enable(true))
				{
					_outputActive = false;
				}
				else
				{
					// MIDI IMPLEMENTATION
					Global::configuration()._pMidiInput->Open();
				}
				*/
			}
			//Repaint();
			pParentMain->RedrawGearRackList();
		}

		void CChildView::OnConfigurationSettings() 
		{
			CConfigDlg dlg("Psycle Settings");
			_outputActive = false;
			dlg.Init(Global::pConfig);
			if (dlg.DoModal() == IDOK)
			{
				KillTimer(159);
				if ( Global::configuration().autosaveSong )
				{
					SetTimer(159,Global::configuration().autosaveSongTime*60000,NULL);
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

		///\todo extemely toxic pollution
		#define TWOPI_F (2.0f*3.141592665f)

		void CChildView::ShowSwingFillDlg(bool bTrackMode)
		{
			int st = Global::song().BeatsPerMin();
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
					AddUndo(_ps(),x,y,1,ny,editcur.track,editcur.line,editcur.col,editPosition);
					for (l=y;l<y+ny;l++)
					{
						int const displace=x*EVENT_SIZE+l*MULTIPLY;
						
						unsigned char *offset=base+displace;
						
						PatternEntry *entry = (PatternEntry*) offset;
						entry->_cmd = 0xff;
						int val = f2i(((sinf(index)*var*st)+st)+dcoffs);//-0x20; // ***** proposed change to ffxx command to allow more useable range since the tempo bar only uses this range anyway...
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
			if (blockSelected && (viewMode == view_modes::pattern)) pCmdUI->Enable(true);
			else pCmdUI->Enable(false);
		}

		void CChildView::OnPopCopy() { CopyBlock(false); }

		void CChildView::OnPopPaste() { PasteBlock(editcur.track,editcur.line,false); }
		void CChildView::OnUpdatePaste(CCmdUI* pCmdUI) 
		{
			if (isBlockCopied && (viewMode == view_modes::pattern)) pCmdUI->Enable(true);
			else  pCmdUI->Enable(false);
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

		void CChildView::OnPopInterpolate() { BlockParamInterpolate(); }

		void CChildView::OnPopChangegenerator() { BlockGenChange(_pSong->seqBus); }

		void CChildView::OnPopChangeinstrument() { BlockInsChange(_pSong->auxcolSelected); }

		void CChildView::OnPopTranspose1() { BlockTranspose(1); }

		void CChildView::OnPopTranspose12() { BlockTranspose(12); }

		void CChildView::OnPopTranspose_1() { BlockTranspose(-1); }

		void CChildView::OnPopTranspose_12() { BlockTranspose(-12); }

		void CChildView::OnPopPattenproperties() 
		{
			ShowPatternDlg();
		}

		/// fill block
		void CChildView::OnPopBlockSwingfill()
		{
			ShowSwingFillDlg(false);
		}

		/// fill track
		void CChildView::OnPopTrackSwingfill()
		{
			ShowSwingFillDlg(true);
		}

		void CChildView::OnUpdateUndo(CCmdUI* pCmdUI)
		{
			if(pUndoList) 
			{
				switch (pUndoList->type)
				{
				case UNDO_SEQUENCE:
					pCmdUI->Enable(true);
					pCmdUI->SetText("Undo");
					break;
				default:
					if(viewMode == view_modes::pattern)// && bEditMode)
					{
						pCmdUI->Enable(true);
						pCmdUI->SetText("Undo");
					}
					else
					{
						pCmdUI->Enable(false);
						pCmdUI->SetText("Undo in Pattern View");
					}
					break;
				}
			}
			else
			{
				pCmdUI->SetText("Undo");
				pCmdUI->Enable(false);
			}
		}

		void CChildView::OnUpdateRedo(CCmdUI* pCmdUI)
		{
			if(pRedoList) 
			{
				switch (pRedoList->type)
				{
				case UNDO_SEQUENCE:
					pCmdUI->Enable(true);
					pCmdUI->SetText("Redo");
					break;
				default:
					if(viewMode == view_modes::pattern)// && bEditMode)
					{
						pCmdUI->Enable(true);
						pCmdUI->SetText("Redo");
					}
					else
					{
						pCmdUI->Enable(false);
						pCmdUI->SetText("Redo in Pattern View");
					}
					break;
				}
			}
			else
			{
				pCmdUI->Enable(false);
				pCmdUI->SetText("Redo");
			}
		}

		void CChildView::OnUpdatePatternCutCopy(CCmdUI* pCmdUI) 
		{
			if(viewMode == view_modes::pattern) pCmdUI->Enable(true);
			else pCmdUI->Enable(false);
		}

		void CChildView::OnUpdatePatternPaste(CCmdUI* pCmdUI) 
		{
			if(patBufferCopy&&(viewMode == view_modes::pattern)) pCmdUI->Enable(true);
			else pCmdUI->Enable(false);
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
			std::string tmpstr = Global::configuration().GetCurrentSongDir();
			ofn.lpstrInitialDir = tmpstr.c_str();
			ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
			// Display the Open dialog box. 
			if (GetOpenFileName(&ofn)==true)
			{
				KillUndo();
				KillRedo();
				pParentMain->CloseAllMacGuis();
				Global::player().Stop();
				Sleep(LOCK_LATENCY);
				_outputActive = false;
				Global::configuration()._pOutputDriver->Enable(false);
				// MIDI IMPLEMENTATION
				Global::configuration()._pMidiInput->Close();
				Sleep(LOCK_LATENCY);

				CString str = ofn.lpstrFile;
				int index = str.ReverseFind('.');
				if (index != -1)
				{
					CString ext = str.Mid(index+1);
					if (ext.CompareNoCase("XM") == 0)
					{
						XMSongLoader xmfile;
						xmfile.Open(ofn.lpstrFile);
						Global::song().New();
						editPosition=0;
						xmfile.Load(*_pSong);
						xmfile.Close();
						char buffer[512];		
						std::sprintf
							(
							buffer,"%s\n\n%s\n\n%s",
							Global::song().Name,
							Global::song().Author,
							Global::song().Comment
							);
						MessageBox(buffer, "XM file imported", MB_OK);
					} else if (ext.CompareNoCase("IT") == 0)
					{
						ITModule2 it;
						it.Open(ofn.lpstrFile);
						Global::song().New();
						editPosition=0;
						if(!it.LoadITModule(_pSong))
						{			
							MessageBox("Load failed");
							Global::song().New();
							it.Close();
							return;
						}
						it.Close();
						char buffer[512];		
						std::sprintf
							(
							buffer,"%s\n\n%s\n\n%s",
							Global::song().Name,
							Global::song().Author,
							Global::song().Comment
							);
						MessageBox(buffer, "IT file imported", MB_OK);
					} else if (ext.CompareNoCase("S3M") == 0)
					{
						ITModule2 s3m;
						s3m.Open(ofn.lpstrFile);
						Global::song().New();
						editPosition=0;
						if(!s3m.LoadS3MModuleX(_pSong))
						{			
							MessageBox("Load failed");
							Global::song().New();
							s3m.Close();
							return;
						}
						s3m.Close();
						char buffer[512];		
						std::sprintf
							(
							buffer,"%s\n\n%s\n\n%s",
							Global::song().Name,
							Global::song().Author,
							Global::song().Comment
							);
						MessageBox(buffer, "S3M file imported", MB_OK);
					} else if (ext.CompareNoCase("MOD") == 0)
					{
						MODSongLoader modfile;
						modfile.Open(ofn.lpstrFile);
						Global::song().New();
						editPosition=0;
						modfile.Load(*_pSong);
						modfile.Close();
						char buffer[512];		
						std::sprintf
							(
							buffer,"%s\n\n%s\n\n%s",
							Global::song().Name,
							Global::song().Author,
							Global::song().Comment
							);
						MessageBox(buffer, "MOD file imported", MB_OK);
					}
				}

				str = ofn.lpstrFile;
				index = str.ReverseFind('\\');
				if (index != -1)
				{
					Global::configuration().SetCurrentSongDir((LPCSTR)str.Left(index));
					Global::song().fileName = str.Mid(index+1)+".psy";
				}
				else
				{
					Global::song().fileName = str+".psy";
				}
				_outputActive = true;
				if (!Global::configuration()._pOutputDriver->Enable(true))
				{
					_outputActive = false;
				}
				else
				{
					// MIDI IMPLEMENTATION
					Global::configuration()._pMidiInput->Open();
				}
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

		void CChildView::AppendToRecent(std::string fName)
		{
			int iCount;
			char* nameBuff;
			UINT nameSize;
			HMENU hFileMenu, hRootMenuBar;
			UINT ids[] =
				{
					ID_FILE_RECENT_01,
					ID_FILE_RECENT_02,
					ID_FILE_RECENT_03,
					ID_FILE_RECENT_04
				};
			MENUITEMINFO hNewItemInfo, hTempItemInfo;
			hRootMenuBar = ::GetMenu(this->GetParent()->m_hWnd);
			//pRootMenuBar = this->GetParent()->GetMenu();
			//hRootMenuBar = HMENU (*pRootMenuBar);
			hFileMenu = GetSubMenu(hRootMenuBar, 0);
			hRecentMenu = GetSubMenu(hFileMenu, 10);
			// Remove initial empty element, if present.
			if(GetMenuItemID(hRecentMenu, 0) == ID_FILE_RECENT_NONE)
			{
				DeleteMenu(hRecentMenu, 0, MF_BYPOSITION);
			}
			// Check for duplicates and eventually remove.
			for(iCount = 0; iCount<GetMenuItemCount(hRecentMenu);iCount++)
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
			hNewItemInfo.cbSize		= sizeof(MENUITEMINFO);
			hNewItemInfo.fMask		= MIIM_ID | MIIM_TYPE;
			hNewItemInfo.fType		= MFT_STRING;
			hNewItemInfo.wID		= ids[0];
			hNewItemInfo.cch		= static_cast<UINT>(fName.length()); ///\todo recheck this on a 64-bit platform
			hNewItemInfo.dwTypeData = (LPSTR)fName.c_str();
			InsertMenuItem(hRecentMenu, 0, true, &hNewItemInfo);
			// Update identifiers.
			for(iCount = 1;iCount < GetMenuItemCount(hRecentMenu);iCount++)
			{
				hTempItemInfo.cbSize	= sizeof(MENUITEMINFO);
				hTempItemInfo.fMask		= MIIM_ID;
				hTempItemInfo.wID		= ids[iCount];
				SetMenuItemInfo(hRecentMenu, iCount, true, &hTempItemInfo);
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
			Global::player().Stop();
			Sleep(LOCK_LATENCY);
			_outputActive = false;
			Global::configuration()._pOutputDriver->Enable(false);
			// MIDI IMPLEMENTATION
			Global::configuration()._pMidiInput->Close();
			Sleep(LOCK_LATENCY);
			
			RiffFile file;
			if (!file.Open(fName.c_str()))
			{
				MessageBox("Could not Open file. Check that the location is correct.", "Loading Error", MB_OK);
				return;
			}
			editPosition = 0;
			_pSong->Load(&file);
			file.Close();
			_pSong->_saved=true;
			AppendToRecent(fName);
			std::string::size_type index = fName.rfind('\\');
			if (index != std::string::npos)
			{
				Global::configuration().SetCurrentSongDir(fName.substr(0,index));
				Global::song().fileName = fName.substr(index+1);
			}
			else
			{
				Global::song().fileName = fName;
			}
//			Global::song().SetBPM(Global::song().BeatsPerMin(), Global::song()._LinesPerBeat, Global::configuration()._pOutputDriver->_samplesPerSec);
			_outputActive = true;
			if (!Global::configuration()._pOutputDriver->Enable(true))
			{
				_outputActive = false;
			}
			else
			{
				// MIDI IMPLEMENTATION
				Global::configuration()._pMidiInput->Open();
			}
			pParentMain->PsybarsUpdate();
			pParentMain->WaveEditorBackUpdate();
			pParentMain->m_wndInst.WaveUpdate();
			pParentMain->RedrawGearRackList();
			pParentMain->UpdateSequencer();
			pParentMain->UpdatePlayOrder(false);
			//pParentMain->UpdateComboIns(); PsyBarsUpdate calls UpdateComboGen that also calls UpdatecomboIns
			RecalculateColourGrid();
			Repaint();
			KillUndo();
			KillRedo();
			SetTitleBarText();
			if (Global::configuration().bShowSongInfoOnLoad)
			{
				std::ostringstream songLoaded;
				songLoaded << '\'' << _pSong->Name << '\'' << std::endl
					<< std::endl
					<< _pSong->Author << std::endl
					<< std::endl
					<< _pSong->Comment;
				MessageBox(songLoaded.str().c_str(), "Psycle song loaded", MB_OK);
			}
		}

		void CChildView::CallOpenRecent(int pos)
		{
			UINT nameSize;
			nameSize = GetMenuString(hRecentMenu, pos, 0, 0, MF_BYPOSITION) + 1;
			char* nameBuff = new char[nameSize];
			GetMenuString(hRecentMenu, pos, nameBuff, nameSize, MF_BYPOSITION);
			OnFileLoadsongNamed(nameBuff, 1);
			zapArray(nameBuff);
		}

		void CChildView::SetTitleBarText()
		{
			std::string titlename = "[";
			titlename+=Global::song().fileName;
			/*
			if(!(Global::song()._saved))
			{
				titlename+=" *";
			}
			else
			*/ 
			if(pUndoList)
			{
				if (UndoSaved != pUndoList->counter)
				{
					titlename+=" *";
				}
			}
			else if (UndoMacSaved != UndoMacCounter)
			{
				titlename+=" *";
			}
			else
			{
				if (UndoSaved != 0)
				{
					titlename+=" *";
				}
			}
			// don't know how to access to the IDR_MAINFRAME String Title.
			titlename += "] Psycle Modular Music Creation Studio (" PSYCLE__VERSION ")";
			pParentMain->SetWindowText(titlename.c_str());
		}

		void CChildView::OnHelpKeybtxt() 
		{
			char path[MAX_PATH];
			sprintf(path,"%sdocs\\keys.txt",Global::configuration().appPath());
			ShellExecute(pParentMain->m_hWnd,"open",path,NULL,"",SW_SHOW);
		}

		void CChildView::OnHelpReadme() 
		{
			char path[MAX_PATH];
			sprintf(path,"%sdocs\\readme.txt",Global::configuration().appPath());
			ShellExecute(pParentMain->m_hWnd,"open",path,NULL,"",SW_SHOW);
		}

		void CChildView::OnHelpTweaking() 
		{
			char path[MAX_PATH];
			sprintf(path,"%sdocs\\tweaking.txt",Global::configuration().appPath());
			ShellExecute(pParentMain->m_hWnd,"open",path,NULL,"",SW_SHOW);
		}

		void CChildView::OnHelpWhatsnew() 
		{
			char path[MAX_PATH];
			sprintf(path,"%sdocs\\whatsnew.txt",Global::configuration().appPath());
			ShellExecute(pParentMain->m_hWnd,"open",path,NULL,"",SW_SHOW);
		}

		void CChildView::LoadMachineSkin()
		{
			std::string szOld;
			LoadMachineDial();
			if (!Global::configuration().machine_skin.empty())
			{
				szOld = Global::configuration().machine_skin;
				if (szOld != PSYCLE__PATH__DEFAULT_MACHINE_SKIN)
				{
					BOOL result = false;
					FindMachineSkin(Global::configuration().GetSkinDir().c_str(),Global::configuration().machine_skin.c_str(), &result);
					if(result)
					{
						return;
					}
				}
				// load defaults
				szOld = PSYCLE__PATH__DEFAULT_MACHINE_SKIN;
				// and coords
				#if defined PSYCLE__CONFIGURATION__SKIN__UGLY_DEFAULT
					MachineCoords.sMaster.x = 0;
					MachineCoords.sMaster.y = 0;
					MachineCoords.sMaster.width = 148;
					MachineCoords.sMaster.height = 48;

					MachineCoords.sGenerator.x = 0;
					MachineCoords.sGenerator.y = 48;
					MachineCoords.sGenerator.width = 148;
					MachineCoords.sGenerator.height = 48;
					MachineCoords.sGeneratorVu0.x = 0;
					MachineCoords.sGeneratorVu0.y = 144;
					MachineCoords.sGeneratorVu0.width = 6;
					MachineCoords.sGeneratorVu0.height = 5;
					MachineCoords.sGeneratorVuPeak.x = 96;
					MachineCoords.sGeneratorVuPeak.y = 144;
					MachineCoords.sGeneratorVuPeak.width = 6;
					MachineCoords.sGeneratorVuPeak.height = 5;
					MachineCoords.sGeneratorPan.x = 21;
					MachineCoords.sGeneratorPan.y = 149;
					MachineCoords.sGeneratorPan.width = 24;
					MachineCoords.sGeneratorPan.height = 9;
					MachineCoords.sGeneratorMute.x = 7;
					MachineCoords.sGeneratorMute.y = 149;
					MachineCoords.sGeneratorMute.width = 7;
					MachineCoords.sGeneratorMute.height = 7;
					MachineCoords.sGeneratorSolo.x = 14;
					MachineCoords.sGeneratorSolo.y = 149;
					MachineCoords.sGeneratorSolo.width = 7;
					MachineCoords.sGeneratorSolo.height = 7;

					MachineCoords.sEffect.x = 0;
					MachineCoords.sEffect.y = 96;
					MachineCoords.sEffect.width = 148;
					MachineCoords.sEffect.height = 48;
					MachineCoords.sEffectVu0.x = 0;
					MachineCoords.sEffectVu0.y = 144;
					MachineCoords.sEffectVu0.width = 6;
					MachineCoords.sEffectVu0.height = 5;
					MachineCoords.sEffectVuPeak.x = 96;
					MachineCoords.sEffectVuPeak.y = 144;
					MachineCoords.sEffectVuPeak.width = 6;
					MachineCoords.sEffectVuPeak.height = 5;
					MachineCoords.sEffectPan.x = 21;
					MachineCoords.sEffectPan.y = 149;
					MachineCoords.sEffectPan.width = 24;
					MachineCoords.sEffectPan.height = 9;
					MachineCoords.sEffectMute.x = 7;
					MachineCoords.sEffectMute.y = 149;
					MachineCoords.sEffectMute.width = 7;
					MachineCoords.sEffectMute.height = 7;
					MachineCoords.sEffectBypass.x = 0;
					MachineCoords.sEffectBypass.y = 149;
					MachineCoords.sEffectBypass.width = 7;
					MachineCoords.sEffectBypass.height = 12;

					MachineCoords.dGeneratorVu.x = 8;
					MachineCoords.dGeneratorVu.y = 3;
					MachineCoords.dGeneratorVu.width = 96;
					MachineCoords.dGeneratorVu.height = 0;
					MachineCoords.dGeneratorPan.x = 3;
					MachineCoords.dGeneratorPan.y = 35;
					MachineCoords.dGeneratorPan.width = 117;
					MachineCoords.dGeneratorPan.height = 0;
					MachineCoords.dGeneratorMute.x = 137;
					MachineCoords.dGeneratorMute.y = 4;
					MachineCoords.dGeneratorSolo.x = 137;
					MachineCoords.dGeneratorSolo.y = 17;
					MachineCoords.dGeneratorName.x = 10;
					MachineCoords.dGeneratorName.y = 12;

					MachineCoords.dEffectVu.x = 8;
					MachineCoords.dEffectVu.y = 3;
					MachineCoords.dEffectVu.width = 96;
					MachineCoords.dEffectVu.height = 0;
					MachineCoords.dEffectPan.x = 3;
					MachineCoords.dEffectPan.y = 35;
					MachineCoords.dEffectPan.width = 117;
					MachineCoords.dEffectPan.height = 0;
					MachineCoords.dEffectMute.x = 137;
					MachineCoords.dEffectMute.y = 4;
					MachineCoords.dEffectBypass.x = 137;
					MachineCoords.dEffectBypass.y = 15;
					MachineCoords.dEffectName.x = 10;
					MachineCoords.dEffectName.y = 12;
					MachineCoords.bHasTransparency = false;
				#else
					MachineCoords.sMaster.x = 0;
					MachineCoords.sMaster.y = 0;
					MachineCoords.sMaster.width = 148;
					MachineCoords.sMaster.height = 47;//48;

					MachineCoords.sGenerator.x = 0;
					MachineCoords.sGenerator.y = 47;//48;
					MachineCoords.sGenerator.width = 148;
					MachineCoords.sGenerator.height = 47;//48;
					MachineCoords.sGeneratorVu0.x = 0;
					MachineCoords.sGeneratorVu0.y = 141;//144;
					MachineCoords.sGeneratorVu0.width = 7;//6;
					MachineCoords.sGeneratorVu0.height = 4;//5;
					MachineCoords.sGeneratorVuPeak.x = 128;//96;
					MachineCoords.sGeneratorVuPeak.y = 141;//144;
					MachineCoords.sGeneratorVuPeak.width = 2;//6;
					MachineCoords.sGeneratorVuPeak.height = 4;//5;
					MachineCoords.sGeneratorPan.x = 45;//102;
					MachineCoords.sGeneratorPan.y = 145;//144;
					MachineCoords.sGeneratorPan.width = 16;//24;
					MachineCoords.sGeneratorPan.height = 5;//9;
					MachineCoords.sGeneratorMute.x = 0;//133;
					MachineCoords.sGeneratorMute.y = 145;//144;
					MachineCoords.sGeneratorMute.width = 15;//7;
					MachineCoords.sGeneratorMute.height = 14;//7;
					MachineCoords.sGeneratorSolo.x = 15;//140;
					MachineCoords.sGeneratorSolo.y = 145;//144;
					MachineCoords.sGeneratorSolo.width = 15;//7;
					MachineCoords.sGeneratorSolo.height = 14;//7;

					MachineCoords.sEffect.x = 0;
					MachineCoords.sEffect.y = 94;//96;
					MachineCoords.sEffect.width = 148;
					MachineCoords.sEffect.height = 47;//48;
					MachineCoords.sEffectVu0.x = 0;
					MachineCoords.sEffectVu0.y = 141;//144;
					MachineCoords.sEffectVu0.width = 7;//6;
					MachineCoords.sEffectVu0.height = 4;//5;
					MachineCoords.sEffectVuPeak.x = 128;//96;
					MachineCoords.sEffectVuPeak.y = 141;//144;
					MachineCoords.sEffectVuPeak.width = 2;//6;
					MachineCoords.sEffectVuPeak.height = 4;//5;
					MachineCoords.sEffectPan.x = 45;//102;
					MachineCoords.sEffectPan.y = 145;//144;
					MachineCoords.sEffectPan.width = 16;//24;
					MachineCoords.sEffectPan.height = 5;//9;
					MachineCoords.sEffectMute.x = 0;//133;
					MachineCoords.sEffectMute.y = 145;//144;
					MachineCoords.sEffectMute.width = 15;//7;
					MachineCoords.sEffectMute.height = 14;//7;
					MachineCoords.sEffectBypass.x = 30;//126;
					MachineCoords.sEffectBypass.y = 145;//144;
					MachineCoords.sEffectBypass.width = 15;//7;
					MachineCoords.sEffectBypass.height = 14;//13;

					MachineCoords.dGeneratorVu.x = 10;//8;
					MachineCoords.dGeneratorVu.y = 35;//3;
					MachineCoords.dGeneratorVu.width = 130;//96;
					MachineCoords.dGeneratorVu.height = 0;
					MachineCoords.dGeneratorPan.x = 39;//3;
					MachineCoords.dGeneratorPan.y = 26;//35;
					MachineCoords.dGeneratorPan.width = 91;//117;
					MachineCoords.dGeneratorPan.height = 0;
					MachineCoords.dGeneratorMute.x = 11;//137;
					MachineCoords.dGeneratorMute.y = 5;//4;
					MachineCoords.dGeneratorSolo.x = 26;//137;
					MachineCoords.dGeneratorSolo.y = 5;//17;
					MachineCoords.dGeneratorName.x = 49;//10;
					MachineCoords.dGeneratorName.y = 7;//12;

					MachineCoords.dEffectVu.x = 10;//8;
					MachineCoords.dEffectVu.y = 35;//3;
					MachineCoords.dEffectVu.width = 130;//96;
					MachineCoords.dEffectVu.height = 0;
					MachineCoords.dEffectPan.x = 39;//3;
					MachineCoords.dEffectPan.y = 26;//35;
					MachineCoords.dEffectPan.width = 91;//117;
					MachineCoords.dEffectPan.height = 0;
					MachineCoords.dEffectMute.x = 11;//137;
					MachineCoords.dEffectMute.y = 5;//4;
					MachineCoords.dEffectBypass.x = 26;//137;
					MachineCoords.dEffectBypass.y = 5;//15;
					MachineCoords.dEffectName.x = 49;//10;
					MachineCoords.dEffectName.y = 7;//12;
					MachineCoords.bHasTransparency = false;
				#endif
				machineskin.DeleteObject();
				DeleteObject(hbmMachineSkin);
				machineskinmask.DeleteObject();
				machineskin.LoadBitmap(IDB_MACHINE_SKIN);
			}
		}

		void CChildView::FindMachineSkin(CString findDir, CString findName, BOOL *result)
		{
			CFileFind finder;
			int loop = finder.FindFile(findDir + "\\*"); // check for subfolders.
			while (loop) 
			{								
				loop = finder.FindNextFile();
				if (finder.IsDirectory() && !finder.IsDots())
				{
					FindMachineSkin(finder.GetFilePath(),findName,result);
					if ( *result == true) return;
				}
			}
			finder.Close();
			loop = finder.FindFile(findDir + "\\" + findName + ".psm"); // check if the directory is empty
			while (loop)
			{
				loop = finder.FindNextFile();
				if (!finder.IsDirectory())
				{
					CString sName, tmpPath;
					sName = finder.GetFileName();
					// ok so we have a .psm, does it have a valid matching .bmp?
					///\todo [bohan] const_cast for now, not worth fixing it imo without making something more portable anyway
					char* pExt = const_cast<char*>(strrchr(sName,46)); // last .
					pExt[0]=0;
					char szOpenName[MAX_PATH];
					sprintf(szOpenName,"%s\\%s.bmp",findDir,sName);

					machineskin.DeleteObject();
					if( hbmMachineSkin) DeleteObject(hbmMachineSkin);
					machineskinmask.DeleteObject();
					hbmMachineSkin = (HBITMAP)LoadImage(NULL, szOpenName, IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE | LR_LOADFROMFILE);
					if (hbmMachineSkin)
					{
						if (machineskin.Attach(hbmMachineSkin))
						{	
							memset(&MachineCoords,0,sizeof(MachineCoords));
							// load settings
							FILE* hfile;
							sprintf(szOpenName,"%s\\%s.psm",findDir,sName);
							if(!(hfile=fopen(szOpenName,"rb")))
							{
								MessageBox("Couldn't open File for Reading. Operation Aborted","File Open Error",MB_OK);
								return;
							}
							char buf[512];
							while (fgets(buf, 512, hfile))
							{
								if (strstr(buf,"\"master_source\"="))
								{
									char *q = strchr(buf,61); // =
									if (q)
									{
										MachineCoords.sMaster.x = atoi(q+1);
										q = strchr(q+1,44); // ,
										if (q)
										{
											MachineCoords.sMaster.y = atoi(q+1);
											q = strchr(q+1,44); // ,
											if (q)
											{
												MachineCoords.sMaster.width = atoi(q+1);
												q = strchr(q+1,44); // ,
												if (q)
												{
													MachineCoords.sMaster.height = atoi(q+1);
												}
											}
										}
									}
								}
								else if (strstr(buf,"\"generator_source\"="))
								{
									char *q = strchr(buf,61); // =
									if (q)
									{
										MachineCoords.sGenerator.x = atoi(q+1);
										q = strchr(q+1,44); // ,
										if (q)
										{
											MachineCoords.sGenerator.y = atoi(q+1);
											q = strchr(q+1,44); // ,
											if (q)
											{
												MachineCoords.sGenerator.width = atoi(q+1);
												q = strchr(q+1,44); // ,
												if (q)
												{
													MachineCoords.sGenerator.height = atoi(q+1);
												}
											}
										}
									}
								}
								else if (strstr(buf,"\"generator_vu0_source\"="))
								{
									char *q = strchr(buf,61); // =
									if (q)
									{
										MachineCoords.sGeneratorVu0.x = atoi(q+1);
										q = strchr(q+1,44); // ,
										if (q)
										{
											MachineCoords.sGeneratorVu0.y = atoi(q+1);
											q = strchr(q+1,44); // ,
											if (q)
											{
												MachineCoords.sGeneratorVu0.width = atoi(q+1);
												q = strchr(q+1,44); // ,
												if (q)
												{
													MachineCoords.sGeneratorVu0.height = atoi(q+1);
												}
											}
										}
									}
								}
								else if (strstr(buf,"\"generator_vu_peak_source\"="))
								{
									char *q = strchr(buf,61); // =
									if (q)
									{
										MachineCoords.sGeneratorVuPeak.x = atoi(q+1);
										q = strchr(q+1,44); // ,
										if (q)
										{
											MachineCoords.sGeneratorVuPeak.y = atoi(q+1);
											q = strchr(q+1,44); // ,
											if (q)
											{
												MachineCoords.sGeneratorVuPeak.width = atoi(q+1);
												q = strchr(q+1,44); // ,
												if (q)
												{
													MachineCoords.sGeneratorVuPeak.height = atoi(q+1);
												}
											}
										}
									}
								}
								else if (strstr(buf,"\"generator_pan_source\"="))
								{
									char *q = strchr(buf,61); // =
									if (q)
									{
										MachineCoords.sGeneratorPan.x = atoi(q+1);
										q = strchr(q+1,44); // ,
										if (q)
										{
											MachineCoords.sGeneratorPan.y = atoi(q+1);
											q = strchr(q+1,44); // ,
											if (q)
											{
												MachineCoords.sGeneratorPan.width = atoi(q+1);
												q = strchr(q+1,44); // ,
												if (q)
												{
													MachineCoords.sGeneratorPan.height = atoi(q+1);
												}
											}
										}
									}
								}
								else if (strstr(buf,"\"generator_mute_source\"="))
								{
									char *q = strchr(buf,61); // =
									if (q)
									{
										MachineCoords.sGeneratorMute.x = atoi(q+1);
										q = strchr(q+1,44); // ,
										if (q)
										{
											MachineCoords.sGeneratorMute.y = atoi(q+1);
											q = strchr(q+1,44); // ,
											if (q)
											{
												MachineCoords.sGeneratorMute.width = atoi(q+1);
												q = strchr(q+1,44); // ,
												if (q)
												{
													MachineCoords.sGeneratorMute.height = atoi(q+1);
												}
											}
										}
									}
								}
								else if (strstr(buf,"\"generator_solo_source\"="))
								{
									char *q = strchr(buf,61); // =
									if (q)
									{
										MachineCoords.sGeneratorSolo.x = atoi(q+1);
										q = strchr(q+1,44); // ,
										if (q)
										{
											MachineCoords.sGeneratorSolo.y = atoi(q+1);
											q = strchr(q+1,44); // ,
											if (q)
											{
												MachineCoords.sGeneratorSolo.width = atoi(q+1);
												q = strchr(q+1,44); // ,
												if (q)
												{
													MachineCoords.sGeneratorSolo.height = atoi(q+1);
												}
											}
										}
									}
								}
								else if (strstr(buf,"\"effect_source\"="))
								{
									char *q = strchr(buf,61); // =
									if (q)
									{
										MachineCoords.sEffect.x = atoi(q+1);
										q = strchr(q+1,44); // ,
										if (q)
										{
											MachineCoords.sEffect.y = atoi(q+1);
											q = strchr(q+1,44); // ,
											if (q)
											{
												MachineCoords.sEffect.width = atoi(q+1);
												q = strchr(q+1,44); // ,
												if (q)
												{
													MachineCoords.sEffect.height = atoi(q+1);
												}
											}
										}
									}
								}
								else if (strstr(buf,"\"effect_vu0_source\"="))
								{
									char *q = strchr(buf,61); // =
									if (q)
									{
										MachineCoords.sEffectVu0.x = atoi(q+1);
										q = strchr(q+1,44); // ,
										if (q)
										{
											MachineCoords.sEffectVu0.y = atoi(q+1);
											q = strchr(q+1,44); // ,
											if (q)
											{
												MachineCoords.sEffectVu0.width = atoi(q+1);
												q = strchr(q+1,44); // ,
												if (q)
												{
													MachineCoords.sEffectVu0.height = atoi(q+1);
												}
											}
										}
									}
								}
								else if (strstr(buf,"\"effect_vu_peak_source\"="))
								{
									char *q = strchr(buf,61); // =
									if (q)
									{
										MachineCoords.sEffectVuPeak.x = atoi(q+1);
										q = strchr(q+1,44); // ,
										if (q)
										{
											MachineCoords.sEffectVuPeak.y = atoi(q+1);
											q = strchr(q+1,44); // ,
											if (q)
											{
												MachineCoords.sEffectVuPeak.width = atoi(q+1);
												q = strchr(q+1,44); // ,
												if (q)
												{
													MachineCoords.sEffectVuPeak.height = atoi(q+1);
												}
											}
										}
									}
								}
								else if (strstr(buf,"\"effect_pan_source\"="))
								{
									char *q = strchr(buf,61); // =
									if (q)
									{
										MachineCoords.sEffectPan.x = atoi(q+1);
										q = strchr(q+1,44); // ,
										if (q)
										{
											MachineCoords.sEffectPan.y = atoi(q+1);
											q = strchr(q+1,44); // ,
											if (q)
											{
												MachineCoords.sEffectPan.width = atoi(q+1);
												q = strchr(q+1,44); // ,
												if (q)
												{
													MachineCoords.sEffectPan.height = atoi(q+1);
												}
											}
										}
									}
								}
								else if (strstr(buf,"\"effect_mute_source\"="))
								{
									char *q = strchr(buf,61); // =
									if (q)
									{
										MachineCoords.sEffectMute.x = atoi(q+1);
										q = strchr(q+1,44); // ,
										if (q)
										{
											MachineCoords.sEffectMute.y = atoi(q+1);
											q = strchr(q+1,44); // ,
											if (q)
											{
												MachineCoords.sEffectMute.width = atoi(q+1);
												q = strchr(q+1,44); // ,
												if (q)
												{
													MachineCoords.sEffectMute.height = atoi(q+1);
												}
											}
										}
									}
								}
								else if (strstr(buf,"\"effect_bypass_source\"="))
								{
									char *q = strchr(buf,61); // =
									if (q)
									{
										MachineCoords.sEffectBypass.x = atoi(q+1);
										q = strchr(q+1,44); // ,
										if (q)
										{
											MachineCoords.sEffectBypass.y = atoi(q+1);
											q = strchr(q+1,44); // ,
											if (q)
											{
												MachineCoords.sEffectBypass.width = atoi(q+1);
												q = strchr(q+1,44); // ,
												if (q)
												{
													MachineCoords.sEffectBypass.height = atoi(q+1);
												}
											}
										}
									}
								}
								else if (strstr(buf,"\"generator_vu_dest\"="))
								{
									char *q = strchr(buf,61); // =
									if (q)
									{
										MachineCoords.dGeneratorVu.x = atoi(q+1);
										q = strchr(q+1,44); // ,
										if (q)
										{
											MachineCoords.dGeneratorVu.y = atoi(q+1);
											q = strchr(q+1,44); // ,
											if (q)
											{
												MachineCoords.dGeneratorVu.width = atoi(q+1);
												q = strchr(q+1,44); // ,
												if (q)
												{
													MachineCoords.dGeneratorVu.height = atoi(q+1);
												}
											}
										}
									}
								}
								else if (strstr(buf,"\"generator_pan_dest\"="))
								{
									char *q = strchr(buf,61); // =
									if (q)
									{
										MachineCoords.dGeneratorPan.x = atoi(q+1);
										q = strchr(q+1,44); // ,
										if (q)
										{
											MachineCoords.dGeneratorPan.y = atoi(q+1);
											q = strchr(q+1,44); // ,
											if (q)
											{
												MachineCoords.dGeneratorPan.width = atoi(q+1);
												q = strchr(q+1,44); // ,
												if (q)
												{
													MachineCoords.dGeneratorPan.height = atoi(q+1);
												}
											}
										}
									}
								}
								else if (strstr(buf,"\"generator_mute_dest\"="))
								{
									char *q = strchr(buf,61); // =
									if (q)
									{
										MachineCoords.dGeneratorMute.x = atoi(q+1);
										q = strchr(q+1,44); // ,
										if (q)
										{
											MachineCoords.dGeneratorMute.y = atoi(q+1);
										}
									}
								}
								else if (strstr(buf,"\"generator_solo_dest\"="))
								{
									char *q = strchr(buf,61); // =
									if (q)
									{
										MachineCoords.dGeneratorSolo.x = atoi(q+1);
										q = strchr(q+1,44); // ,
										if (q)
										{
											MachineCoords.dGeneratorSolo.y = atoi(q+1);
										}
									}
								}
								else if (strstr(buf,"\"generator_name_dest\"="))
								{
									char *q = strchr(buf,61); // =
									if (q)
									{
										MachineCoords.dGeneratorName.x = atoi(q+1);
										q = strchr(q+1,44); // ,
										if (q)
										{
											MachineCoords.dGeneratorName.y = atoi(q+1);
										}
									}
								}
								else if (strstr(buf,"\"effect_vu_dest\"="))
								{
									char *q = strchr(buf,61); // =
									if (q)
									{
										MachineCoords.dEffectVu.x = atoi(q+1);
										q = strchr(q+1,44); // ,
										if (q)
										{
											MachineCoords.dEffectVu.y = atoi(q+1);
											q = strchr(q+1,44); // ,
											if (q)
											{
												MachineCoords.dEffectVu.width = atoi(q+1);
												q = strchr(q+1,44); // ,
												if (q)
												{
													MachineCoords.dEffectVu.height = atoi(q+1);
												}
											}
										}
									}
								}
								else if (strstr(buf,"\"effect_pan_dest\"="))
								{
									char *q = strchr(buf,61); // =
									if (q)
									{
										MachineCoords.dEffectPan.x = atoi(q+1);
										q = strchr(q+1,44); // ,
										if (q)
										{
											MachineCoords.dEffectPan.y = atoi(q+1);
											q = strchr(q+1,44); // ,
											if (q)
											{
												MachineCoords.dEffectPan.width = atoi(q+1);
												q = strchr(q+1,44); // ,
												if (q)
												{
													MachineCoords.dEffectPan.height = atoi(q+1);
												}
											}
										}
									}
								}
								else if (strstr(buf,"\"effect_mute_dest\"="))
								{
									char *q = strchr(buf,61); // =
									if (q)
									{
										MachineCoords.dEffectMute.x = atoi(q+1);
										q = strchr(q+1,44); // ,
										if (q)
										{
											MachineCoords.dEffectMute.y = atoi(q+1);
										}
									}
								}
								else if (strstr(buf,"\"effect_bypass_dest\"="))
								{
									char *q = strchr(buf,61); // =
									if (q)
									{
										MachineCoords.dEffectBypass.x = atoi(q+1);
										q = strchr(q+1,44); // ,
										if (q)
										{
											MachineCoords.dEffectBypass.y = atoi(q+1);
										}
									}
								}
								else if (strstr(buf,"\"effect_name_dest\"="))
								{
									char *q = strchr(buf,61); // =
									if (q)
									{
										MachineCoords.dEffectName.x = atoi(q+1);
										q = strchr(q+1,44); // ,
										if (q)
										{
											MachineCoords.dEffectName.y = atoi(q+1);
										}
									}
								}
								else if (strstr(buf,"\"transparency\"="))
								{
									char *q = strchr(buf,61); // =
									if (q)
									{
										hexstring_to_integer(q+1, MachineCoords.cTransparency);
										MachineCoords.bHasTransparency = true;
									}
								}
							}
							if (MachineCoords.bHasTransparency)
							{
								PrepareMask(&machineskin,&machineskinmask,MachineCoords.cTransparency);
							}
							fclose(hfile);
							*result = true;
							break;
						}
					}
				}
			}
			finder.Close();
		}

		void CChildView::LoadPatternHeaderSkin()
		{
			std::string szOld;
			if (!Global::configuration().pattern_header_skin.empty())
			{
				szOld = Global::configuration().pattern_header_skin;
				// ok so...
				if (szOld != std::string(PSYCLE__PATH__DEFAULT_PATTERN_HEADER_SKIN))
				{
					BOOL result = false;
					FindPatternHeaderSkin(Global::configuration().GetSkinDir().c_str(),Global::configuration().pattern_header_skin.c_str(), &result);
					if (result)
					{
						return;
					}
				}
				// load defaults
				szOld = PSYCLE__PATH__DEFAULT_PATTERN_HEADER_SKIN;
				// and coords
				#if defined PSYCLE__PATH__CONFIGURATION__SKIN__UGLY_DEFAULT
					PatHeaderCoords.sBackground.x=0;
					PatHeaderCoords.sBackground.y=0;
					PatHeaderCoords.sBackground.width=109;
					PatHeaderCoords.sBackground.height=16;
					PatHeaderCoords.sNumber0.x = 0;
					PatHeaderCoords.sNumber0.y = 16;
					PatHeaderCoords.sNumber0.width = 7;
					PatHeaderCoords.sNumber0.height = 12;
					PatHeaderCoords.sRecordOn.x = 70;
					PatHeaderCoords.sRecordOn.y = 16;
					PatHeaderCoords.sRecordOn.width = 7;
					PatHeaderCoords.sRecordOn.height = 7;
					PatHeaderCoords.sMuteOn.x = 77;
					PatHeaderCoords.sMuteOn.y = 16;
					PatHeaderCoords.sMuteOn.width = 7;
					PatHeaderCoords.sMuteOn.height = 7;
					PatHeaderCoords.sSoloOn.x = 84;
					PatHeaderCoords.sSoloOn.y = 16;
					PatHeaderCoords.sSoloOn.width = 7;
					PatHeaderCoords.sSoloOn.height = 7;
					PatHeaderCoords.dDigitX0.x = 23;
					PatHeaderCoords.dDigitX0.y = 2;
					PatHeaderCoords.dDigit0X.x = 30;
					PatHeaderCoords.dDigit0X.y = 2;
					PatHeaderCoords.dRecordOn.x = 52;
					PatHeaderCoords.dRecordOn.y = 5;
					PatHeaderCoords.dMuteOn.x = 75;
					PatHeaderCoords.dMuteOn.y = 5;
					PatHeaderCoords.dSoloOn.x = 96;
					PatHeaderCoords.dSoloOn.y = 5;
					PatHeaderCoords.bHasTransparency = false;
				#else
					PatHeaderCoords.sBackground.x=0;
					PatHeaderCoords.sBackground.y=0;
					PatHeaderCoords.sBackground.width=109;
					PatHeaderCoords.sBackground.height=18;//16
					PatHeaderCoords.sNumber0.x = 0;
					PatHeaderCoords.sNumber0.y = 18;//16
					PatHeaderCoords.sNumber0.width = 7;
					PatHeaderCoords.sNumber0.height = 12;
					PatHeaderCoords.sRecordOn.x = 70;
					PatHeaderCoords.sRecordOn.y = 18;//16
					PatHeaderCoords.sRecordOn.width = 11;//7;
					PatHeaderCoords.sRecordOn.height = 11;//7;
					PatHeaderCoords.sMuteOn.x = 81;//77;
					PatHeaderCoords.sMuteOn.y = 18;//16;
					PatHeaderCoords.sMuteOn.width = 11;//7;
					PatHeaderCoords.sMuteOn.height = 11;//7;
					PatHeaderCoords.sSoloOn.x = 92;//84;
					PatHeaderCoords.sSoloOn.y = 18;//16;
					PatHeaderCoords.sSoloOn.width = 11;//7;
					PatHeaderCoords.sSoloOn.height = 11;//7;
					PatHeaderCoords.dDigitX0.x = 24;//22;
					PatHeaderCoords.dDigitX0.y = 3;//2;
					PatHeaderCoords.dDigit0X.x = 31;//29;
					PatHeaderCoords.dDigit0X.y = 3;//2;
					PatHeaderCoords.dRecordOn.x = 52;
					PatHeaderCoords.dRecordOn.y = 3;//5;
					PatHeaderCoords.dMuteOn.x = 75;
					PatHeaderCoords.dMuteOn.y = 3;//5;
					PatHeaderCoords.dSoloOn.x = 97;//96;
					PatHeaderCoords.dSoloOn.y = 3;//5;
					PatHeaderCoords.bHasTransparency = false;
				#endif
				patternheader.DeleteObject();
				DeleteObject(hbmPatHeader);
				patternheadermask.DeleteObject();
				patternheader.LoadBitmap(IDB_PATTERN_HEADER_SKIN);
			}
		}

		void CChildView::FindPatternHeaderSkin(CString findDir, CString findName, BOOL *result)
		{
			CFileFind finder;
			int loop = finder.FindFile(findDir + "\\*");	// check for subfolders.
			while (loop) 
			{		
				loop = finder.FindNextFile();
				if (finder.IsDirectory() && !finder.IsDots())
				{
					FindPatternHeaderSkin(finder.GetFilePath(),findName,result);
				}
			}
			finder.Close();
			loop = finder.FindFile(findDir + "\\" + findName + ".psh"); // check if the directory is empty
			while (loop)
			{
				loop = finder.FindNextFile();
				if (!finder.IsDirectory())
				{
					CString sName, tmpPath;
					sName = finder.GetFileName();
					// ok so we have a .psh, does it have a valid matching .bmp?
					///\todo [bohan] const_cast for now, not worth fixing it imo without making something more portable anyway
					char* pExt = const_cast<char*>(strrchr(sName,46)); // last .
					pExt[0]=0;
					char szOpenName[MAX_PATH];
					std::sprintf(szOpenName,"%s\\%s.bmp",findDir,sName);
					patternheader.DeleteObject();
					if (hbmPatHeader)DeleteObject(hbmPatHeader);
					patternheadermask.DeleteObject();
					hbmPatHeader = (HBITMAP)LoadImage(NULL, szOpenName, IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE | LR_LOADFROMFILE);
					if (hbmPatHeader)
					{
						if (patternheader.Attach(hbmPatHeader))
						{	
							memset(&PatHeaderCoords,0,sizeof(PatHeaderCoords));
							// load settings
							FILE* hfile;
							sprintf(szOpenName,"%s\\%s.psh",findDir,sName);
							if(!(hfile=fopen(szOpenName,"rb")))
							{
								MessageBox("Couldn't open File for Reading. Operation Aborted","File Open Error",MB_OK);
								return;
							}
							char buf[512];
							while (fgets(buf, 512, hfile))
							{
								if (strstr(buf,"\"background_source\"="))
								{
									char *q = strchr(buf,61); // =
									if (q)
									{
										PatHeaderCoords.sBackground.x = atoi(q+1);
										q = strchr(q+1,44); // ,
										if (q)
										{
											PatHeaderCoords.sBackground.y = atoi(q+1);
											q = strchr(q+1,44); // ,
											if (q)
											{
												PatHeaderCoords.sBackground.width = atoi(q+1);
												q = strchr(q+1,44); // ,
												if (q)
												{
													PatHeaderCoords.sBackground.height = atoi(q+1);
												}
											}
										}
									}
								}
								else if (strstr(buf,"\"number_0_source\"="))
								{
									char *q = strchr(buf,61); // =
									if (q)
									{
										PatHeaderCoords.sNumber0.x = atoi(q+1);
										q = strchr(q+1,44); // ,
										if (q)
										{
											PatHeaderCoords.sNumber0.y = atoi(q+1);
											q = strchr(q+1,44); // ,
											if (q)
											{
												PatHeaderCoords.sNumber0.width = atoi(q+1);
												q = strchr(q+1,44); // ,
												if (q)
												{
													PatHeaderCoords.sNumber0.height = atoi(q+1);
												}
											}
										}
									}
								}
								else if (strstr(buf,"\"record_on_source\"="))
								{
									char *q = strchr(buf,61); // =
									if (q)
									{
										PatHeaderCoords.sRecordOn.x = atoi(q+1);
										q = strchr(q+1,44); // ,
										if (q)
										{
											PatHeaderCoords.sRecordOn.y = atoi(q+1);
											q = strchr(q+1,44); // ,
											if (q)
											{
												PatHeaderCoords.sRecordOn.width = atoi(q+1);
												q = strchr(q+1,44); // ,
												if (q)
												{
													PatHeaderCoords.sRecordOn.height = atoi(q+1);
												}
											}
										}
									}
								}
								else if (strstr(buf,"\"mute_on_source\"="))
								{
									char *q = strchr(buf,61); // =
									if (q)
									{
										PatHeaderCoords.sMuteOn.x = atoi(q+1);
										q = strchr(q+1,44); // ,
										if (q)
										{
											PatHeaderCoords.sMuteOn.y = atoi(q+1);
											q = strchr(q+1,44); // ,
											if (q)
											{
												PatHeaderCoords.sMuteOn.width = atoi(q+1);
												q = strchr(q+1,44); // ,
												if (q)
												{
													PatHeaderCoords.sMuteOn.height = atoi(q+1);
												}
											}
										}
									}
								}
								else if (strstr(buf,"\"solo_on_source\"="))
								{
									char *q = strchr(buf,61); // =
									if (q)
									{
										PatHeaderCoords.sSoloOn.x = atoi(q+1);
										q = strchr(q+1,44); // ,
										if (q)
										{
											PatHeaderCoords.sSoloOn.y = atoi(q+1);
											q = strchr(q+1,44); // ,
											if (q)
											{
												PatHeaderCoords.sSoloOn.width = atoi(q+1);
												q = strchr(q+1,44); // ,
												if (q)
												{
													PatHeaderCoords.sSoloOn.height = atoi(q+1);
												}
											}
										}
									}
								}
								else if (strstr(buf,"\"digit_x0_dest\"="))
								{
									char *q = strchr(buf,61); // =
									if (q)
									{
										PatHeaderCoords.dDigitX0.x = atoi(q+1);
										q = strchr(q+1,44); // ,
										if (q)
										{
											PatHeaderCoords.dDigitX0.y = atoi(q+1);
										}
									}
								}
								else if (strstr(buf,"\"digit_0x_dest\"="))
								{
									char *q = strchr(buf,61); // =
									if (q)
									{
										PatHeaderCoords.dDigit0X.x = atoi(q+1);
										q = strchr(q+1,44); // ,
										if (q)
										{
											PatHeaderCoords.dDigit0X.y = atoi(q+1);
										}
									}
								}
								else if (strstr(buf,"\"record_on_dest\"="))
								{
									char *q = strchr(buf,61); // =
									if (q)
									{
										PatHeaderCoords.dRecordOn.x = atoi(q+1);
										q = strchr(q+1,44); // ,
										if (q)
										{
											PatHeaderCoords.dRecordOn.y = atoi(q+1);
										}
									}
								}
								else if (strstr(buf,"\"mute_on_dest\"="))
								{
									char *q = strchr(buf,61); // =
									if (q)
									{
										PatHeaderCoords.dMuteOn.x = atoi(q+1);
										q = strchr(q+1,44); // ,
										if (q)
										{
											PatHeaderCoords.dMuteOn.y = atoi(q+1);
										}
									}
								}
								else if (strstr(buf,"\"solo_on_dest\"="))
								{
									char *q = strchr(buf,61); // =
									if (q)
									{
										PatHeaderCoords.dSoloOn.x = atoi(q+1);
										q = strchr(q+1,44); // ,
										if (q)
										{
											PatHeaderCoords.dSoloOn.y = atoi(q+1);
										}
									}
								}
								else if (strstr(buf,"\"transparency\"="))
								{
									char *q = strchr(buf,61); // =
									if (q)
									{
										hexstring_to_integer(q+1, PatHeaderCoords.cTransparency);
										PatHeaderCoords.bHasTransparency = true;
									}
								}
							}
							if (PatHeaderCoords.bHasTransparency)
							{
								PrepareMask(&patternheader,&patternheadermask,PatHeaderCoords.cTransparency);
							}
							fclose(hfile);
							*result = true;
							break;
						}
					}
				}
			}
			finder.Close();
		}


		void CChildView::RecalcMetrics()
		{
			if (Global::configuration().pattern_draw_empty_data)
			{
				sprintf(szBlankParam,".");
				sprintf(szBlankNote,"---");
			}
			else
			{
				sprintf(szBlankParam," ");
				sprintf(szBlankNote,"   ");
			}
			TEXTHEIGHT = Global::configuration().pattern_font_y;
			ROWHEIGHT = TEXTHEIGHT+1;
			TEXTWIDTH = Global::configuration().pattern_font_x;
			for (int c=0; c<256; c++)	
			{ 
				FLATSIZES[c]=Global::configuration().pattern_font_x; 
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
			HEADER_ROWWIDTH = PatHeaderCoords.sBackground.width+1;
			HEADER_HEIGHT = PatHeaderCoords.sBackground.height+2;
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
			if (Global::configuration()._linenumbers)
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
			triangle_size_tall = Global::configuration().mv_triangle_size+((23*Global::configuration().mv_wirewidth)/16);

			triangle_size_center = triangle_size_tall/2;
			triangle_size_wide = triangle_size_tall/2;
			triangle_size_indent = triangle_size_tall/6;
		}

		void CChildView::PrepareMask(CBitmap* pBmpSource, CBitmap* pBmpMask, COLORREF clrTrans)
		{
			BITMAP bm;
			// Get the dimensions of the source bitmap
			pBmpSource->GetObject(sizeof(BITMAP), &bm);
			// Create the mask bitmap
			pBmpMask->DeleteObject();
			pBmpMask->CreateBitmap( bm.bmWidth, bm.bmHeight, 1, 1, NULL);
			// We will need two DCs to work with. One to hold the Image
			// (the source), and one to hold the mask (destination).
			// When blitting onto a monochrome bitmap from a color, pixels
			// in the source color bitmap that are equal to the background
			// color are blitted as white. All the remaining pixels are
			// blitted as black.
			CDC hdcSrc, hdcDst;
			hdcSrc.CreateCompatibleDC(NULL);
			hdcDst.CreateCompatibleDC(NULL);
			// Load the bitmaps into memory DC
			CBitmap* hbmSrcT = (CBitmap*) hdcSrc.SelectObject(pBmpSource);
			CBitmap* hbmDstT = (CBitmap*) hdcDst.SelectObject(pBmpMask);
			// Change the background to trans color
			hdcSrc.SetBkColor(clrTrans);
			// This call sets up the mask bitmap.
			hdcDst.BitBlt(0,0,bm.bmWidth, bm.bmHeight, &hdcSrc,0,0,SRCCOPY);
			// Now, we need to paint onto the original image, making
			// sure that the "transparent" area is set to black. What
			// we do is AND the monochrome image onto the color Image
			// first. When blitting from mono to color, the monochrome
			// pixel is first transformed as follows:
			// if  1 (black) it is mapped to the color set by SetTextColor().
			// if  0 (white) is is mapped to the color set by SetBkColor().
			// Only then is the raster operation performed.
			hdcSrc.SetTextColor(RGB(255,255,255));
			hdcSrc.SetBkColor(RGB(0,0,0));
			hdcSrc.BitBlt(0,0,bm.bmWidth, bm.bmHeight, &hdcDst,0,0,SRCAND);
			// Clean up by deselecting any objects, and delete the
			// DC's.
			hdcSrc.SelectObject(hbmSrcT);
			hdcDst.SelectObject(hbmDstT);
			hdcSrc.DeleteDC();
			hdcDst.DeleteDC();
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
						_pSong->_trackMuted[i] = false;
					}
					_pSong->_trackSoloed = -1;
				}
				else
				{
					for (int i = 0; i < MAX_TRACKS; i++)
					{
						_pSong->_trackMuted[i] = true;
					}
					_pSong->_trackMuted[editcur.track] = false;
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

		void CChildView::DoMacPropDialog(Machine::id_type propMac)
		{
			if((propMac < 0 ) || (propMac >= MAX_MACHINES-1)) return;
			CMacProp dlg;
			dlg.m_view=this;
			dlg.pMachine = Global::song()._pMachine[propMac];
			dlg.pSong = &Global::song();
			dlg.thisMac = propMac;
			if(dlg.DoModal() == IDOK)
			{
				dlg.pMachine->SetEditName(dlg.txt);
				pParentMain->StatusBarText(dlg.txt.c_str());
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
				Global::song().DestroyMachine(propMac);
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
			Global::player()._loopSong = !Global::player()._loopSong;
		}

		void CChildView::OnUpdateConfigurationLoopplayback(CCmdUI* pCmdUI) 
		{
			if (Global::player()._loopSong)
				pCmdUI->SetCheck(1);
			else
				pCmdUI->SetCheck(0);	
		}

		void CChildView::LoadMachineDial()
		{
			machinedial.DeleteObject();
			if ( hbmMachineDial) DeleteObject(hbmMachineDial);
			if (Global::configuration().bBmpDial)
			{
				Global::configuration().bBmpDial=false;
				hbmMachineDial = (HBITMAP)LoadImage(NULL, Global::configuration().szBmpDialFilename.c_str(), IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE | LR_LOADFROMFILE);
				if (hbmMachineDial)
				{
					if (machinedial.Attach(hbmMachineDial))
					{	
						BITMAP bm;
						GetObject(hbmMachineDial,sizeof(BITMAP),&bm);

						if ((bm.bmWidth == 1792) && (bm.bmHeight == 28))
						{
							Global::configuration().bBmpDial=true;
						}
						else
							machinedial.LoadBitmap(IDB_KNOB);
					}
				}
			}
			else 
				machinedial.LoadBitmap(IDB_KNOB);
		}


		void CChildView::LoadMachineBackground()
		{
			machinebkg.DeleteObject();
			if ( hbmMachineBkg) DeleteObject(hbmMachineBkg);
			if (Global::configuration().bBmpBkg)
			{
				Global::configuration().bBmpBkg=false;
				hbmMachineBkg = (HBITMAP)LoadImage(NULL, Global::configuration().szBmpBkgFilename.c_str(), IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE | LR_LOADFROMFILE);
				if (hbmMachineBkg)
				{
					if (machinebkg.Attach(hbmMachineBkg))
					{	
						BITMAP bm;
						GetObject(hbmMachineBkg,sizeof(BITMAP),&bm);

						bkgx=bm.bmWidth;
						bkgy=bm.bmHeight;

						if ((bkgx > 0) && (bkgy > 0))
						{
							Global::configuration().bBmpBkg=true;
						}
					}
				}
			}
		}

	UNIVERSALIS__COMPILER__NAMESPACE__END
UNIVERSALIS__COMPILER__NAMESPACE__END

// graphics operations, private headers
#include "machineview.private.hpp"
#include "patviewnew.private.hpp"
#include "seqview.private.hpp"

// User/Mouse Responses, private headers
#include "keybhandler.private.hpp"
#include "mouseHandler.private.hpp"
