///\file
///\brief interface file for psycle::host::CPsycleApp.
#pragma once
#include "resources/resources.hpp" // main symbols
#include <operating_system/logger.hpp>
namespace psycle
{
	namespace host
	{
		class CMainFrame; // forward declaration

		/// root class.
		class CPsycleApp : public CWinApp
		{
		public:
			CPsycleApp();
			virtual ~CPsycleApp() throw();
		// Overrides
			// ClassWizard generated virtual function overrides
			//{{AFX_VIRTUAL(CPsycleApp)
			public:
			virtual BOOL InitInstance();
			virtual int ExitInstance();
			//}}AFX_VIRTUAL
		// Implementation
		private:
			void ProcessCmdLine(CMainFrame*);
			void LoadRecent(CMainFrame*);
			void SaveRecent(CMainFrame*); friend /*void*/ class CMainFrame/*::OnClose()*/;
		public:
			Global _global;
			//{{AFX_MSG(CPsycleApp)
			afx_msg void OnAboutpsycle();
			//}}AFX_MSG
			DECLARE_MESSAGE_MAP()
		};

		//{{AFX_INSERT_LOCATION}}
		// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
	}
}
