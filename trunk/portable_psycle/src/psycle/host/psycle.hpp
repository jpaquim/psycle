///\file
///\brief interface file for psycle::host::CPsycleApp.
#pragma once
#include "resources/resources.hpp" // main symbols
#include <operating_system/logger.hpp>
NAMESPACE__BEGIN(psycle)
	NAMESPACE__BEGIN(host)
		class CMainFrame; // forward declaration

		/// root class.
		class CPsycleApp : public CWinApp
		{
		public:
			CPsycleApp();
			virtual ~CPsycleApp() throw();
		public:
			virtual BOOL InitInstance();
			virtual int ExitInstance();
		private:
			void ProcessCmdLine(CMainFrame*);
			void LoadRecent(CMainFrame*);
			void SaveRecent(CMainFrame*); friend /*void*/ class CMainFrame/*::OnClose()*/;
		public:
			DECLARE_MESSAGE_MAP()
		public:
			Global _global;
			afx_msg void OnAppAbout();
		};
	NAMESPACE__END
NAMESPACE__END
