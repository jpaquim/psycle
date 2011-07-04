///\file
///\brief implementation file for psycle::host::CMainFrame.

#include "MainFrm.hpp"

#include "InputHandler.hpp"
#include "MidiInput.hpp"

#include "GearRackDlg.hpp"
#include "KeyConfigDlg.hpp"

#include "MasterDlg.hpp"
#include "GearTracker.hpp"
#include "XMSamplerUI.hpp"
#include "FrameMachine.hpp"
#include "VstEffectWnd.hpp"
#include "WaveInMacDlg.hpp"
#include "WireDlg.hpp"
#include "WaveEdFrame.hpp"

#include "Player.hpp"
#include "Plugin.hpp"
#include "VstHost24.hpp"
#include "XMSampler.hpp"

#include <cmath>
#include <sstream>
#include <iomanip>

#include <HtmlHelp.h>

namespace psycle { namespace host {

	extern CPsycleApp theApp;

		#define WM_SETMESSAGESTRING 0x0362

		IMPLEMENT_DYNAMIC(CMainFrame, CFrameWnd)

		static UINT indicators[] =
		{
			ID_SEPARATOR,           // status line indicator
			ID_INDICATOR_SEQPOS,
			ID_INDICATOR_PATTERN,
			ID_INDICATOR_LINE,
			ID_INDICATOR_TIME,
			ID_INDICATOR_OCTAVE,
			ID_INDICATOR_EDIT,
			ID_INDICATOR_FOLLOW,
			ID_INDICATOR_NOTEOFF,
			ID_INDICATOR_TWEAKS,
		};

		CMainFrame::CMainFrame()
		{
			Global::pInputHandler->SetMainFrame(this);
			_pSong = 0;
			pGearRackDialog = 0;
			for(int c=0;c<MAX_MACHINES;c++) isguiopen[c]=false;
		}

		CMainFrame::~CMainFrame()
		{
			Global::pInputHandler->SetMainFrame(NULL);
		}

		BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
			ON_MESSAGE (WM_SETMESSAGESTRING, OnSetMessageString)
			ON_WM_CREATE()
			ON_WM_SETFOCUS()
			ON_WM_CLOSE()
			ON_WM_ACTIVATE()
			ON_WM_DESTROY()
			ON_WM_DROPFILES()
//songbar start
			ON_CBN_SELCHANGE(IDC_TRACKCOMBO, OnSelchangeTrackcombo)
			ON_CBN_CLOSEUP(IDC_TRACKCOMBO, OnCloseupTrackcombo)
			ON_BN_CLICKED(IDC_BPM_ADDONE, OnBpmAddOne)
			ON_BN_CLICKED(IDC_BPM_ADDTEN, OnBpmAddTen)
			ON_BN_CLICKED(IDC_BPM_DECONE, OnBpmDecOne)
			ON_BN_CLICKED(IDC_BPM_DECTEN, OnBpmDecTen)
			ON_BN_CLICKED(IDC_DEC_TPB, OnDecTPB)
			ON_BN_CLICKED(IDC_INC_TPB, OnIncTPB)
			ON_CBN_CLOSEUP(IDC_COMBOOCTAVE, OnCloseupCombooctave)
			ON_CBN_SELCHANGE(IDC_COMBOOCTAVE, OnSelchangeCombooctave)
			ON_BN_CLICKED(IDC_CLIPBUT, OnClipbut)
			ON_NOTIFY_EX(TTN_NEEDTEXT, 0, OnToolTipNotify)
//Machinebar start
			ON_CBN_SELCHANGE(IDC_COMBOSTEP, OnSelchangeCombostep)
			ON_CBN_CLOSEUP(IDC_COMBOSTEP, OnCloseupCombostep)
			ON_CBN_SELCHANGE(IDC_BAR_COMBOGEN, OnSelchangeBarCombogen)
			ON_CBN_CLOSEUP(IDC_BAR_COMBOGEN, OnCloseupBarCombogen)
			ON_CBN_CLOSEUP(IDC_AUXSELECT, OnCloseupAuxselect)
			ON_CBN_SELCHANGE(IDC_AUXSELECT, OnSelchangeAuxselect)
			ON_CBN_SELCHANGE(IDC_BAR_COMBOINS, OnSelchangeBarComboins)
			ON_CBN_CLOSEUP(IDC_BAR_COMBOINS, OnCloseupBarComboins)
			ON_BN_CLICKED(IDC_B_DECGEN, OnBDecgen)
			ON_BN_CLICKED(IDC_B_INCGEN, OnBIncgen)
			ON_BN_CLICKED(IDC_GEAR_RACK, OnGearRack)
			ON_BN_CLICKED(IDC_B_DECWAV, OnBDecwav)
			ON_BN_CLICKED(IDC_B_INCWAV, OnBIncwav)
			ON_BN_CLICKED(IDC_LOADWAVE, OnLoadwave)
			ON_BN_CLICKED(IDC_SAVEWAVE, OnSavewave)
			ON_BN_CLICKED(IDC_EDITWAVE, OnEditwave)
			ON_BN_CLICKED(IDC_WAVEBUT, OnWavebut)
//Machinebar end
//seqbar start
			ON_LBN_SELCHANGE(IDC_SEQLIST, OnSelchangeSeqlist)
			ON_LBN_DBLCLK(IDC_SEQLIST, OnDblclkSeqlist)
			ON_BN_CLICKED(IDC_INCSHORT, OnIncshort)
			ON_BN_CLICKED(IDC_DECSHORT, OnDecshort)
			ON_BN_CLICKED(IDC_SEQNEW, OnSeqnew)
			ON_BN_CLICKED(IDC_SEQDUPLICATE, OnSeqduplicate)
			ON_BN_CLICKED(IDC_SEQINS, OnSeqins)
			ON_BN_CLICKED(IDC_SEQDELETE, OnSeqdelete)
			//Popup menu of seqbar						
			ON_COMMAND(ID__SEQRENAME, OnSeqrename)
			ON_COMMAND(ID__SEQCHANGEPOS, OnSeqchange)
			ON_COMMAND(IDC_SEQCUT, OnSeqcut)
			ON_COMMAND(IDC_SEQCOPY, OnSeqcopy)
			ON_COMMAND(IDC_SEQPASTE, OnSeqpaste)
			ON_COMMAND(ID__SEQPASTEBELOW, OnSeqpasteBelow)
			ON_COMMAND(ID__SEQSORT, OnSeqsort)
			ON_COMMAND(ID__SEQCLEAR, OnSeqclear)
			ON_UPDATE_COMMAND_UI(IDC_SEQPASTE, OnUpdatepaste)
			ON_UPDATE_COMMAND_UI(ID__SEQPASTEBELOW, OnUpdatepasteBelow)
			//Popup menu end
			ON_BN_CLICKED(IDC_FOLLOW, OnFollow)
			ON_BN_CLICKED(IDC_RECORD_NOTEOFF, OnRecordNoteoff)
			ON_BN_CLICKED(IDC_RECORD_TWEAKS, OnRecordTweaks)
			ON_BN_CLICKED(IDC_SHOWPATTERNAME, OnShowpattername)
			ON_BN_CLICKED(IDC_MULTICHANNEL_AUDITION, OnMultichannelAudition)
			ON_BN_CLICKED(IDC_NOTESTOEFFECTS, OnNotestoeffects)
			ON_BN_CLICKED(IDC_MOVECURSORPASTE, OnMovecursorpaste)
//seqbar end
//statusbar start
			ON_UPDATE_COMMAND_UI(ID_INDICATOR_SEQPOS, OnUpdateIndicatorSeqPos)
			ON_UPDATE_COMMAND_UI(ID_INDICATOR_PATTERN, OnUpdateIndicatorPattern)
			ON_UPDATE_COMMAND_UI(ID_INDICATOR_LINE, OnUpdateIndicatorLine)
			ON_UPDATE_COMMAND_UI(ID_INDICATOR_TIME, OnUpdateIndicatorTime)
			ON_UPDATE_COMMAND_UI(ID_INDICATOR_EDIT, OnUpdateIndicatorEdit)
			ON_UPDATE_COMMAND_UI(ID_INDICATOR_FOLLOW, OnUpdateIndicatorFollow)
			ON_UPDATE_COMMAND_UI(ID_INDICATOR_NOTEOFF, OnUpdateIndicatorNoteoff)
			ON_UPDATE_COMMAND_UI(ID_INDICATOR_TWEAKS, OnUpdateIndicatorTweaks)
			ON_UPDATE_COMMAND_UI(ID_INDICATOR_OCTAVE, OnUpdateIndicatorOctave)
//statusbar end
//menu start
			ON_COMMAND(ID_APP_EXIT, OnClose)
			ON_COMMAND(ID_VIEW_SONGBAR, OnViewSongbar)
			ON_COMMAND(ID_VIEW_MACHINEBAR, OnViewMachinebar)
			ON_COMMAND(ID_VIEW_SEQUENCERBAR, OnViewSequencerbar)
			ON_UPDATE_COMMAND_UI(ID_VIEW_SONGBAR, OnUpdateViewSongbar)
			ON_UPDATE_COMMAND_UI(ID_VIEW_SEQUENCERBAR, OnUpdateViewSequencerbar)
			ON_UPDATE_COMMAND_UI(ID_VIEW_MACHINEBAR, OnUpdateViewMachinebar)
			ON_COMMAND(ID_PSYHELP, OnPsyhelp)
//menu end
		END_MESSAGE_MAP()


		int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
		{
			if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
				return -1;

			_pSong=Global::_pSong;

			// create a view to occupy the client area of the frame
			m_wndView.pParentFrame = this;
			if (!m_wndView.Create(NULL, NULL, AFX_WS_DEFAULT_VIEW,
				CRect(0, 0, 0, 0), this, AFX_IDW_PANE_FIRST, NULL))
			{
				TRACE0("Failed to create view window\n");
				return -1;
			}
			m_wndView.ValidateParent();
			// Create Toolbars.
			//m_rebar.Create(this);
			//m_rebar.SetBarStyle(m_rebar.GetBarStyle() | CBRS_FLYBY);
			if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT|TBSTYLE_LIST|TBSTYLE_TRANSPARENT|TBSTYLE_TOOLTIPS|TBSTYLE_WRAPABLE) ||
				!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
			{
				TRACE0("Failed to create toolbar\n");
				return -1;      // fail to create
			}
			m_wndToolBar.SetBarStyle(m_wndToolBar.GetBarStyle() | CBRS_FLYBY | CBRS_GRIPPER);
			m_songBar.InitializeValues(this, &m_wndView, _pSong);
			if (!m_songBar.Create(this, IDD_SONGBAR, CBRS_TOP|CBRS_FLYBY|CBRS_GRIPPER, IDD_SONGBAR))
			{
				TRACE0("Failed to create songbar\n");
				return -1;		// fail to create
			}
			m_machineBar.InitializeValues(this, &m_wndView, _pSong);
			if (!m_machineBar.Create(this, IDD_MACHINEBAR, CBRS_TOP|CBRS_FLYBY|CBRS_GRIPPER, IDD_MACHINEBAR))
			{
				TRACE0("Failed to create machinebar\n");
				return -1;		// fail to create
			}
			// Status bar
			if (!m_wndStatusBar.Create(this) ||
				!m_wndStatusBar.SetIndicators(indicators, sizeof(indicators)/sizeof(UINT)))
			{
				TRACE0("Failed to create status bar\n");
				return -1;      // fail to create
			}

