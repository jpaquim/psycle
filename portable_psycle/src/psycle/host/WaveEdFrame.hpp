///\file
///\brief interface file for psycle::host::CWaveEdFrame.
#pragma once
#include "WaveEdChildView.hpp"
NAMESPACE__BEGIN(psycle)
	NAMESPACE__BEGIN(host)
		class CMainFrame;

		/// wave editor frame window.
		class CWaveEdFrame : public CFrameWnd
		{
		public:
			CWaveEdFrame();
			CWaveEdFrame(Song* _sng,CMainFrame* pframe);
			virtual ~CWaveEdFrame() throw();
		protected: 
			DECLARE_DYNAMIC(CWaveEdFrame)

		public:
		//	SetWave(signed short *pleft,signed short *pright,int numsamples, bool stereo);
			void GenerateView();
			void Notify(void);
			Song *_pSong;
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
			void AdjustStatusBar(int ins, int wav);
			CStatusBar statusbar;
			CWaveEdChildView wavview;
		// Implementation
		protected:
			// Generated message map functions
			//{{AFX_MSG(CWaveEdFrame)
			afx_msg void OnClose();
			afx_msg void OnUpdateStatusBar(CCmdUI *pCmdUI);
			afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
			afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
			afx_msg void OnEditCut();
			//}}AFX_MSG
			DECLARE_MESSAGE_MAP()
		};

		//{{AFX_INSERT_LOCATION}}
		// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

	NAMESPACE__END
NAMESPACE__END
