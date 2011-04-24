///\file
///\brief interface file for psycle::host::CFrameMachine.
#pragma once
#include "Psycle.hpp"

namespace psycle {
namespace host {
		class CChildView;
		class Machine;
		class CBaseParamView;

		/// machine window.
		class CFrameMachine : public CFrameWnd
		{
			DECLARE_DYNAMIC(CFrameMachine)
		public:
			CFrameMachine(Machine* pMachine);
			virtual ~CFrameMachine();
		protected:
			CFrameMachine(); // protected constructor used by dynamic creation

		public:
			virtual void ResizeWindow(CRect* pRect);
			virtual void GetWindowSize(CRect &rcFrame, CRect &rcClient, CRect *pRect = NULL);
			virtual void PostOpenWnd();

			virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
			virtual void PostNcDestroy();

		protected:
			virtual CBaseParamView* CreateView();

		// Attributes
		public:
			inline Machine& machine(){ return *_machine; }
			bool *_pActive; // It is used to help the program know if this window is open or not.
			CChildView *wndView;
		protected:
			Machine* _machine;
			CBaseParamView* pView;

		protected:
			afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
			afx_msg void OnClose();
			afx_msg void OnDestroy();
			afx_msg void OnTimer(UINT_PTR nIDEvent);
			afx_msg void OnSetFocus(CWnd* pOldWnd);
			afx_msg void OnSizing(UINT fwSide, LPRECT pRect);
			afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
			afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
			afx_msg void OnOperationsEnabled();
			afx_msg void OnUpdateOperationsEnabled(CCmdUI *pCmdUI);
			afx_msg void OnProgramsRandomizeprogram();
			afx_msg void OnParametersResetparameters();
			afx_msg void OnParametersCommand();
			afx_msg void OnViewsBankmanager();
			afx_msg void OnUpdateParametersCommand(CCmdUI *pCmdUI);
			afx_msg void OnMachineAboutthismachine();
			DECLARE_MESSAGE_MAP()
		};

	}   // namespace
}   // namespace

