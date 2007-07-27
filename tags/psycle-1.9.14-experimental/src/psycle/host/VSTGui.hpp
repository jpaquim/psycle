///\file
///\brief interface file for psycle::host::CVstGui.
#pragma once
#include "ChildView.hpp"
#include <psycle/engine/VSTHost.hpp>
UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(psycle)
	UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(host)
		/// vst gui window.
		class CVstGui : public CFrameWnd
		{
			DECLARE_DYNCREATE(CVstGui)
			CVstGui(int n){};
			vst::proxy * proxy;
			Machine* _pMachine;
		protected:
			CVstGui(); // protected constructor used by dynamic creation
		// Attributes
		public:
		// Operations
		public:
			virtual ~CVstGui();
			void DoTheInit();
		// Overrides
			// ClassWizard generated virtual function overrides
			//{{AFX_VIRTUAL(CVstGui)
			//}}AFX_VIRTUAL
		// Implementation
		protected:
			// Generated message map functions
			//{{AFX_MSG(CVstGui)
			afx_msg void OnPaint();
			afx_msg void OnDestroy();
			afx_msg void OnEnterIdle(UINT nWhy, CWnd* pWho);
			afx_msg void OnTimer(UINT nIDEvent);
			afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
			afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
			afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
			//}}AFX_MSG
			DECLARE_MESSAGE_MAP()
		};

		//{{AFX_INSERT_LOCATION}}
		// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
	UNIVERSALIS__COMPILER__NAMESPACE__END
UNIVERSALIS__COMPILER__NAMESPACE__END
