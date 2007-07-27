///\file
///\brief interface file for psycle::host::CVstEditorDlg.
#pragma once
#include <psycle/engine/VSTHost.hpp>
#include <psycle/host/DefaultVstGui.hpp>
UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(psycle)
	UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(host)
		class CChildView;

		/// vst editor window.
		class CVstEditorDlg : public CFrameWnd
		{
		public:
			DECLARE_DYNCREATE(CVstEditorDlg)
			CVstEditorDlg(int n) {};
		protected:
			CVstEditorDlg(); // protected constructor used by dynamic creation
		// Attributes
		public:
			CWnd * pGui;
			CDefaultVstGui * pParamGui;
			vst::plugin * _pMachine;
			CChildView * wndView;
			UINT MachineIndex;
			bool * _editorActive;
		// Operations
		public:
			void Resize(int w,int h);
			void Refresh(int par,float val);
		// Overrides
			// ClassWizard generated virtual function overrides
			//{{AFX_VIRTUAL(CVstEditorDlg)
			protected:
			virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);
			//}}AFX_VIRTUAL
		// Implementation
		protected:
			bool editorgui;
			bool creatingwindow;
			CSplitterWnd _splitter;
			virtual ~CVstEditorDlg();
			// Generated message map functions
			//{{AFX_MSG(CVstEditorDlg)
			afx_msg void OnParametersRandomparameters();
			afx_msg void OnParametersShowpreset();
			afx_msg void OnSetFocus(CWnd* pOldWnd);
			afx_msg void OnClose();
			//}}AFX_MSG
			DECLARE_MESSAGE_MAP()
		};

		//{{AFX_INSERT_LOCATION}}
		// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
	UNIVERSALIS__COMPILER__NAMESPACE__END
UNIVERSALIS__COMPILER__NAMESPACE__END
