///\file
///\brief interface file for psycle::host::CPsycleApp.
#pragma once
#include <psycle/host/resources/resources.hpp> // main symbols
#include <psycle/host/uiglobal.hpp>
#include <operating_system/logger.hpp>
UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(psycle)
	UNIVERSALIS__COMPILER__NAMESPACE__BEGIN(host)

		class CMainFrame; // forward declaration

		/// root class.
		class CPsycleApp : public CWinApp
		{
			DECLARE_MESSAGE_MAP()
		public:
			CPsycleApp();
			virtual ~CPsycleApp() throw();
		public:
			virtual BOOL InitInstance();
			virtual int ExitInstance();
		public:
			UIGlobal _global;
			afx_msg void OnAppAbout();
		private:
			void ProcessCmdLine(CMainFrame*);
			void LoadRecent(CMainFrame*);
			void SaveRecent(CMainFrame*); friend /*void*/ class CMainFrame/*::OnClose()*/;
		};

		extern CPsycleApp theApp;

	UNIVERSALIS__COMPILER__NAMESPACE__END
UNIVERSALIS__COMPILER__NAMESPACE__END