			// sequence bar (created after status bar since UpdatePlayOrder refreshes the status bar)
			m_seqBar.InitializeValues(this, &m_wndView, _pSong);
			if (!m_seqBar.Create(this,IDD_SEQUENCER,CBRS_LEFT|CBRS_FLYBY| CBRS_GRIPPER, IDD_SEQUENCER))
			{
				TRACE0("Failed to create sequencebar\n");
				return -1;		// fail to create
			}
			
			// CPU info Window
			m_wndInfo.Create(IDD_INFO,this);

			// MIDI monitor Dialog
			m_midiMonitorDlg.Create(IDD_MIDI_MONITOR,this);
			
			// Instrument editor
			m_wndInst._pSong=_pSong;
			m_wndInst.pParentMain=this;
			m_wndInst.Create(IDD_INSTRUMENT,this);
			m_wndInst.Validate();
			// Wave Editor Window
			m_pWndWed = new CWaveEdFrame(this->_pSong,this);
			m_pWndWed->LoadFrame(IDR_WAVEFRAME ,WS_OVERLAPPEDWINDOW, this);
			m_pWndWed->GenerateView();

			DragAcceptFiles(true);

			HMENU hFileMenu = GetSubMenu(::GetMenu(m_hWnd), 0);
			m_wndView.hRecentMenu = GetSubMenu(hFileMenu, 11);

			/*
			int buttons = m_wndToolBar.GetToolBarCtrl().GetButtonCount();
			for(int i=0;i<buttons;i++) {
				if(!(m_wndToolBar.GetButtonStyle(i)&BTNS_SEP)) {
					m_wndToolBar.SetButtonStyle(i, m_wndToolBar.GetButtonStyle(i)|BTNS_AUTOSIZE|BTNS_SHOWTEXT);
				}
			}
			*/

			m_wndToolBar.SetWindowText("Psycle tool bar");
			m_songBar.SetWindowText("Psycle song controls bar");
			m_machineBar.SetWindowText("Psycle machine controls bar");
			m_seqBar.SetWindowText("Psycle sequence controls bar");

			m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
			m_songBar.EnableDocking(CBRS_ALIGN_TOP|CBRS_ALIGN_BOTTOM);
			m_machineBar.EnableDocking(CBRS_ALIGN_TOP|CBRS_ALIGN_BOTTOM);
			m_seqBar.EnableDocking(CBRS_ALIGN_LEFT|CBRS_ALIGN_RIGHT);

			EnableDocking(CBRS_ALIGN_ANY);
			DockControlBar(&m_wndToolBar);
			DockControlBar(&m_songBar);
			DockControlBar(&m_machineBar);
			DockControlBar(&m_seqBar);
			LoadBarState(_T("General"));

			// Sets Icon
			HICON tIcon;
			tIcon=theApp.LoadIcon(IDR_MAINFRAME);
			SetIcon(tIcon, true);
			SetIcon(tIcon, false);


			// Finally initializing timer
			// Show Machine view and init 
			PsybarsUpdate();
			m_wndView.RecalcMetrics();
			m_wndView.RecalculateColourGrid();
			m_wndView.OnMachineview();
			m_wndView.InitTimer();
		//	m_wndView.Repaint();
			m_wndView.SetFocus();
		//	m_wndView.EnableSound();
			
			return 0;
		}

		BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
		{
			if( !CFrameWnd::PreCreateWindow(cs) )
				return FALSE;
			
			cs.dwExStyle &= ~WS_EX_CLIENTEDGE;
			cs.lpszClass = AfxRegisterWndClass(0);
			return TRUE;
		}

	#if !defined NDEBUG
		void CMainFrame::AssertValid() const
		{
			CFrameWnd::AssertValid();
		}

		void CMainFrame::Dump(CDumpContext& dc) const
		{
			CFrameWnd::Dump(dc);
		}
	#endif

		BOOL CMainFrame::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
		{
			// let the view have first crack at the command
			if (m_wndView.OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
				return TRUE;
			
			// otherwise, do default handling
			return CFrameWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
		}

		void CMainFrame::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized) 
		{
			CFrameWnd::OnActivate(nState, pWndOther, bMinimized);
			if (((nState == WA_ACTIVE) || (nState == WA_CLICKACTIVE)) && (bMinimized == FALSE))
			{
			}
		}

		void CMainFrame::OnDropFiles(HDROP hDropInfo)
		{
			char szFileName[MAX_PATH];
			char * szExtension;

			int iNumFiles = DragQueryFile((HDROP)  hDropInfo,	// handle of structure for dropped files
				0xFFFFFFFF, // this returns number of dropped files
				NULL,
				NULL);

			for (int i = 0; i < iNumFiles; i++)
			{
				DragQueryFile((HDROP)  hDropInfo,	// handle of structure for dropped files
					i,	// index of file to query
					szFileName,	// buffer for returned filename
					MAX_PATH); 	// size of buffer for filename

				// check for .bmp files only

				if (szExtension = strrchr(szFileName, 46)) // point to everything past last "."
				{
					if (!strcmpi(szExtension, ".psy")) // compare to ".psy"
					{
						SetForegroundWindow();
						m_wndView.OnFileLoadsongNamed(szFileName, 1);
						DragFinish((HDROP)  hDropInfo);	// handle of structure for dropped files
						return;
					}
					// add psb, psv?
					// load waves and crap here
				}
			}
			DragFinish((HDROP)  hDropInfo);	// handle of structure for dropped files
			SetForegroundWindow();
		}

