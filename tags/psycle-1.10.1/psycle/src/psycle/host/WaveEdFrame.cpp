///\file
///\brief implementation file for psycle::host::CWaveEdFrame.
#include <psycle/host/detail/project.private.hpp>
#include "WaveEdFrame.hpp"

#include "Configuration.hpp"
#include "AudioDriver.hpp"
#include "MainFrm.hpp"

#include "Song.hpp"

namespace psycle { namespace host {
		extern CPsycleApp theApp;
		IMPLEMENT_DYNAMIC(CWaveEdFrame, CFrameWnd)

		BEGIN_MESSAGE_MAP(CWaveEdFrame, CFrameWnd)
			ON_WM_CLOSE()
			ON_UPDATE_COMMAND_UI ( ID_INDICATOR_SIZE, OnUpdateStatusBar )
			ON_WM_CREATE()
			ON_WM_SHOWWINDOW()
			ON_UPDATE_COMMAND_UI ( ID_INDICATOR_MODE, OnUpdateStatusBar )
			ON_UPDATE_COMMAND_UI ( ID_INDICATOR_SEL,  OnUpdateSelection )
			ON_UPDATE_COMMAND_UI ( ID_WAVED_PLAYFROMSTART, OnUpdatePlayButtons )
			ON_UPDATE_COMMAND_UI ( ID_WAVED_PLAY,	OnUpdatePlayButtons		)
			ON_UPDATE_COMMAND_UI ( ID_WAVED_STOP,	OnUpdateStopButton		)
			ON_UPDATE_COMMAND_UI ( ID_WAVED_RELEASE,OnUpdateReleaseButton		)
			ON_UPDATE_COMMAND_UI ( ID_WAVED_FASTFORWARD, OnUpdateFFandRWButtons )
			ON_UPDATE_COMMAND_UI ( ID_WAVED_REWIND, OnUpdateFFandRWButtons )
			ON_COMMAND ( ID_WAVED_PLAY, OnPlay )
			ON_COMMAND ( ID_WAVED_PLAYFROMSTART, OnPlayFromStart )
			ON_COMMAND ( ID_WAVED_STOP, OnStop )
			ON_COMMAND ( ID_WAVED_RELEASE, OnRelease)
			ON_COMMAND ( ID_WAVED_FASTFORWARD, OnFastForward )
			ON_COMMAND ( ID_WAVED_REWIND, OnRewind )
			ON_WM_DESTROY()
		END_MESSAGE_MAP()

		static UINT indicators[] =
		{
			ID_SEPARATOR,           // status line indicator
			ID_INDICATOR_SEL,
			ID_INDICATOR_SIZE,
			ID_INDICATOR_MODE
		};

		CWaveEdFrame::CWaveEdFrame()
		{
		}
		CWaveEdFrame::CWaveEdFrame(Song& _sng, CMainFrame* pframe)
		{
			this->_pSong = &_sng;
			wavview.SetSong(_sng);
			wavview.SetMainFrame(pframe);
		}

		CWaveEdFrame::~CWaveEdFrame() throw()
		{
		}

		BOOL CWaveEdFrame::PreCreateWindow(CREATESTRUCT& cs) 
		{
			if( !CFrameWnd::PreCreateWindow(cs) )
			{
				return false;
			}

			cs.dwExStyle &= ~WS_EX_CLIENTEDGE;
			cs.lpszClass = AfxRegisterWndClass(0);

			return true;	
		}

