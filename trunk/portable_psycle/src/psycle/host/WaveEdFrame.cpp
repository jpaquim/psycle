///\file
///\brief implementation file for psycle::host::CWaveEdFrame.
#include <packageneric/pre-compiled.private.hpp>
#include "Psycle.hpp"
#include "Song.hpp"
#include "WaveEdFrame.hpp"
#include "MainFrm.hpp"
UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(psycle)
	UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(host)
		IMPLEMENT_DYNAMIC(CWaveEdFrame, CFrameWnd)

		BEGIN_MESSAGE_MAP(CWaveEdFrame, CFrameWnd)
			//{{AFX_MSG_MAP(CWaveEdFrame)
			ON_WM_CLOSE()
			ON_UPDATE_COMMAND_UI ( ID_INDICATOR_SIZE, OnUpdateStatusBar )
			ON_WM_CREATE()
			ON_WM_SHOWWINDOW()
			ON_UPDATE_COMMAND_UI ( ID_INDICATOR_MODE, OnUpdateStatusBar )
			//}}AFX_MSG_MAP
		END_MESSAGE_MAP()

		static UINT indicators[] =
		{
			ID_SEPARATOR,           // status line indicator
			ID_INDICATOR_SIZE,
			ID_INDICATOR_MODE
		};

		CWaveEdFrame::CWaveEdFrame()
		{
		}
		CWaveEdFrame::CWaveEdFrame(Song* _sng,CMainFrame* pframe)
		{
			this->_pSong=_sng;
			wavview.SetSong(this->_pSong);
			wavview.SetParent(pframe);
		}

		CWaveEdFrame::~CWaveEdFrame() throw()
		{

		}

		void CWaveEdFrame::OnClose() 
		{
			ShowWindow(SW_HIDE);
		}

		int CWaveEdFrame::OnCreate(LPCREATESTRUCT lpCreateStruct) 
		{
			if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
			{
				return -1;
			}

		/*	toolbar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP |
				CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);
			toolbar.LoadToolBar(IDR_WAVEBAR); */
			
			statusbar.Create(this);
			statusbar.SetIndicators(indicators, sizeof(indicators)/sizeof(UINT));
			statusbar.SetPaneStyle(0, SBPS_NORMAL);
			statusbar.SetPaneInfo(1, ID_INDICATOR_SIZE, SBPS_NORMAL, 100);
			statusbar.SetPaneInfo(2, ID_INDICATOR_MODE, SBPS_NORMAL, 70);
			
			wavview.Create(NULL, "Psycle wave editor", AFX_WS_DEFAULT_VIEW,
			CRect(0, 0, 0, 0), this, AFX_IDW_PANE_FIRST, NULL);

			/*	toolbar.EnableDocking(CBRS_ALIGN_ANY);
			EnableDocking(CBRS_ALIGN_ANY); */
			
			SetWindowText("Psycle wave editor");
			return 0;
		}

		BOOL CWaveEdFrame::PreCreateWindow(CREATESTRUCT& cs) 
		{
			if( !CFrameWnd::PreCreateWindow(cs) )
			{
				return FALSE;
			}

		//	cs.dwExStyle &= ~WS_EX_CLIENTEDGE;
		//	cs.lpszClass = AfxRegisterWndClass(0,0,0, AfxGetApp()->LoadIcon(IDR_WAVEFRAME));
			
			return TRUE;	
		}

		void CWaveEdFrame::GenerateView() 
		{	
			this->wavview.GenerateAndShow(); 
		}

		BOOL CWaveEdFrame::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo) 
		{
			if (wavview.OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
			{
				this->AdjustStatusBar(_pSong->instSelected);
				return TRUE;	
			}
			return CFrameWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
		}

		void CWaveEdFrame::OnUpdateStatusBar(CCmdUI *pCmdUI)  
		{     
			pCmdUI->Enable ();  
		}

		void CWaveEdFrame::AdjustStatusBar(int ins)
		{
			char buff[32];
			int	wl=_pSong->_pInstrument[ins]->waveLength;
			sprintf(buff, "Size: %u", wl);
			statusbar.SetPaneText(1, buff, TRUE);

			if (wl)
			{
				if (_pSong->_pInstrument[ins]->waveStereo) statusbar.SetPaneText(2, "Mode: Stereo", TRUE);
				else statusbar.SetPaneText(2, "Mode: Mono", TRUE);
			}
			else statusbar.SetPaneText(2, "Mode: Empty", TRUE);
		}

		void CWaveEdFrame::OnShowWindow(BOOL bShow, UINT nStatus) 
		{
			CFrameWnd::OnShowWindow(bShow, nStatus);

			Notify();
		//	AdjustStatusBar(_pSong->instSelected, _pSong->waveSelected);
			UpdateWindow();
		}

		void CWaveEdFrame::Notify(void)
		{
			wavview.SetViewData(_pSong->instSelected);
			AdjustStatusBar(_pSong->instSelected);
		}
	UNIVERSALIS__COMPILER__NAMESPACE__END
UNIVERSALIS__COMPILER__NAMESPACE__END