		void CMainFrame::OnSetFocus(CWnd* pOldWnd)
		{
			CFrameWnd::OnSetFocus(pOldWnd);
			// forward focus to the view window
			m_wndView.Repaint();
			m_wndView.SetFocus();
			m_wndView.EnableSound();
		}

		void CMainFrame::OnDestroy() 
		{
			m_wndInfo.DestroyWindow();
			m_midiMonitorDlg.DestroyWindow();
			m_wndInst.DestroyWindow();
			m_wndToolBar.DestroyWindow();
			m_songBar.DestroyWindow();
			m_machineBar.DestroyWindow();
			m_seqBar.DestroyWindow();
			// m_pWndWed->DestroyWindow(); is called by the default CWnd::DestroyWindow() function, and the memory freed by subsequent CWnd::OnPostNCDestroy()
			m_wndView.DestroyWindow();
			HICON _icon = GetIcon(false);
			DestroyIcon(_icon);
			DragAcceptFiles(false);
			CFrameWnd::OnDestroy();
		}


		void CMainFrame::CheckForAutosave()
		{
			CString filepath = Global::psycleconf().GetSongDir().c_str();
			filepath += "\\autosave.psy";

			OldPsyFile file;
			if(file.Open(filepath.GetBuffer(1)))
			{
				file.Close();
				int val = MessageBox("An autosave.psy file has been found in the root song dir. Do you want to reload it? (Press \"No\" to delete it)","Song Recovery",MB_YESNOCANCEL);

				if (val == IDYES ) m_wndView.FileLoadsongNamed(filepath.GetBuffer(1));
				else if (val == IDNO ) DeleteFile(filepath);
			}
		}


		void CMainFrame::StatusBarText(std::string txt)
		{
			m_wndStatusBar.SetWindowText(txt.c_str());
		}

		void CMainFrame::PsybarsUpdate()
		{
			SetAppSongBpm(0);
			SetAppSongTpb(0);

			m_machineBar.m_stepcombo.SetCurSel(m_wndView.patStep);
			
			m_songBar.m_trackcombo.SetCurSel(_pSong->SONGTRACKS-4);
			m_songBar.m_octavecombo.SetCurSel(_pSong->currentOctave);
			
			UpdateComboGen();
			UpdateMasterValue(((Master*)_pSong->_pMachine[MASTER_INDEX])->_outDry);
			
		}

		void CMainFrame::WaveEditorBackUpdate()
		{
			m_pWndWed->Notify();
		}


		//////////////////
		////////////////// Some Menu Commands plus ShowMachineGui
		//////////////////

		void CMainFrame::OnClose() 
		{
			if (m_wndView.CheckUnsavedSong("Exit Psycle"))
			{
				CloseAllMacGuis();
				if(pGearRackDialog) pGearRackDialog->SendMessage(WM_CLOSE);
				m_wndView._outputActive = false;

				//Psycle manages its own list for recent files
				//((CPsycleApp*)AfxGetApp())->SaveRecent(this);

				CString filepath = Global::psycleconf().GetSongDir().c_str();
				filepath += "\\autosave.psy";
				DeleteFile(filepath);
				SaveBarState(_T("General"));

				CFrameWnd::OnClose();
			}
		}

		void CMainFrame::OnViewSongbar() 
		{
			if (m_songBar.IsWindowVisible())
			{
				ShowControlBar(&m_songBar,FALSE,FALSE);
			}
			else {	ShowControlBar(&m_songBar,TRUE,FALSE);	}
		}

		void CMainFrame::OnViewMachinebar() 
		{
			if (m_machineBar.IsWindowVisible())
			{
				ShowControlBar(&m_machineBar,FALSE,FALSE);
			}
			else {	ShowControlBar(&m_machineBar,TRUE,FALSE);	}
		}

		void CMainFrame::OnViewSequencerbar() 
		{
			if (m_seqBar.IsWindowVisible())
			{
				ShowControlBar(&m_seqBar,FALSE,FALSE);
			}
			else {	ShowControlBar(&m_seqBar,TRUE,FALSE);	}	
		}

		void CMainFrame::OnUpdateViewSongbar(CCmdUI* pCmdUI) 
		{
			if ( m_songBar.IsWindowVisible()) pCmdUI->SetCheck(TRUE);
			else pCmdUI->SetCheck(FALSE);
			
		}

		void CMainFrame::OnUpdateViewMachinebar(CCmdUI* pCmdUI) 
		{
			if ( m_machineBar.IsWindowVisible()) pCmdUI->SetCheck(TRUE);
			else pCmdUI->SetCheck(FALSE);
			
		}

		void CMainFrame::OnUpdateViewSequencerbar(CCmdUI* pCmdUI) 
		{
			if ( m_seqBar.IsWindowVisible()) pCmdUI->SetCheck(TRUE);
			else pCmdUI->SetCheck(FALSE);
		}

		void CMainFrame::ShowPerformanceDlg()
		{
			m_wndInfo.UpdateInfo();
			m_wndInfo.ShowWindow(SW_SHOWNORMAL);
			m_wndInfo.SetActiveWindow();
		}

		void CMainFrame::ShowMidiMonitorDlg()
		{
			m_midiMonitorDlg.UpdateInfo();
			m_midiMonitorDlg.ShowWindow(SW_SHOWNORMAL);
			m_midiMonitorDlg.SetActiveWindow();
		}

		void CMainFrame::ShowInstrumentEditor()
		{
			CComboBox *cc2=(CComboBox *)m_machineBar.GetDlgItem(IDC_AUXSELECT);
			cc2->SetCurSel(AUX_INSTRUMENT);
			_pSong->auxcolSelected=_pSong->instSelected;
			UpdateComboIns();

			Global::pInputHandler->AddMacViewUndo();

			m_wndInst.WaveUpdate();
			m_wndInst.ShowWindow(SW_SHOWNORMAL);
			m_wndInst.SetActiveWindow();
		}

