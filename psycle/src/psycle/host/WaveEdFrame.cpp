///\file
///\brief implementation file for psycle::host::CWaveEdFrame.

///\todo The use of song class could be removed. It only needs to have the current selected sample
// and in psycore, the InstPreview has been moved to the Sampler class.
#include "WaveEdFrame.hpp"

#include "ProjectData.hpp"
#include "Configuration.hpp"
#include "MainFrm.hpp"

#include <psycle/core/song.h>
#include <psycle/core/sampler.h>

#if !defined NDEBUG
   #define new DEBUG_NEW
   #undef THIS_FILE
   static char THIS_FILE[] = __FILE__;
#endif

namespace psycle { namespace host {

		IMPLEMENT_DYNAMIC(CWaveEdFrame, CFrameWnd)

		BEGIN_MESSAGE_MAP(CWaveEdFrame, CFrameWnd)
			//{{AFX_MSG_MAP(CWaveEdFrame)
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
			//}}AFX_MSG_MAP
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
			: wavview(this, 0) {
		}

		CWaveEdFrame::CWaveEdFrame(ProjectData* projects, CMainFrame* pframe)
			: projects_(projects),
			  wavview(this, pframe) {
		}

		Song* CWaveEdFrame::song() {
			return &projects_->active_project()->song();
		}

		void CWaveEdFrame::OnClose() 
		{
			ShowWindow(SW_HIDE);
			OnStop();
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
			statusbar.SetPaneStyle(0, /*SBPS_NORMAL*/ SBPS_STRETCH);
			statusbar.SetPaneInfo(1, ID_INDICATOR_SEL, SBPS_NORMAL, 180);
			statusbar.SetPaneInfo(2, ID_INDICATOR_SIZE, SBPS_NORMAL, 180);
			statusbar.SetPaneInfo(3, ID_INDICATOR_MODE, SBPS_NORMAL, 70);

			if( !(ToolBar.Create(this, WS_CHILD|WS_VISIBLE|CBRS_TOP|CBRS_FLYBY)) || !ToolBar.LoadToolBar(IDR_WAVEDFRAME))
				this->MessageBox("Error creating toolbar!", "whoops!", MB_OK);

			
			wavview.Create(NULL, "Psycle wave editor", AFX_WS_DEFAULT_VIEW,
			CRect(0, 0, 0, 0), this, AFX_IDW_PANE_FIRST, NULL);

			/*	toolbar.EnableDocking(CBRS_ALIGN_ANY);
			EnableDocking(CBRS_ALIGN_ANY); */
			bPlaying=false;
			SetWindowText("Psycle wave editor");
			return 0;
		}

		BOOL CWaveEdFrame::PreCreateWindow(CREATESTRUCT& cs) 
		{
			if( !CFrameWnd::PreCreateWindow(cs) )
			{
				return false;
			}

		//	cs.dwExStyle &= ~WS_EX_CLIENTEDGE;
		//	cs.lpszClass = AfxRegisterWndClass(0,0,0, AfxGetApp()->LoadIcon(IDR_WAVEFRAME));
			
			return true;	
		}

		void CWaveEdFrame::OnDestroy()
		{
			CFrameWnd::OnDestroy();

			OnStop();
		}


		void CWaveEdFrame::GenerateView() 
		{	
			this->wavview.GenerateAndShow(); 
		}

		BOOL CWaveEdFrame::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo) 
		{
			if (wavview.OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
			{
				this->AdjustStatusBar(projects_->active_project()->song().instSelected());
				return true;	
			}
			return CFrameWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
		}

		void CWaveEdFrame::OnUpdateStatusBar(CCmdUI *pCmdUI)  
		{     
			pCmdUI->Enable ();  
		}

		void CWaveEdFrame::OnUpdatePlayButtons(CCmdUI *pCmdUI)
		{
			pCmdUI->Enable(!Sampler::waved.IsEnabled());
		}

		void CWaveEdFrame::OnUpdateStopButton(CCmdUI *pCmdUI)
		{
			pCmdUI->Enable(Sampler::waved.IsEnabled());
		}

		void CWaveEdFrame::OnUpdateReleaseButton(CCmdUI *pCmdUI)
		{
			pCmdUI->Enable(Sampler::waved.IsEnabled() && Sampler::waved.IsLooping());
		}

		void CWaveEdFrame::OnUpdateSelection(CCmdUI *pCmdUI)
		{
			pCmdUI->Enable();

			char buff[48];
			int sl=wavview.GetSelectionLength();
			if(sl==0 || projects_->active_project()->song()._pInstrument[projects_->active_project()->song().instSelected()]==NULL)
				sprintf(buff, "No Data in Selection.");
			else
			{
				float slInSecs = sl / float(Global::configuration().GetSamplesPerSec());
				sprintf(buff, "Selection: %u (%0.3f secs.)", sl, slInSecs);
			}
			statusbar.SetPaneText(1, buff, true);
		}

		void CWaveEdFrame::AdjustStatusBar(int ins)
		{
			char buff[48];
			int	wl=projects_->active_project()->song()._pInstrument[ins]->waveLength;
			float wlInSecs = wl / float(Global::configuration().GetSamplesPerSec());
			sprintf(buff, "Size: %u (%0.3f secs.)", wl, wlInSecs);
			statusbar.SetPaneText(2, buff, true);

			if (wl)
			{
				if (projects_->active_project()->song()._pInstrument[ins]->waveStereo) statusbar.SetPaneText(3, "Mode: Stereo", true);
				else statusbar.SetPaneText(3, "Mode: Mono", true);
			}
			else statusbar.SetPaneText(3, "Mode: Empty", true);
		}

		void CWaveEdFrame::OnShowWindow(BOOL bShow, UINT nStatus) 
		{
			CFrameWnd::OnShowWindow(bShow, nStatus);

			Notify();
		//	AdjustStatusBar(projects_->active_project()->song().instSelected(), projects_->active_project()->song().waveSelected);
			UpdateWindow();
		}

		void CWaveEdFrame::Notify(void)
		{
			wavview.SetViewData(projects_->active_project()->song().instSelected());
			AdjustStatusBar(projects_->active_project()->song().instSelected());
			wsInstrument = projects_->active_project()->song().instSelected();
		}

		void CWaveEdFrame::OnPlay() {PlayFrom(wavview.GetCursorPos());}
		void CWaveEdFrame::OnPlayFromStart() {PlayFrom(0);}
		void CWaveEdFrame::OnRelease()
		{
			Sampler::waved.Release();
		}
		void CWaveEdFrame::OnStop()
		{
			Sampler::waved.Stop();
		}

		void CWaveEdFrame::PlayFrom(unsigned long startPos)
		{
			if( startPos<0 || startPos >= projects_->active_project()->song()._pInstrument[wsInstrument]->waveLength )
				return;

			OnStop();

			Sampler::waved.SetInstrument( projects_->active_project()->song()._pInstrument[wsInstrument] );
			Sampler::waved.Play(startPos);
		}
		void CWaveEdFrame::OnUpdateFFandRWButtons(CCmdUI* pCmdUI)
		{
			pCmdUI->Enable(true);
		}

		void CWaveEdFrame::OnFastForward()
		{
			unsigned long wl = projects_->active_project()->song()._pInstrument[wsInstrument]->waveLength;
			wavview.SetCursorPos( wl-1 );
		}
		void CWaveEdFrame::OnRewind()
		{
			wavview.SetCursorPos( 0 );
		}

	}   // namespace
}   // namespace