		BOOL CWaveEdFrame::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo) 
		{
			if (wavview.OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
			{
				this->AdjustStatusBar(_pSong->instSelected);
				return true;	
			}
			return CFrameWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
		}

		int CWaveEdFrame::OnCreate(LPCREATESTRUCT lpCreateStruct) 
		{
			if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
			{
				return -1;
			}
			if (!wavview.Create(NULL, NULL, AFX_WS_DEFAULT_VIEW,
				CRect(0, 0, 0, 0), this, AFX_IDW_PANE_FIRST, NULL))
			{
				TRACE0("Failed to create view window\n");
				return -1;
			}

			if( !ToolBar.CreateEx(this, TBSTYLE_FLAT| TBSTYLE_TRANSPARENT) ||
				!ToolBar.LoadToolBar(IDR_WAVEDFRAME))
			{
				TRACE0("Failed to create toolbar\n");
				return -1;      // fail to create
			}

			// Status bar
			if (!statusbar.Create(this) ||
				!statusbar.SetIndicators(indicators, sizeof(indicators)/sizeof(UINT)))
			{
				TRACE0("Failed to create status bar\n");
				return -1;      // fail to create
			}
			statusbar.SetPaneStyle(0, /*SBPS_NORMAL*/ SBPS_STRETCH);
			statusbar.SetPaneInfo(1, ID_INDICATOR_SEL, SBPS_NORMAL, 180);
			statusbar.SetPaneInfo(2, ID_INDICATOR_SIZE, SBPS_NORMAL, 180);
			statusbar.SetPaneInfo(3, ID_INDICATOR_MODE, SBPS_NORMAL, 70);

			
			ToolBar.SetWindowText("Psycle Wave Editor tool bar");
			ToolBar.EnableDocking(CBRS_ALIGN_ANY);
			EnableDocking(CBRS_ALIGN_ANY);
			DockControlBar(&ToolBar);
			LoadBarState(_T("WaveEdToolbar"));
			// Sets Icon
			HICON tIcon;
			tIcon=theApp.LoadIcon(IDR_WAVEFRAME);
			SetIcon(tIcon, true);
			SetIcon(tIcon, false);

			bPlaying=false;
			SetWindowText("Psycle wave editor");
			return 0;
		}

		void CWaveEdFrame::OnClose() 
		{
			AfxGetMainWnd()->SetFocus();
			ShowWindow(SW_HIDE);
			OnStop();
		}
		void CWaveEdFrame::OnDestroy()
		{
			SaveBarState(_T("WaveEdToolbar"));
			OnStop();
			HICON _icon = GetIcon(false);
			DestroyIcon(_icon);
			CFrameWnd::OnDestroy();
		}

		void CWaveEdFrame::GenerateView() 
		{	
			this->wavview.GenerateAndShow(); 
		}

		void CWaveEdFrame::OnUpdateStatusBar(CCmdUI *pCmdUI)  
		{     
			pCmdUI->Enable();  
		}

		void CWaveEdFrame::OnUpdatePlayButtons(CCmdUI *pCmdUI)
		{
			pCmdUI->Enable(!_pSong->waved.IsEnabled());
		}

		void CWaveEdFrame::OnUpdateStopButton(CCmdUI *pCmdUI)
		{
			pCmdUI->Enable(_pSong->waved.IsEnabled());
		}

		void CWaveEdFrame::OnUpdateReleaseButton(CCmdUI *pCmdUI)
		{
			pCmdUI->Enable(_pSong->waved.IsEnabled() && _pSong->waved.IsLooping());
		}

		void CWaveEdFrame::OnUpdateSelection(CCmdUI *pCmdUI)
		{
			pCmdUI->Enable();

			char buff[48];
			int sl=wavview.GetSelectionLength();
			if(sl==0 || _pSong->_pInstrument[_pSong->instSelected]==NULL)
				sprintf(buff, "No Data in Selection.");
			else
			{
				float slInSecs = sl / float(Global::configuration()._pOutputDriver->GetSamplesPerSec());
				sprintf(buff, "Selection: %u (%0.3f secs.)", sl, slInSecs);
			}
			statusbar.SetPaneText(1, buff, true);
		}

		void CWaveEdFrame::AdjustStatusBar(int ins)
		{
			char buff[48];
			int	wl=_pSong->_pInstrument[ins]->waveLength;
			float wlInSecs = wl / float(Global::configuration()._pOutputDriver->GetSamplesPerSec());
			sprintf(buff, "Size: %u (%0.3f secs.)", wl, wlInSecs);
			statusbar.SetPaneText(2, buff, true);

			if (wl)
			{
				if (_pSong->_pInstrument[ins]->waveStereo) statusbar.SetPaneText(3, "Mode: Stereo", true);
				else statusbar.SetPaneText(3, "Mode: Mono", true);
			}
			else statusbar.SetPaneText(3, "Mode: Empty", true);
		}

		void CWaveEdFrame::OnShowWindow(BOOL bShow, UINT nStatus) 
		{
			CFrameWnd::OnShowWindow(bShow, nStatus);
			if(bShow) {
				Notify();
				UpdateWindow();
				wavview.StartTimer();
			}
			else 
			{
				wavview.StopTimer();
			}
		}

		void CWaveEdFrame::Notify(void)
		{
			wavview.SetViewData(_pSong->instSelected);
			AdjustStatusBar(_pSong->instSelected);
			wsInstrument = _pSong->instSelected;
		}

		void CWaveEdFrame::OnPlay() {PlayFrom(wavview.GetCursorPos());}
		void CWaveEdFrame::OnPlayFromStart() {PlayFrom(0);}
		void CWaveEdFrame::OnRelease()
		{
			_pSong->waved.Release();
		}
		void CWaveEdFrame::OnStop()
		{
			_pSong->waved.Stop();
		}

		void CWaveEdFrame::PlayFrom(unsigned long startPos)
		{
			if( startPos<0 || startPos >= _pSong->_pInstrument[wsInstrument]->waveLength )
				return;

			OnStop();

			_pSong->waved.SetInstrument( _pSong->_pInstrument[wsInstrument] );
			_pSong->waved.Play(startPos);
			wavview.SetTimer(ID_TIMER_WAVED_PLAYING, 33, 0);
		}
		void CWaveEdFrame::OnUpdateFFandRWButtons(CCmdUI* pCmdUI)
		{
			pCmdUI->Enable(true);
		}

		void CWaveEdFrame::OnFastForward()
		{
			unsigned long wl = _pSong->_pInstrument[wsInstrument]->waveLength;
			wavview.SetCursorPos( wl-1 );
		}
		void CWaveEdFrame::OnRewind()
		{
			wavview.SetCursorPos( 0 );
		}

	}   // namespace
}   // namespace