		void CMainFrame::OnPsyhelp() 
		{
			CString helppath(Global::pConfig->appPath().c_str());
			helppath +=  "Docs\\psycle.chm";
			::HtmlHelp(::GetDesktopWindow(),helppath, HH_DISPLAY_TOPIC, 0);
		}

		void CMainFrame::UpdateEnvInfo()
		{
			m_wndInfo.UpdateInfo();
		}


		void CMainFrame::ShowMachineGui(int tmac, CPoint point)
		{
			Machine *ma = _pSong->_pMachine[tmac];

			if (ma)
			{
				if (isguiopen[tmac])
				{
					m_pWndMac[tmac]->SetActiveWindow();
				}
				else
				{
					Global::pInputHandler->AddMacViewUndo();

					switch (ma->_type)
					{
					case MACH_MASTER:
						if (!m_wndView.MasterMachineDialog)
						{
							m_wndView.MasterMachineDialog = new CMasterDlg(&m_wndView, *(Master*)ma, &m_wndView.MasterMachineDialog);
							for (int i=0;i<MAX_CONNECTIONS; i++)
							{
								if ( ma->_inputCon[i])
								{
									if (_pSong->_pMachine[ma->_inputMachines[i]])
									{
										strcpy(m_wndView.MasterMachineDialog->macname[i],_pSong->_pMachine[ma->_inputMachines[i]]->_editName);
									}
								}
							}
						}
						CenterWindowOnPoint(m_wndView.MasterMachineDialog, point);
						m_wndView.MasterMachineDialog->ShowWindow(SW_SHOW);
						break;
					case MACH_SAMPLER:
						if (m_wndView.SamplerMachineDialog)
						{
							if (((Machine&)m_wndView.SamplerMachineDialog->machine)._macIndex != ma->_macIndex)
							{
								m_wndView.SamplerMachineDialog->SendMessage(WM_CLOSE);
							}
							else return;
						}
						m_wndView.SamplerMachineDialog = new CGearTracker(&m_wndView.SamplerMachineDialog,*(Sampler*)ma);
						CenterWindowOnPoint(m_wndView.SamplerMachineDialog, point);
						m_wndView.SamplerMachineDialog->ShowWindow(SW_SHOW);
						break;
					case MACH_XMSAMPLER:
						{
						if (m_wndView.XMSamplerMachineDialog)
						{
							if (m_wndView.XMSamplerMachineDialog->GetMachine()->_macIndex != ma->_macIndex)
							{
								m_wndView.XMSamplerMachineDialog->SendMessage(WM_CLOSE);
							}
							else return;
						}
						m_wndView.XMSamplerMachineDialog = new XMSamplerUI(ma->GetEditName(),AfxGetMainWnd());
						m_wndView.XMSamplerMachineDialog->Init((XMSampler*)ma, &m_wndView.XMSamplerMachineDialog);
						m_wndView.XMSamplerMachineDialog->Create(this);
						CenterWindowOnPoint(m_wndView.XMSamplerMachineDialog, point);
						}
						break;
					case MACH_RECORDER:
						{
							if (m_wndView.WaveInMachineDialog)
							{
								if (((Machine&)m_wndView.WaveInMachineDialog->recorder)._macIndex != ma->_macIndex)
								{
									m_wndView.WaveInMachineDialog->SendMessage(WM_CLOSE);
								}
								else return;
							}
							m_wndView.WaveInMachineDialog = new CWaveInMacDlg(&m_wndView, &m_wndView.WaveInMachineDialog,*(AudioRecorder*)ma);
							CenterWindowOnPoint(m_wndView.WaveInMachineDialog, point);
						}
						break;
					case MACH_PLUGIN:
					case MACH_DUPLICATOR:
					case MACH_MIXER:
						{
							CFrameMachine* newwin;
							m_pWndMac[tmac] = newwin = new CFrameMachine(ma);
							newwin->_pActive = &isguiopen[tmac];
							newwin->wndView = &m_wndView;

							newwin->LoadFrame(IDR_FRAMEMACHINE, WS_POPUPWINDOW | WS_CAPTION, this);
							std::ostringstream winname;
							winname<<std::setfill('0') << std::setw(2) << std::hex;
							winname << _pSong->FindBusFromIndex(tmac) << " : " << ma->_editName;
							newwin->SetWindowText(winname.str().c_str());
							newwin->ShowWindow(SW_SHOWNORMAL);
							isguiopen[tmac] = true;
							newwin->PostOpenWnd();
							CenterWindowOnPoint(m_pWndMac[tmac], point);
						}
						break;
					case MACH_VST:
					case MACH_VSTFX:
						{
							CVstEffectWnd* newwin;
							m_pWndMac[tmac] = newwin = new CVstEffectWnd(reinterpret_cast<vst::plugin*>(ma));
							newwin->_pActive = &isguiopen[tmac];
							newwin->LoadFrame(IDR_FRAMEMACHINE, WS_POPUPWINDOW | WS_CAPTION, this);
							std::ostringstream winname;
							winname<<std::setfill('0') << std::setw(2) << std::hex;
							winname << _pSong->FindBusFromIndex(tmac) << " : " << ma->_editName;
							newwin->SetTitleText(winname.str().c_str());
							// C_Tuner.dll crashes if asking size before opening.
//							newwin->ResizeWindow(0);
							newwin->ShowWindow(SW_SHOWNORMAL);
							newwin->PostOpenWnd();
							CenterWindowOnPoint(m_pWndMac[tmac], point);
						break;
						}
					}
				}
			}
		}

