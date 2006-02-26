///\file
///\brief interface file for psycle::host::CPsycleApp.
#pragma once
#include "resources/resources.hpp" // main symbols
#include "psycle/host/global.hpp"
#include <operating_system/logger.hpp>
NAMESPACE__BEGIN(psycle)
	NAMESPACE__BEGIN(host)

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
			Global _global;
			afx_msg void OnAppAbout();
		private:
			void GetNaiveCPUFreq();
			void ProcessCmdLine(CMainFrame*);
			void LoadRecent(CMainFrame*);
			void SaveRecent(CMainFrame*); friend /*void*/ class CMainFrame/*::OnClose()*/;
		};

	NAMESPACE__END
NAMESPACE__END
