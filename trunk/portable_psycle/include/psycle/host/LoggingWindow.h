#pragma once
#include "resource.h"
///\file
///\brief interface file for psycle::host::CLoggingWindow.
namespace psycle
{
	namespace host
	{
		class CMainFrame;

		/// logging window.
		class CLoggingWindow : public CDialog
		{
		public:
			CLoggingWindow(CWnd* pParent = 0);
			void Validate();

			CMainFrame* pParentMain;

			// Dialog Data
			//{{AFX_DATA(CLoggingWindow)
			enum { IDD = IDD_ERRORLOGGER };
			//}}AFX_DATA
		// Overrides
			// ClassWizard generated virtual function overrides
			//{{AFX_VIRTUAL(CLoggingWindow)
			protected:
			virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
			//}}AFX_VIRTUAL
		// Implementation
		protected:
			// Generated message map functions
			//{{AFX_MSG(CInstrumentEditor)
			virtual BOOL OnInitDialog();
			//}}AFX_MSG
			DECLARE_MESSAGE_MAP()
		};

		//{{AFX_INSERT_LOCATION}}
		// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
	}
}