		void CMainFrame::CenterWindowOnPoint(CWnd* pWnd, POINT point)
		{
			RECT r,rw;
			WINDOWPLACEMENT w1;
			pWnd->GetWindowRect(&r);
			m_wndView.GetWindowPlacement(&w1);

			if ( point.x == -1 || point.y == -1)
			{
				point.x = r.right/2;
				point.y = r.bottom/2;
			}
			/*
			WINDOWPLACEMENT w2;
			GetWindowPlacement(&w2);
			if (w2.showCmd & SW_SHOWMAXIMIZED)
			{
			*/
				rw.top = w1.rcNormalPosition.top;
				rw.left = w1.rcNormalPosition.left;
				rw.right = w1.rcNormalPosition.right;
				rw.bottom = w1.rcNormalPosition.bottom+64;
				/*
			}
			else
			{
				rw.top = w1.rcNormalPosition.top + w2.rcNormalPosition.top;
				rw.left = w1.rcNormalPosition.left + w2.rcNormalPosition.left;
				rw.bottom = w1.rcNormalPosition.bottom + w2.rcNormalPosition.top;
				rw.right = w1.rcNormalPosition.right + w2.rcNormalPosition.left;
			}
			*/

			int x = rw.left+point.x-((r.right-r.left)/2);
			int y = rw.top+point.y-((r.bottom-r.top)/2);

			if (x+(r.right-r.left) > (rw.right))
			{
				x = rw.right-(r.right-r.left);
			}
			// no else incase window is bigger than screen
			if (x < rw.left)
			{
				x = rw.left;
			}

			if (y+(r.bottom-r.top) > (rw.bottom))
			{
				y = rw.bottom-(r.bottom-r.top);
			}
			// no else incase window is bigger than screen
			if (y < rw.top)
			{
				y = rw.top;
			}

			pWnd->SetWindowPos(NULL,x,y,0,0,SWP_NOSIZE | SWP_NOZORDER | SWP_SHOWWINDOW);
		}

		void CMainFrame::CloseAllMacGuis()
		{
			CExclusiveLock lock(&Global::_pSong->semaphore, 2, true);
			for (int i = 0; i < MAX_WIRE_DIALOGS; i++)
			{
				if (m_wndView.WireDialog[i])
				{
					m_wndView.WireDialog[i]->SendMessage(WM_CLOSE);
				}
			}
			for (int c=0; c<MAX_MACHINES; c++)
			{
				if ( _pSong->_pMachine[c] ) CloseMacGui(c,false);
			}
		}

		void CMainFrame::CloseMacGui(int mac,bool closewiredialogs)
		{
			if (closewiredialogs ) 
			{
				for (int i = 0; i < MAX_WIRE_DIALOGS; i++)
				{
					if (m_wndView.WireDialog[i])
					{
						if ((m_wndView.WireDialog[i]->srcMachine._macIndex == _pSong->_pMachine[mac]->_macIndex) ||
							(m_wndView.WireDialog[i]->dstMachine._macIndex == _pSong->_pMachine[mac]->_macIndex))
						{
							m_wndView.WireDialog[i]->SendMessage(WM_CLOSE);
						}
					}
				}
			}
			if (_pSong->_pMachine[mac])
			{
				switch (_pSong->_pMachine[mac]->_type)
				{
					case MACH_MASTER:
						if (m_wndView.MasterMachineDialog) m_wndView.MasterMachineDialog->SendMessage(WM_CLOSE);
						break;
					case MACH_SAMPLER:
						if (m_wndView.SamplerMachineDialog) m_wndView.SamplerMachineDialog->SendMessage(WM_CLOSE);
						break;
					case MACH_XMSAMPLER:
						if (m_wndView.XMSamplerMachineDialog) m_wndView.XMSamplerMachineDialog->SendMessage(WM_CLOSE);
						break;
					case MACH_RECORDER:
						if (m_wndView.WaveInMachineDialog) m_wndView.WaveInMachineDialog->SendMessage(WM_CLOSE);
						break;
					case MACH_DUPLICATOR:
//					case MACH_LFO:
//					case MACH_AUTOMATOR:
					case MACH_MIXER:
					case MACH_PLUGIN:
					case MACH_VST:
					case MACH_VSTFX:
						if (isguiopen[mac])
						{
							m_pWndMac[mac]->SendMessage(WM_CLOSE);
						}
						break;
					default:break;
				}
			}
		}



		//
		//Dialogbars are a bit special...
		///////////////////////////////////////////

