///\file
///\brief interface file for psycle::host::CWaveEdFrame.
#pragma once
#include "Psycle.hpp"
#include "WaveEdChildView.hpp"

namespace psycle { namespace host {

		class CMainFrame;

		/// wave editor frame window.
		class CWaveEdFrame : public CFrameWnd
		{
			DECLARE_DYNAMIC(CWaveEdFrame)
		public:
			CWaveEdFrame(class ProjectData* projects_, CMainFrame* pframe);
			CWaveEdFrame();

		public:
		//	SetWave(signed short *pleft,signed short *pright,int numsamples, bool stereo);
			void GenerateView();
			void Notify(void);

			Song* song();
			CMainFrame *_pFrame;
		// Overrides
			// ClassWizard generated virtual function overrides
			//{{AFX_VIRTUAL(CWaveEdFrame)
			public:
			virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
			protected:
			virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
			//}}AFX_VIRTUAL	
		private:
			void AdjustStatusBar(int ins);
			void PlayFrom(unsigned long startpos);
			CStatusBar statusbar;
			CToolBar ToolBar;
			CWaveEdChildView wavview;
			int wsInstrument;
			bool bPlaying;
			ProjectData* projects_;
		// Implementation
		protected:
			// Generated message map functions
			//{{AFX_MSG(CWaveEdFrame)
			afx_msg void OnClose();
			afx_msg void OnUpdateStatusBar(CCmdUI *pCmdUI);
			afx_msg void OnUpdateSelection(CCmdUI *pCmdUI);
			afx_msg void OnUpdatePlayButtons(CCmdUI *pCmdUI);
			afx_msg void OnUpdateStopButton(CCmdUI *pCmdUI);
			afx_msg void OnUpdateReleaseButton(CCmdUI *pCmdUI);
			afx_msg void OnUpdateFFandRWButtons(CCmdUI* pCmdUI);
			afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
			afx_msg void OnDestroy();
			afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
			afx_msg void OnEditCut();
			afx_msg void OnPlay();
			afx_msg void OnRelease();
			afx_msg void OnPlayFromStart();
			afx_msg void OnStop();
			afx_msg void OnFastForward();
			afx_msg void OnRewind();
			//}}AFX_MSG
			DECLARE_MESSAGE_MAP()
		};

		//{{AFX_INSERT_LOCATION}}
		// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

	}   // namespace
}   // namespace
