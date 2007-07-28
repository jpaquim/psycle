///\file
///\brief interface file for psycle::host::CPsycleApp.
#pragma once
#include "resources/resources.hpp" // main symbols
#include "global.hpp"
#include "loggers.hpp"
#include "mfc_namespace.hpp"
PSYCLE__MFC__NAMESPACE__BEGIN(psycle)
	PSYCLE__MFC__NAMESPACE__BEGIN(host)

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
			BOOL PreTranslateMessage(MSG* pMsg);
			virtual BOOL IsIdleMessage( MSG* pMsg );
			virtual BOOL OnIdle(LONG lCount);
		public:
			Global _global;
			afx_msg void OnAppAbout();
		private:
			void GetNaiveCPUFreq();
			void ProcessCmdLine(LPSTR cmdline);
			void LoadRecent(CMainFrame*);
			void SaveRecent(CMainFrame*); friend /*void*/ class CMainFrame/*::OnClose()*/;
			UINT m_uUserMessage;
		};


		/////////////////////////////////////////////////////////////////////////////
		// CAboutDlg dialog used for App About

		class CAboutDlg : public CDialog
		{
		public:
			CAboutDlg();

			// Dialog Data
			//{{AFX_DATA(CAboutDlg)
			enum { IDD = IDD_ABOUTBOX };
			CStatic	m_asio;
			CEdit	m_sourceforge;
			CEdit	m_psycledelics;
			CStatic	m_steincopyright;
			CStatic	m_headerdlg;
			CButton	m_showabout;
			CStatic	m_headercontrib;
			CStatic	m_aboutbmp;
			CEdit	m_contrib;
			CStatic m_versioninfo;
			//}}AFX_DATA

			// ClassWizard generated virtual function overrides
			//{{AFX_VIRTUAL(CAboutDlg)
		protected:
			virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
			//}}AFX_VIRTUAL

			// Implementation
		protected:
			//{{AFX_MSG(CAboutDlg)
			afx_msg void OnContributors();
			virtual BOOL OnInitDialog();
			afx_msg void OnShowatstartup();
			//}}AFX_MSG
			DECLARE_MESSAGE_MAP()
		};



	PSYCLE__MFC__NAMESPACE__END
PSYCLE__MFC__NAMESPACE__END