		//
		//SongBar
		//
		void CMainFrame::OnSelchangeTrackcombo() { m_songBar.OnSelchangeTrackcombo(); }
		void CMainFrame::OnCloseupTrackcombo() { m_songBar.OnCloseupTrackcombo(); }
		void CMainFrame::OnBpmAddOne() { m_songBar.OnBpmAddOne(); }
		void CMainFrame::OnBpmAddTen() { m_songBar.OnBpmAddTen(); }
		void CMainFrame::OnBpmDecOne() { m_songBar.OnBpmDecOne(); }
		void CMainFrame::OnBpmDecTen() { m_songBar.OnBpmDecTen(); }
		void CMainFrame::OnDecTPB() { m_songBar.OnDecTPB(); }
		void CMainFrame::OnIncTPB() { m_songBar.OnIncTPB(); }
		void CMainFrame::OnCloseupCombooctave() { m_songBar.OnCloseupCombooctave(); }
		void CMainFrame::OnSelchangeCombooctave() { m_songBar.OnSelchangeCombooctave(); }
		void CMainFrame::OnClipbut() { m_songBar.OnClipbut(); }
		BOOL CMainFrame::OnToolTipNotify( UINT unId, NMHDR *pstNMHDR, LRESULT *pstResult ) {
			return m_songBar.OnToolTipNotify(unId, pstNMHDR, pstResult);
		}
		void CMainFrame::SetAppSongBpm(int x) { m_songBar.SetAppSongBpm(x); }
		void CMainFrame::SetAppSongTpb(int x) { m_songBar.SetAppSongTpb(x); }
		void CMainFrame::ShiftOctave(int x) { m_songBar.ShiftOctave(x); }
		void CMainFrame::UpdateMasterValue(int newvalue) { m_songBar.UpdateMasterValue(newvalue); }
		void CMainFrame:: UpdateVumeters(float l, float r, COLORREF vu1,COLORREF vu2,COLORREF vu3,bool clip) 
		{
			m_songBar.UpdateVumeters(l,r,vu1,vu2,vu3,clip);
		}
		//
		//MachineBar
		//
		void CMainFrame::OnSelchangeCombostep() { m_machineBar.OnSelchangeCombostep(); }
		void CMainFrame::OnCloseupCombostep() { m_machineBar.OnCloseupCombostep(); }
		void CMainFrame::OnSelchangeBarCombogen() { m_machineBar.OnSelchangeBarCombogen(); }
		void CMainFrame::OnCloseupBarCombogen() { m_machineBar.OnCloseupBarCombogen(); }
		void CMainFrame::OnCloseupAuxselect() { m_machineBar.OnCloseupAuxselect(); }
		void CMainFrame::OnSelchangeAuxselect() { m_machineBar.OnSelchangeAuxselect(); }
		void CMainFrame::OnSelchangeBarComboins() { m_machineBar.OnSelchangeBarComboins(); }
		void CMainFrame::OnCloseupBarComboins() { m_machineBar.OnCloseupBarComboins(); }
		void CMainFrame::OnBDecgen() { m_machineBar.OnBDecgen(); }
		void CMainFrame::OnBIncgen() { m_machineBar.OnBIncgen(); }
		void CMainFrame::OnGearRack() { m_machineBar.OnGearRack(); }
		void CMainFrame::OnBDecwav() { m_machineBar.OnBDecwav(); }
		void CMainFrame::OnBIncwav() { m_machineBar.OnBIncwav(); }
		void CMainFrame::OnLoadwave() { m_machineBar.OnLoadwave(); }
		void CMainFrame::OnSavewave() { m_machineBar.OnSavewave(); }
		void CMainFrame::OnEditwave() { m_machineBar.OnEditwave(); }
		void CMainFrame::OnWavebut() { m_machineBar.OnWavebut(); }
		void CMainFrame::ChangeIns(int i) { m_machineBar.ChangeIns(i); }
		void CMainFrame::ChangeGen(int i) { m_machineBar.ChangeGen(i); }
		void CMainFrame::UpdateComboIns(bool updatelist) { m_machineBar.UpdateComboIns(updatelist); }
		void CMainFrame::UpdateComboGen(bool updatelist) { m_machineBar.UpdateComboGen(updatelist); }
		void CMainFrame::EditQuantizeChange(int diff) { m_machineBar.EditQuantizeChange(diff); }
		void CMainFrame::RedrawGearRackList()
		{
			if (pGearRackDialog)
			{
				pGearRackDialog->RedrawList();
			}
		}
		//
		//SequenceBar
		//
		void CMainFrame::OnSelchangeSeqlist() { m_seqBar.OnSelchangeSeqlist(); }
		void CMainFrame::OnDblclkSeqlist() { m_seqBar.OnDblclkSeqlist(); }
		void CMainFrame::OnIncshort() { m_seqBar.OnIncshort(); }
		void CMainFrame::OnDecshort() { m_seqBar.OnDecshort(); }
		void CMainFrame::OnSeqnew() { m_seqBar.OnSeqnew(); }
		void CMainFrame::OnSeqduplicate() { m_seqBar.OnSeqduplicate(); }
		void CMainFrame::OnSeqins() { m_seqBar.OnSeqins(); }
		void CMainFrame::OnSeqdelete() { m_seqBar.OnSeqdelete(); }
		void CMainFrame::OnSeqrename() { m_seqBar.OnSeqrename(); }
		void CMainFrame::OnSeqchange() { m_seqBar.OnSeqchange(); }
		void CMainFrame::OnSeqcut() { m_seqBar.OnSeqcut(); }
		void CMainFrame::OnSeqcopy() { m_seqBar.OnSeqcopy(); }
		void CMainFrame::OnSeqpaste() { m_seqBar.OnSeqpasteAbove(); }
		void CMainFrame::OnSeqpasteBelow() { m_seqBar.OnSeqpasteBelow(); }
		void CMainFrame::OnSeqsort() { m_seqBar.OnSeqsort(); }
		void CMainFrame::OnSeqclear() { m_seqBar.OnSeqclear(); }
		void CMainFrame::OnUpdatepaste(CCmdUI* pCmdUI) {m_seqBar.OnUpdatepaste(pCmdUI);}
		void CMainFrame::OnUpdatepasteBelow(CCmdUI* pCmdUI) {m_seqBar.OnUpdatepasteBelow(pCmdUI);}
		void CMainFrame::OnFollow() { m_seqBar.OnFollow(); }
		void CMainFrame::OnRecordNoteoff() { m_seqBar.OnRecordNoteoff(); }
		void CMainFrame::OnRecordTweaks() { m_seqBar.OnRecordTweaks(); }
		void CMainFrame::OnShowpattername() { m_seqBar.OnShowpattername(); }
		void CMainFrame::OnMultichannelAudition() { m_seqBar.OnMultichannelAudition(); }
		void CMainFrame::OnNotestoeffects() { m_seqBar.OnNotestoeffects(); }
		void CMainFrame::OnMovecursorpaste() { m_seqBar.OnMovecursorpaste(); }
		bool CMainFrame::ToggleFollowSong()
		{
			bool check;
			if(m_seqBar.m_follow.GetCheck() == 0) {
				m_seqBar.m_follow.SetCheck(1);
				check=true;
			} else {
				m_seqBar.m_follow.SetCheck(0);
				check=false;
			}
			m_seqBar.OnFollow();
			return check;
		}
		void CMainFrame::UpdatePlayOrder(bool mode) { m_seqBar.UpdatePlayOrder(mode); }
		void CMainFrame::UpdateSequencer(int bottom) { m_seqBar.UpdateSequencer(bottom); }

