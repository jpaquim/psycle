///\file
///\brief interface file for psycle::host::CFrameMachine.
#pragma once
#include "Machine.hpp"
NAMESPACE__BEGIN(psycle)
	NAMESPACE__BEGIN(host)

	class Song;
	class CChildView;

#define K_XSIZE				28
#define K_YSIZE				28
#define K_NUMFRAMES			63
#define W_ROWWIDTH			150


		/// machine window.
		class CFrameMachine : public CFrameWnd
		{
			DECLARE_DYNCREATE(CFrameMachine)
		protected:
			CFrameMachine(); // protected constructor used by dynamic creation
		// Attributes
		public:
			CFrameMachine(int dum){MachineIndex = dum;};
		private:
			Machine* _pMachine;
			CBitmap b_knob;
			CFont	b_font;
		// Enviroment
			int numParameters;
			bool me;

			bool istweak;
			bool finetweak;
			bool ultrafinetweak;
			int tweakpar;
			int tweakbase;
			int sourcepoint;
			
			int ncol;
			int parspercol;
		// Operations
		public:
			void SelectMachine(Machine* pMachine);
			void Generate();
			CChildView *wndView;
			UINT MachineIndex;
			bool* _pActive;
		// Overrides
			// ClassWizard generated virtual function overrides
			//{{AFX_VIRTUAL(CFrameMachine)
			//}}AFX_VIRTUAL
		// Implementation
		protected:
			virtual ~CFrameMachine();
			// Generated message map functions
			//{{AFX_MSG(CFrameMachine)
			afx_msg void OnPaint();
			afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
			afx_msg void OnMouseMove(UINT nFlags, CPoint point);
			afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
			afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
			afx_msg void OnParametersRandomparameters();
			afx_msg void OnParametersResetparameters();
			afx_msg void OnParametersCommand();
			afx_msg void OnMachineAboutthismachine();
			afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
			afx_msg void OnDestroy();
			afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
			afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
			afx_msg void OnParametersShowpreset();
			afx_msg void OnSetFocus(CWnd* pOldWnd);
			afx_msg void OnTimer(UINT nIDEvent);
			//}}AFX_MSG
			DECLARE_MESSAGE_MAP()
		};

		//{{AFX_INSERT_LOCATION}}
		// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
	NAMESPACE__END
NAMESPACE__END