		//
		//StatusBar
		//
		void CMainFrame::OnUpdateIndicatorSeqPos(CCmdUI *pCmdUI) 
		{
			pCmdUI->Enable();
			CString str;
			str.Format("Pos %.2X", (Global::pPlayer->_playing) ? Global::pPlayer->_playPosition : m_wndView.editPosition); 
			pCmdUI->SetText(str); 
		}

		void CMainFrame::OnUpdateIndicatorPattern(CCmdUI *pCmdUI) 
		{
			pCmdUI->Enable(); 
			CString str;
			str.Format("Pat %.2X", (Global::pPlayer->_playing) ? Global::pPlayer->_playPattern : _pSong->playOrder[m_wndView.editPosition]); 
			pCmdUI->SetText(str); 
		}

		void CMainFrame::OnUpdateIndicatorLine(CCmdUI *pCmdUI) 
		{
			pCmdUI->Enable(); 
			CString str;
			str.Format("Line %u", (Global::pPlayer->_playing) ? Global::pPlayer->_lineCounter : m_wndView.editcur.line); 
			pCmdUI->SetText(str); 
		}

		void CMainFrame::OnUpdateIndicatorTime(CCmdUI *pCmdUI) 
		{
			pCmdUI->Enable(); 
			if (Global::pPlayer->_playing)
			{
				CString str;
				str.Format( "%.02u:%.02u:%.02f", Global::pPlayer->_playTimem / 60, Global::pPlayer->_playTimem % 60, Global::pPlayer->_playTime);
				pCmdUI->SetText(str); 
			}
		}

		void CMainFrame::OnUpdateIndicatorEdit(CCmdUI *pCmdUI) 
		{
			pCmdUI->Enable(m_wndView.bEditMode); 
		}

		void CMainFrame::OnUpdateIndicatorFollow(CCmdUI *pCmdUI) 
		{
			pCmdUI->Enable(Global::psycleconf()._followSong);
		}

		void CMainFrame::OnUpdateIndicatorNoteoff(CCmdUI *pCmdUI) 
		{
			pCmdUI->Enable(Global::psycleconf().inputHandler()._RecordNoteoff); 
		}

		void CMainFrame::OnUpdateIndicatorTweaks(CCmdUI *pCmdUI) 
		{
			pCmdUI->Enable(Global::psycleconf().inputHandler()._RecordTweaks);
		}

		void CMainFrame::OnUpdateIndicatorOctave(CCmdUI *pCmdUI) 
		{
			pCmdUI->Enable(); 
			CString str;
			str.Format("Oct %u", _pSong->currentOctave); 
			pCmdUI->SetText(str); 

		}
		LRESULT CMainFrame::OnSetMessageString(WPARAM wParam, LPARAM lParam)
		{
			if (wParam == AFX_IDS_IDLEMESSAGE)
			{
				if (StatusBarIdleText())
				{
					return CFrameWnd::OnSetMessageString (0,(LPARAM)szStatusIdle.c_str());
				}
			}
			return CFrameWnd::OnSetMessageString (wParam, lParam);

		}

		void CMainFrame::StatusBarIdle()
		{
			if (StatusBarIdleText())
			{
				m_wndStatusBar.SetWindowText(szStatusIdle.c_str());
			}
		}

		BOOL CMainFrame::StatusBarIdleText()
		{
			if (_pSong)
			{
				std::ostringstream oss;
				oss << _pSong->name
					<< " - " << _pSong->patternName[_pSong->playOrder[m_wndView.editPosition]];

				if ((m_wndView.viewMode==view_modes::pattern)	&& (!Global::pPlayer->_playing))
				{
					unsigned char *toffset=_pSong->_ptrackline(m_wndView.editPosition,m_wndView.editcur.track,m_wndView.editcur.line);
					int machine = toffset[2];
					if (machine<MAX_MACHINES)
					{
						if (_pSong->_pMachine[machine])
						{
							oss << " - " << _pSong->_pMachine[machine]->_editName;
							if (_pSong->_pMachine[machine]->_type == MACH_SAMPLER)
							{
								if (_pSong->_pInstrument[toffset[1]]->_sName[0])
									oss <<  " - " << _pSong->_pInstrument[toffset[1]]->_sName;
							}
							else if (_pSong->_pMachine[machine]->_type == MACH_XMSAMPLER)
							{
								if (XMSampler::rInstrument(toffset[1]).IsEnabled())
									oss <<  " - " << XMSampler::rInstrument(toffset[1]).Name();
							}
							else
							{
								char buf[64];
								buf[0]=0;
								_pSong->_pMachine[machine]->GetParamName(toffset[1],buf);
								if(buf[0])
									oss <<  " - " << buf;
							}
						}
						else
						{
							oss << " - Machine Out of Range";
						}
					}
				}

				szStatusIdle=oss.str();
				return TRUE;
			}
			return FALSE;
		}

}}
